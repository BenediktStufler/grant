/*
 * We use Luc Devroye's algorithm [1] to simulate a weighted balls in boxes
 * model. 
 *
 * The idea is to sample multinomially distributed random variates many times 
 * until a certain stopping condition is reached. 
 * 
 * We do our best to speed up the computation by distributing the workload 
 * on multiple threads.
 *
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
	mpfr_t *xi;
	gsl_rng *rgen;
	INT **N;
	DOUBLE *q;
	unsigned int num;
	unsigned int *counter;
	pthread_mutex_t *mut;
	int *ex;
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
	mpfr_t *xi = ((struct targ *)dim)->xi;
	DOUBLE *q = ((struct targ *)dim)->q;
	INT **N = ((struct targ *)dim)->N;
	gsl_rng *rgen = ((struct targ *)dim)->rgen;
	unsigned int num = ((struct targ *)dim)->num;
	unsigned int *counter = ((struct targ *)dim)->counter;
	pthread_mutex_t *mut = ((struct targ *)dim)->mut;
	int *ex = ((struct targ *)dim)->ex;

	INT i;
	INT sumN = 0;
	INT sumE = 0;
	INT *Nentry;


	/* calls to calloc need to be protected by mutex */
	pthread_mutex_lock(mut);
	Nentry = (INT *) calloc(n, sizeof(INT));
	if(Nentry == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function balls in boxes\n");
		exit(-1);
	}
	pthread_mutex_unlock(mut);


	/*
	 * N[0] ~ binom(n, xi[0])
	 * N[1] ~ binom(n - N[0], xi[1] / (norm - xi[0])
	 * N[2] ~ binom(n - N[0] - N[1], xi[2] / (norm - xi[0] - xi[1])
	 * and so on
	 *
	 */
	while(1) {
		sumE = 0;
		while( sumE != m) {

			// exit condition
			//
			if(*ex) {
				// we found enough configurations 
				
				// time to clean up...
				free(Nentry);

				// ... and go home
				return (void *) 0;
			}

			// take next sample
			for(i=0, sumN=0, sumE=0; i<n; i++) {
				if(mpfr_sgn(xi[i])) {		// check if xi[i] > 0.0
					Nentry[i] = gsl_ran_binomial(rgen, q[i], n - sumN);
					//DEBUG
					//printf("%u -- %u, %17.17Lf, %u\n", i, Nentry[i], q[i], n - sumN );
					sumN += Nentry[i];
					sumE += i*Nentry[i];
				} else {
					Nentry[i] = 0;
				}

				// we may abort and start over if we surpass the target
				if( sumE > m) break;
			}	
		}

		// we found a valid balls in boxes configuration

		/* begin of part that is partially locked by mutex */
		pthread_mutex_lock(mut);
		if(*counter >= num) {
			// another thread set the exit condition and we found a valid
			// balls in boxes configuration right afterwards before getting
			// to the next check of the exit condition

			// free allocated memory
			free(Nentry);
			
			// unlock mutex  - we're done here
			pthread_mutex_unlock(mut);

			return (void *) 0;
		} else {
			// yay, we found a valid balls in boxes configuration
			N[*counter] = Nentry;
			*counter += 1;

			// check if we gathered enough samples
			if(*counter >= num) {
				// set exit condition
				// nothing bad will happen if this operation is not atomic
				*ex = 1;

				// unlock mutex  - we're done here
				pthread_mutex_unlock(mut);

				return (void *) 0;
			}

			// get array for next entry
			// (remember to protec calloc calls by mutex)
			Nentry = (INT *) calloc(n, sizeof(INT));
			if(Nentry == NULL) {
				// memory allocation error
				fprintf(stderr, "Memory allocation error in function balls in boxes\n");
				exit(-1);
			}

		}
		// unlock mutex - there's still work to do
		pthread_mutex_unlock(mut);
		/* end of part that is partially locked by mutex */	
	}
	
}


/*
 * Simulate a balls in boxes model using multiple threads
 */
INT **threadedbinb(INT n, INT m, mpfr_t *xi, unsigned int numThreads, gsl_rng **rgens, unsigned int num) {
	struct targ *argList;   // arguments for the separate threads
	pthread_t *th;			// array of threads
	INT i;
	void *ret;

	INT **N = NULL;

	// mutex for thread synchronization
	pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

	// the variable counter will only be read / modified in an area 
	// protected by the mutex mut
	unsigned int counter = 0;

	// this variable will only be written to in an area protected by mutex mut
	int ex = 0;

	// variables for preprocessing 
	mpfr_t norm, sumXi, diff, quot;
	DOUBLE *q;



	// build array of arrays that will hold the results	
	N = (INT **) calloc(num, sizeof(INT*));
	if(N == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function threadedbinb\n");
		exit(-1);
	}
	for(i=0; i<num; i++) {
		N[i] = (INT *)  calloc(n, sizeof(INT*));
		if(N[i] == NULL) {
			// memory allocation error
			fprintf(stderr, "Memory allocation error in function threadedbinb\n");
			exit(-1);
		}
	}

	// preprocess the sequence q[] given by 
	// q[0] = xi[0]
	// q[1] = xi[1] / (1.0 - xi[0])
	// q[2] = xi[2] / (1.0 - xi[0] - xi[1])
	// and so on
	// 
	q = (DOUBLE *) calloc(n, sizeof(DOUBLE));
	if(q == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function threadedbinb\n");
		exit(-1);
	}

	// initializes high precision float variables
	// warning: mpfr sets default value to NaN (gmp initializes with 0.0)
	mpfr_init2(norm, PREC);
	mpfr_init2(sumXi, PREC);
	mpfr_init2(diff, PREC);
	mpfr_init2(quot, PREC);

	// norm = xi[0] + ... + xi[n-1]
	mpfr_set_ld(norm, 0.0, MPFR_RNDN);
	for(i=0; i<n; i++)
		mpfr_add(norm, norm, xi[i], MPFR_RNDN);		

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


	// pack list of arguments
	argList = calloc(sizeof(struct targ), numThreads);
	for(i=0; i<numThreads; i++) {
		argList[i].n = n;
		argList[i].m = m;
		argList[i].xi = xi;
		argList[i].N = N;
		argList[i].rgen = rgens[i];
		argList[i].num = num;
		argList[i].counter = &counter;
		argList[i].mut = &mut;
		argList[i].q = q;
		argList[i].ex = &ex;
	}



	/* launch threads */
	th = calloc(sizeof(pthread_t), numThreads);
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


	/* clean up */
	free(argList);
	free(th);
	free(q);

	return N; 
}



/*
 * Wrapper function for generating a single configuration
 * Used when we want to sample a small number of very large trees
 */
INT *tbinb(INT n, INT m, mpfr_t *xi, unsigned int numThreads, gsl_rng **rgens) {
	INT **multi;
	INT *single;
	multi = threadedbinb(n,m,xi,numThreads,rgens,1);
	single = multi[0];
	free(multi);
	return single;
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
		fprintf(stderr, "Memory allocation error in function threadedbinb\n");
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


	return N;
}

