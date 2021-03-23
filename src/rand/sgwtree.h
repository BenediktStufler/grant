/*
 * generate degree sequence from degree profile
 */
INT *gendegsequence(INT *N, INT size, gsl_rng *rgen) {
	INT i, j, p;	
	INT *out;

	// result will be stored in an array of integers
	out = (INT *) calloc(size, sizeof(INT));
	if(out == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function gendegsequence\n");
		exit(-1);
	}

	// create initial array with sequential list of outdegrees
	for(i=0, p=0; i<size; i++)
		for(j=0; j<N[i]; j++, p++)
			out[p]=i;

	// shuffle 
	gsl_ran_shuffle (rgen, out, size, sizeof (INT));

	return out;
}

/*
 * Cyclically shifts a vector to make it a degree sequence of a tree
 */

int cycshift(INT *D, INT size) {
	INT i, j;
	INT indmin;
	long long sum, min;
	INT *tmp;

	for(i=0, sum=0, min=0, indmin=0; i<size; i++) {	
		sum += (long long) D[i] - 1;
		if(sum < min) {
			min = sum;
			indmin = i;
		}
	}

	if(indmin < size-1) {
		tmp = (INT *) calloc(size, sizeof(INT));
		if(tmp == NULL) {
			// memory allocation error
			fprintf(stderr, "Memory allocation error in function cycshift\n");
			exit(-1);
		}	
		
		// save reordered list in temporary array
		for(i=indmin+1,j=0; i<size; i++,j++)
			tmp[j] = D[i];
		for(i=0; i<indmin+1; i++,j++)
			tmp[j] = D[i];

		// copy back
		for(i=0; i<size; i++)
			D[i] = tmp[i];

		// free memory
		free(tmp);
	}

	return 0;
}
				
/*
 * Compute tree from outdegree sequence - bfs order
 */
struct graph *deg2bfstree(INT *D, INT len) {
	struct graph *G;
	INT pos, i, j;

	// initialize graph
   	G = newgraph(len); 
	
	// add edges such that the i th entry of the outdegree list corresponds
	// to the outdegree of the i th vertex in bfs order and such that
	// the first vertex in the neighbourhood list of any non-root vertex is 
	// its parent

	if(len>0) {
		G->arr[0]->height = 0;	
		G->root = G->arr[0];
	}
	for(i=0, pos=1; i<len; i++) {
		// save vertex degree for later use
		G->arr[i]->deg = D[i];
		for(j=0; j<D[i]; j++, pos++) {
			addEdge(G->arr[i], G->arr[pos]);
			// save vertex height for later use
			G->arr[pos]->height = G->arr[i]->height + 1;
		}
	}	
	

	return G;
}



/*
 * Compute tree from outdegree sequence - dfs order
 */
struct graph *deg2dfstree(INT *D, INT len) {
	struct graph *G;
	INT i;

	G = deg2bfstree(D, len);	// construct tree in BFS-order
								// sets deg and height attributes
	G->bfs = G->arr;			// set bfs-order list
	G->arr = dfsorder(G, G->root); // calculate DFS-order
	
	// change vertex ids to dfs order
	for(i=0; i<G->num; i++)
		G->arr[i]->id = i;

	return G;
}




/*
 * Simulate a Galton-Watson tree conditioned on its number of vertices
 */
