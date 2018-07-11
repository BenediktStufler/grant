
/*
 * Read connected graph from file instead of generating the graph at random
 */
int rfile(struct cmdarg *comarg) {
	struct graph *G, *H;
	INT *degprofile;


	/* read graph from file */
	G = parsegraphml(comarg->infile, comarg->vid);

	if(G->num == 0) return 0;	 // nothing to do if there are no vertices

	if(G->root == NULL) G->root = G->arr[0]; // set root if none was specified

	/* set height, vertex degrees, bfs order, disconnected warning flag */
	G->bfs = bfsorder(G, G->root, 1, 1);
	if(G->disconnected) {
		fprintf(stderr, "Error: graph from input file is disconnected. Disconnected graphs are not supported at the moment.\n"); 
		return -1;
	}

	/* output degree sequence if requested */
	if( comarg->Tdegfile ) {
		outdegseq(G, comarg->degfile);
	}
	
	/* output degree profile if requested */
	if( comarg->Tprofile ) {
		degprofile = makedegprofile(G);	// assumes that the deg parameters
										// have already been set
		outdegprofile(degprofile, G->num, comarg->profile);
		free(degprofile);
	}
	
	/* output height sequence if requested */
	if( comarg->Theightfile ) {
		outheightseq(G, comarg->heightfile);
	}

	/* Calculate closeness centrality if requested */
	if( comarg->Tcentfile ) {
		threadedcentrality(G, 0, G->num, comarg->threads);
		outcent(G, comarg->centfile);
	}

	/* calculate and output looptree if requested */	
	/* this only works if the graph is a tree */
	if( comarg->Tloopfile ) {
		/*
		// check if the graph is a tree (works only in case of undirected edges)
		for(sum=0, i=0; i<G->num; i++){
			sum += G->arr[i]->deg
		}
		// sum_v d(v) = 2 * #edges = 2 * (#vertices -1)
		if(sum + 2 != 2*G->num) {	
			fprintf(stderr, "Error: Graph is not a tree. Cannot construct loop tree.\n");
			exit(-1);
		}
		*/

		H = looptree(G, G->root);
		if(H) {
			outgraph(H, comarg->loopfile);
			free_graph(H);
		}
	}			


	/* clean up */
	free_graph(G);

	return 0;
}
