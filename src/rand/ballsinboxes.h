/*
 * We use Luc Devroye's algorithm [1] to simulate a weighted balls in boxes
 * model. 
 *
 * The idea is to sample multinomially distributed random variates many times 
 * until a certain stopping condition is reached. 
 * 
 * We speed up the computation by distributing the workload on multiple threads.
 *
 * References:
 *
 * [1] Luc Devroye, Simulating Size-constrained Galton–Watson Trees,
 * SIAM Journal on Computing 2012 41:1, 1-11
 */
 



/*
 * data that gets passed to a thread
 */
struct targ {
	INT n;
	INT m;
	INT *N;
	gsl_rng *rgen;
	pthread_mutex_t *mut;
	DOUBLE *q;
	INT prio;
	INT bsize;
	_Atomic INT *mprio;
	_Atomic INT *wprio;
	int *win;
	int id;
};


/*
 * We simulate a balls in boxes model with n balls, m boxes, and probability
 * weights given by xi[]. 
 *
 * xi[i], 0 <= i < n: probability weight sequence for the offspring law
 * N[i], 0 <= i < n: the number of boxes that contain precisely i balls
 * m: the number of balls
 *
 */
void *ballsinboxes(void *dim) {
	// retrieve data that got passed to thread
	INT n = ((struct targ *)dim)->n;
	INT m = ((struct targ *)dim)->m;
	INT *N = ((struct targ *)dim)->N;
	gsl_rng *rgen = ((struct targ *)dim)->rgen;
	pthread_mutex_t *mut = ((struct targ *)dim)->mut;
	DOUBLE *q = ((struct targ *)dim)->q;
	INT prio = ((struct targ *)dim)->prio;
	INT bsize = ((struct targ *)dim)->bsize;
	_Atomic INT *mprio = ((struct targ *)dim)->mprio;
	_Atomic INT *wprio = ((struct targ *)dim)->wprio;
	int *win = ((struct targ *)dim)->win;
	int id = ((struct targ *)dim)->id;

	INT i, k;
	INT sumN = 0;
	INT sumE = 0;
	INT best = 0;


	/*
	 * N[0] ~ binom(n, xi[0])
	 * N[1] ~ binom(n - N[0], xi[1] / (norm - xi[0])
	 * N[2] ~ binom(n - N[0] - N[1], xi[2] / (norm - xi[0] - xi[1])
	 * and so on
	 */
	k=0;
	while(1) {
		sumE = 0;
		while( sumE != m || sumN != n) {
			// update logical order of chunks
			if(k>=bsize) {
				k=0;
				prio = atomic_fetch_add_explicit(mprio, 1, memory_order_relaxed);
				// DEBUG
				//printf("Max priority: %"STR(FINT)"\n", *mprio); 
			}
			k++;

			// exit condition
			best = atomic_load_explicit(wprio, memory_order_relaxed);
			if(best != 0 && best < prio) {
    			return NULL;
			}

			// take next sample
			for(i=0, sumN=0, sumE=0; i<n; i++) {
				if(q[i]>0 && n > sumN) {
					N[i] = gsl_ran_binomial(rgen, q[i], n - sumN);
					//DEBUG
					//printf("%u -- %u, %17.17Lf, %u\n", i, Nentry[i], q[i], n - sumN );
					sumN += N[i];
					sumE += i*N[i];
				} else {
					N[i] = 0;
				}

				// if we used all slots but did not achieve target sumE start over
				if( n == sumN && sumE != m ) break;

				// if we overshoot the target size start over
				if(sumE > m) break;

				// the remaining n - sumN slots each contribute at least i+1
				// we can start over if this will definitely overshoot the target value for sumE
				// use integer division to avoid overflow in calculation
				if( (n - sumN) > (m - sumE) / (i+1) ) break;
			}	
		}

		// we found a valid balls in boxes configuration

		/* begin of part that is partially locked by mutex */
		pthread_mutex_lock(mut);

		// best==0: we are the first to find a valid configuration
		// best!=0 && prio < best: we are not the first to find a valid configuration
		// and ours takes precedence
		best = atomic_load_explicit(wprio, memory_order_relaxed);
		if(best == 0 || prio < best) {
    		*win = id;
    		atomic_store_explicit(wprio, prio, memory_order_relaxed);
		}
		
		// unlock mutex 
		pthread_mutex_unlock(mut);
		/* end of part that is partially locked by mutex */	

		// exit
		return (void *) 0;
	}	
}


/*
 * precompute probability weights for balls in boxes model
 */