int gwtree(struct cmdarg *comarg, gsl_rng **rgens) {
	INT *degprofile;	// outdeg profile
	mpfr_t *xi;			// offspring law
	INT *D;				// degree sequence
	unsigned int counter;
	char *cname;
	struct graph *G, *H;
	INT i;

	// select offspring distribution
	xi = NULL;
	if( comarg->Tbeta ) { 
		if(comarg->beta <= 1.0) { 
			fprintf(stderr, "Error: please specify a sensible value BETA > 2.0\n"); 
			exit(-1); 
		} 
		if( !comarg->Tmu || comarg->mu <= 0.0) {
			fprintf(stderr, "Error: please specify a sensible value mu > 0.\n"); 
			exit(-1); 
		}
		xi = xipow(comarg->size, comarg->beta, comarg->mu); 
	} else if( comarg->Tgamma ) { 
		if(comarg->gamma <= 1.0) { 
			fprintf(stderr, "Error: please specify a sensible value GAMMA > 1.0\n"); 
			exit(-1); 
		} 
		if( !comarg->Tmu || comarg->mu <= 0.0) {
			fprintf(stderr, "Error: please specify a sensible value mu > 0.\n"); 
			exit(-1); 
		}
		xi = xicau(comarg->size, comarg->gamma, comarg->mu); 
	} else if( comarg->Tpoisson ) {
		// do nothing
	} else if( comarg->Ttria ) {
		xi = xitria(comarg->size);
	} else { 
		fprintf(stderr, "Please specify a branching mechanism and an output function via command line options. Example usage:\n\ngrant --beta=2.5 --mu=1.0 --size=100000 --outfile=./stabletree_1.5_100k.graphml --profile=degree_profile.txt\n\nSimulates a critical Galton-Watson tree with a power-law offspring distribution (P(xi = k) ~ const / k^beta) conditioned on having 100k vertices. The resulting graph is written in the graphml format to the specified outfile and the vertex outdegree profile is written to the file specified by the --profile parameter.\n\nYou may run `grant --help' for further options and detailed usage information.\n"); 
		exit(-1); 
	}


	for(counter=1; counter <= comarg->num; counter++) {	
		/* simulate balls in boxes model */
		if( comarg->Tpoisson ) {
			degprofile = binbpoisson(comarg->size, comarg->size-1, rgens);
		} else {
			degprofile = tbinb(comarg->size, comarg->size-1, xi, comarg->threads, rgens);
		}

		/* output vertex outdegree profile if requested */
		if( comarg->Tprofile ) {
			cname = convname(comarg->profile, counter, comarg->num, comarg->Tnum);
			outdegprofile(degprofile, comarg->size, cname);
			free(cname);
		}


		/* calculate degree sequence if necessary */
		if( comarg->Tdegfile || comarg->Toutfile || comarg->Tloopfile || comarg->Theightfile || comarg->Tcentfile ) {

			/* generate degree sequence with a fresh seed*/
			D = gendegsequence(degprofile, comarg->size, rgens[0]);
			/* cyclically shift sequence */
			cycshift(D, comarg->size);


			/* calculate graph if necessary */
			if( comarg->Tdegfile || comarg->Toutfile || comarg->Tloopfile || comarg->heightfile || comarg->Tcentfile ) {

				// already sets the height deg attribute of each vertex
				// list of vertices is now in dfs order, bfs order is set
				G = deg2dfstree(D, comarg->size);


				/* output tree if requested */
				if( comarg->Toutfile ) {
					cname = convname(comarg->outfile, counter, comarg->num, comarg->Tnum);
					outgraph(G,cname);
					free(cname);
				}
			
				/* calculate and output looptree if requested */	
				if( comarg->Tloopfile ) {
					cname = convname(comarg->loopfile, counter, comarg->num, comarg->Tnum);
					H = looptree(G, G->arr[0]);
					outgraph(H, cname);
					free_graph(H);
					free(cname);
				}


				/* output degree sequence if requested */
				if( comarg->Tdegfile ) {
					cname = convname(comarg->degfile, counter, comarg->num, comarg->Tnum);
					outdegseq(G, cname);
					free(cname);
				}
				
				/* output height sequence if requested */
				if( comarg->Theightfile ) {
					cname = convname(comarg->heightfile, counter, comarg->num, comarg->Tnum);
					outheightseq(G, cname);
					free(cname);
				}

				/* Calculate closeness centrality if requested */
				if( comarg->Tcentfile ) {
					cname = convname(comarg->centfile, counter, comarg->num, comarg->Tnum);
					threadedcentrality(G, 0, G->num, comarg->threads);
					outcent(G, cname);
					free(cname);
				}

				// clean up
				free_graph(G);
			}

			// clean up
			free(D);
		}
		// clean up
		free(degprofile);
	}

	// clean up offspring distribution
	if(xi != NULL) {
		for(i=0; i<comarg->size; i++)
			mpfr_clear(xi[i]);
		free(xi);
	}

	return 0;
}