DOUBLE *precq(mpfr_t *xi, INT n) {
	mpfr_t norm, sumXi, diff, quot;
	DOUBLE *q;
	INT i;

	// precompute the sequence q[] given by 
	// q[0] = xi[0]
	// q[1] = xi[1] / (1.0 - xi[0])
	// q[2] = xi[2] / (1.0 - xi[0] - xi[1])
	// and so on
	// 
	q = (DOUBLE *) calloc(n, sizeof(DOUBLE));
	if(q == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function precq\n");
		exit(-1);
	}

	// initializes high precision float variables
	// warning: mpfr sets default value to NaN (gmp initializes with 0.0)
	mpfr_init2(norm, PREC);
	mpfr_init2(sumXi, PREC);
	mpfr_init2(diff, PREC);
	mpfr_init2(quot, PREC);

	// norm = xi[0] + ... + xi[n-1]
	//mpfr_set_ld(norm, 0.0, MPFR_RNDN);
	//for(i=0; i<n; i++)
	//	mpfr_add(norm, norm, xi[i], MPFR_RNDN);		
	mpfr_set_ld(norm, 1.0, MPFR_RNDN);

	// q[i] = xi[i] / (xi[i] + ... + xi[n-1])
	//      = xi[i] / (norm - xi[0] - ... xi[i-1])
	mpfr_set_ld(sumXi, 0.0, MPFR_RNDN);
	for(i=0; i<n; i++) {
		mpfr_sub(diff, norm, sumXi, MPFR_RNDN);		// diff = norm - sumXi
		mpfr_div(quot, xi[i], diff, MPFR_RNDN);		// quot = xi[i] / diff
		q[i] = mpfr_get_ld(quot, MPFR_RNDN);			// cast to DOUBLE
		mpfr_add(sumXi, sumXi, xi[i], MPFR_RNDN);	// sumXi += xi[i]

		// check for precision error
		if(q[i]>1.1) {
			printf("Calculation precision too small for parameter range.\n");
			printf("Emergency abort.\n");
			exit(-1);
		}
	}

	// free space occupied by high precision variables
	mpfr_clear(norm);
	mpfr_clear(sumXi);
	mpfr_clear(diff);
	mpfr_clear(quot);

	return q;
}


/*
 * Simulate a balls in boxes model using multiple threads
 */
INT *tbinb(INT n, INT m, DOUBLE *q, unsigned int numThreads, gsl_rng **rgens) {
	struct targ *argList;   // arguments for the separate threads
	pthread_t *th;			// array of threads
	INT i;
	void *ret;

	INT *N = NULL;

	// mutex for thread synchronization
	pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

	// keep track of logical execution order
	_Atomic INT mprio;
	atomic_init(&mprio, numThreads + 1);
	_Atomic INT wprio;
	atomic_init(&wprio, 0);
	int win = 0;

	// pack list of arguments
	argList = calloc(numThreads, sizeof(struct targ));
	if(argList == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function tbinb\n");
		exit(-1);
	}

	for(i=0; i<numThreads; i++) {
		argList[i].n = n;
		argList[i].m = m;
		argList[i].N = calloc(n, sizeof(INT));
		if(argList[i].N == NULL) {
			// memory allocation error
			fprintf(stderr, "Memory allocation error in function tbinb\n");
			exit(-1);
		}
		argList[i].rgen = rgens[i];
		argList[i].mut = &mut;
		argList[i].q = q;
		argList[i].prio = i+1;
		argList[i].mprio = &mprio;
		argList[i].wprio = &wprio;
		argList[i].bsize = 5;
		argList[i].win = &win;
		argList[i].id = i;
	}

	/* launch threads */
	th = calloc(numThreads, sizeof(pthread_t));
	if(th == NULL) {
		fprintf(stderr, "Memory allocation error in function tbinb\n");
		exit(-1);
	}
	for(i=0; i<numThreads; i++) {
		if(pthread_create(&th[i], NULL, &ballsinboxes, &argList[i] )) {
			fprintf(stderr, "Error launching thread number %"STR(FINT)"\n", i);
			exit(-1);
		}
	}

	/* wait for threads to finish */
	for(i=0; i<numThreads; i++) {
		pthread_join(th[i], &ret);
		if(ret) {
			fprintf(stderr, "Error executing thread number %"STR(FINT)"\n", i);
			exit(-1);
		}
	}

	/* winning configuration */
	N = argList[win].N;

	//DEBUG
	//printf("Max priority: %"STR(FINT)"\n", mprio); 

	/* clean up */
	for(i=0; i<numThreads; i++)
		if(i != win) free(argList[i].N);

	free(argList);
	free(th);
	pthread_mutex_destroy(&mut);

	return N; 
}



/*
 * Sample a Poisson Balls in Boxes model
 */
INT *binbpoisson(INT n, INT m, gsl_rng **rgens) {
	INT i;
	INT *bnb;
	INT *N;
	INT box;

	// the array of boxes
	bnb = (INT *) calloc(n, sizeof(INT));
	N = (INT *) calloc(n, sizeof(INT));
	if(bnb == NULL || N == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function binbpoisson\n");
		exit(-1);
	}
	
	//DEBUG
	//printf("RANGE: %lu - %lu\n", gsl_rng_min(rgens[0]), gsl_rng_max(rgens[0]));

	// initialize boxes and profile to 0
	for(i=0; i<n; i++) {
		bnb[i] = 0;
		N[i] = 0;
	}

	// fill boxes
	for(i=0; i<m; i++) {
		box = gsl_rng_uniform_int(rgens[0], n);	
		bnb[box]++;
	}

	// calculate profile
	for(i=0; i<n; i++)
		N[bnb[i]]++;

	// clean up
	free(bnb);

	return N;
}

