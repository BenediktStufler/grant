/*
 *
 * We define a data structure that holds our graph
 *
 */ 


struct vertex;				// prototype

struct list {
	struct vertex *ve;		// the neighbour vertex
							// equal to NULL if we are at the end of the list
	struct list *ne;		// the next neighbour vertex in the list
	struct list *pr;		// the previous neighbour vertex in the list
};
	
struct queue {				// implements doubly ended queue of vertices
	struct list *li;		// start
	struct list *lie;		// end
};


struct vertex {
	struct queue *qu;		// list of neighbour vertices
	INT id;					// unique id 
	INTD cent;				// sum of distances from this vertex to all others
	INT height;				// height of vertex 
	INT deg;				// (out)degree of vertex
	int x;					// temporary variable
};


struct graph {				// holds a graph; optional arguments need to 
							// be initialized with NULL
	INT num;				// the number of vertices
	struct vertex *root;	// root vertex (optional)
	struct vertex **arr;	// dynamically allocated array with pointers
   							// to all vertices 
	struct vertex **bfs;	// dynamically allocated array with pointers
   							// to all vertices (optional)
	struct vertex **dfs;	// dynamically allocated array with pointers
   							// to all vertices (optional)
	int disconnected;		// warning flag if the graph is disconnected
};




// generates an empty queue
struct queue *newqueue() {
	struct queue *qu;
	qu = (struct queue *) malloc(sizeof(struct queue));
	if(qu == NULL) {
		fprintf(stderr, "Memory allocation error in function newqueue.\n");	
		exit(-1);
	}
	qu->li = NULL;
	qu->lie = NULL;
	return qu;
}

// generates a graph with num vertices and no edges
struct graph* newgraph(INT num) {
	INT i;	
	struct graph *G;

	G = (struct graph *) malloc(sizeof(struct graph));
	if(G == NULL) {
		fprintf(stderr, "Memory allocation error in function newgraph.\n");	
		exit(-1);
	}

	G->num = num;
	G->root = NULL;
	G->bfs = NULL;
	G->dfs = NULL;
	G->disconnected = 0;

	G->arr = (struct vertex **) calloc(num, sizeof(struct vertex *));
	if(G->arr == NULL) {
		fprintf(stderr, "Memory allocation error in function newgraph.\n");	
		exit(-1);
	}
	
	for(i=0; i<num; i++) {
		G->arr[i] = (struct vertex *) malloc(sizeof(struct vertex));
		if(G->arr[i] == NULL) {
			fprintf(stderr, "Memory allocation error in function newgraph.\n");	
			exit(-1);
		}
		G->arr[i]->id = i;
		G->arr[i]->qu = newqueue();
		G->arr[i]->deg = 0;
		G->arr[i]->height = 0;
		G->arr[i]->cent = 0;
	}

	return G;
}


// insert list entry to the right
// returns address of new entry
struct list *insr(struct list *li, struct vertex *v) {
	struct list *newlist;

	if(li == NULL) return NULL;

	// create new list entry
	newlist = (struct list *) malloc( sizeof(struct list) );
	if(newlist == NULL) {
		fprintf(stderr, "Error allocating memory in function insr.\n");
		exit(-1);
	}

	// initialize new list entry
	newlist->ve = v;
	newlist->pr = li;
	newlist->ne = li->ne;

	// insert new list entry to the right of target
	if(li->ne) li->ne->pr = newlist;
	li->ne = newlist;

	return newlist;
}

// insert list entry to the left
// returns address of the new entry
struct list *insl(struct list *li, struct vertex *v) {
	struct list *newlist;

	if(li == NULL) return NULL;	

	// create new list entry
	newlist = (struct list *) malloc( sizeof(struct list) );
	if(newlist == NULL) {
		fprintf(stderr, "Error allocating memory in function insl.\n");
		exit(-1);
	}

	// initialize new list entry
	newlist->ve = v;
	newlist->pr = li->pr;
	newlist->ne = li;

	// insert new list entry to the left of target
	if(li->pr) li->pr->ne = newlist;
	li->pr = newlist;

	return newlist;
}

// remove list entry
void delli(struct list *li) {
	if(li->pr) li->pr->ne = li->ne;
	if(li->ne) li->ne->pr = li->pr;
	free(li);
}

// delete queue 
void delqueue(struct queue *qu) {
	struct list *li, *la;
	
	if(qu) {
		for(li = qu->li; li != NULL; ) {
			la = li->ne;
			free(li);
			li = la;
		}
		free(qu);
	}
}
	

// add element to the right of queue
int pushr(struct queue *qu, struct vertex *v) {
	
	if(qu == NULL) return -1;

	if(qu->lie == NULL) {
		qu->li = (struct list *) malloc( sizeof(struct list) );
		if(qu->li == NULL) {
			fprintf(stderr, "Error allocating memory in function insl.\n");
			exit(-1);
		}
		qu->lie = qu->li;
		qu->li->ve = v;
		qu->li->ne = NULL;
		qu->li->pr = NULL;
	} else {
		qu->lie = insr(qu->lie, v);
	}

	return 0;
}

// add element to the left of queue
int pushl(struct queue *qu, struct vertex *v) {
	
	if(qu == NULL) return -1;

	if(qu->li == NULL) {
		qu->li = (struct list *) malloc( sizeof(struct list) );
		if(qu->li == NULL) {
			fprintf(stderr, "Error allocating memory in function insl.\n");
			exit(-1);
		}
		qu->lie = qu->li;
		qu->li->ve = v;
		qu->li->ne = NULL;
		qu->li->pr = NULL;
	} else {
		qu->li = insl(qu->li, v);
	}

	return 0;
}

// remove element from the right of list
struct vertex *popr(struct queue *qu) {
	struct list *li;
	struct vertex *v;
	if(qu == NULL) return NULL;
	if(qu->lie == NULL) return NULL;

	v = qu->lie->ve;
	li = qu->lie->pr;
	delli(qu->lie);
	qu->lie = li;

	// if the queue is empty now, we also need to adjust the left end point
	if(qu->lie == NULL) qu->li = NULL;

	return v;
}


// remove element from the left of list
struct vertex *popl(struct queue *qu) {
	struct list *li;
	struct vertex *v;
	if(qu == NULL) return NULL;
	if(qu->li == NULL) return NULL;

	v = qu->li->ve;
	li = qu->li->ne;
	delli(qu->li);
	qu->li = li;

	// if the queue is empty now, we also need to adjust the right end point
	if(qu->li == NULL) qu->lie = NULL;

	return v;
}


void free_graph(struct graph *G) {
	INT i;


	for(i=0; i<G->num; i++) {
		// free edges
		delqueue(G->arr[i]->qu);	

		// free vertex
		free(G->arr[i]);
	}

	// free vertex arrays
	free(G->arr);
	if(G->dfs != NULL) free(G->dfs);
	if(G->bfs != NULL) free(G->bfs);

	// free graph
	free(G);
}


/* 
 * outputs undirected graph in graphml format
 * assumption is that any edge corresponds to a pair of di-edges 
 */ 
void print_graphml(struct graph *G, FILE *outstream) {
	INT i;
	struct list *li;

	fprintf(outstream, "<graphml>\n");
	fprintf(outstream, "  <graph id='randomgraph' edgedefault='undirected'>\n");

	// write nodes
	for(i=0; i<G->num; i++) 
		fprintf(outstream, "    <node id=\'%" STR(FINT) "\' />\n", i);

	// write edges
	// in order to avoid writing an edge twice we only write the edge
	// if the id of the source is smaller than the id of the target
	for(i=0; i<G->num; i++) {
		for(li = G->arr[i]->qu->li; li != NULL; li = li->ne) {
			if(li->ve->id > G->arr[i]->id) {
				fprintf(outstream, "    <edge source=\'%" STR(FINT) "\' target=\'%" STR(FINT) "\' />\n", G->arr[i]->id, li->ve->id);
			}
		}
	}
	fprintf(outstream, "  </graph>\n");
	fprintf(outstream, "</graphml>\n");
}



// adds a directed edge from v to w
// if the edge is already present a second (multi-)edge will be added
int addDiEdge(struct vertex *v, struct vertex *w) {
	return pushr(v->qu, w);
}

// adds a directed edge from v to w and from w to v
// if any edge is already present, then a second (multi-)edge will be added
int addEdge(struct vertex *v, struct vertex *w) {
	if(addDiEdge(v,w)) return -1;
	if(addDiEdge(w,v)) return -1;

	return 0;
}

// calculate dfsorder of vertices
struct vertex **dfsorder(struct graph *G, struct vertex *root) {
	struct vertex **dfs;
	struct vertex *v;
	struct list *li;
	struct queue *qu;
	INT i;

	/* sanity checks */
	if(root == NULL || G == NULL) return NULL;
	if(G->num < 1) return NULL;

	dfs = (struct vertex **) calloc(G->num, sizeof(struct vertex *));
	if(dfs == NULL) {
		fprintf(stderr, "Error allocating memory in function dfsorder.\n");
		exit(-1);
	}

	/* initialize marker for vertices */
	for(i = 0; i < G->num; i++)
		G->arr[i]->x = 1;


	qu = newqueue();
	pushr(qu, root);
	root->x = 0;	// mark root as queued
	for(i = 0; i < G->num; i++) {
		v = popr(qu);
		dfs[i] = v;
		for(li = v->qu->lie; li != NULL; li = li->pr) {
			// check if vertex was visited before
			if(li->ve->x) {
				pushr(qu, li->ve);
				li->ve->x = 0;
			}
		}
	}
		
	delqueue(qu);
	return dfs;
}

// calculate bfsorder of vertices
// setheight --> sets height parameter for each vertex
// setdeg --> sets deg = degree for each vertex
// setoutdeg --> sets deg = outdegree for each vertex
struct vertex **bfsorder(struct graph *G, struct vertex *root, int setdeg, int setheight) {
	struct vertex **bfs;
	struct vertex *v;
	struct list *li;
	struct queue *qu;
	INT i;

	/* sanity checks */
	if(root == NULL || G == NULL) return NULL;
	if(G->num < 1) return NULL;

	bfs = (struct vertex **) calloc(G->num, sizeof(struct vertex *));
	if(bfs == NULL) {
		fprintf(stderr, "Error allocating memory in function dfsorder.\n");
		exit(-1);
	}


	/* initialize vertex states */
	for(i = 0; i < G->num; i++) {
		/* initialize marker for vertices to state 'unqueued' */
		G->arr[i]->x = 1;
		/* initialize vertex degree to 0 if necessary */
		if(setdeg) G->arr[i]->deg = 0;
	}


	qu = newqueue();
	pushr(qu, root);

	root->x = 0;	// mark root as queued
	if(setheight) root->height = 0;	// set height of root to zero
	for(i=0; qu->li != NULL; i++) {
		v = popl(qu);
		bfs[i] = v;
		for(li = v->qu->li; li != NULL; li = li->ne) {
			if(setdeg) v->deg += 1; 	// increment vertex degree
			// check if vertex was visited before
			if(li->ve->x) {
				if(setheight) li->ve->height = v->height + 1;	// set height
				pushr(qu, li->ve);
				li->ve->x = 0;	// mark vertex as queued
			}
		}
	}

	// set flag if graph is disconnected
	if(i < G->num) G->disconnected = 1;
		
	delqueue(qu);
	return bfs;
}

// calculate looptree
struct graph *looptree(struct graph *G, struct vertex *root) {
	struct vertex *v, *w, *x;
	struct list *li;
	struct queue *qu, *cyc;
	struct graph *H;
	INT i;
	int flag;

	/* sanity checks */
	if(root == NULL || G == NULL) return NULL;
	if(G->num <= 0) return NULL;

	/* new graph */
	H = newgraph(G->num);
	if(H == NULL) {
		fprintf(stderr, "Error allocating memory in function looptree.\n");
		exit(-1);
	}


	/* initialize vertex states */
	for(i = 0; i < G->num; i++) {
		/* initialize marker for vertices to state 'unqueued' */
		G->arr[i]->x = 1;
	}


	qu = newqueue();
	cyc = newqueue();
	pushr(qu, root);

	flag = 0;
	root->x = 0;	// mark root as queued
	while(qu->li) {
		v = popl(qu);
		for(li = v->qu->li; li != NULL; li = li->ne) {
			// check if vertex was visited before
			if(li->ve->x) {
				pushr(cyc, li->ve);
				pushr(qu, li->ve);
				li->ve->x = 0;	// mark vertex as queued
			}
		}
		// create cycle
		if(cyc->li !=  NULL) {
			w = popl(cyc);	
			// add starting edge 
			addEdge(H->arr[v->id], H->arr[w->id]);
			// add middle edges
			// flag indicates presence of middle edges
			if(cyc->li) flag=1;
			while(cyc->li) {
				x = popl(cyc);
				addEdge(H->arr[w->id], H->arr[x->id]);
				w = x;
			}	
			// add end edge if necessary
			if(flag) {
				addEdge(H->arr[v->id], H->arr[w->id]);
				flag = 0;
			}
		}
	}

		
	delqueue(cyc);
	delqueue(qu);
	return H;
}

INT *makedegprofile(struct graph *G) {
	INT *N, i;

	N = (INT *) calloc(G->num, sizeof(INT));
	if(N == NULL) {
		fprintf(stderr, "Error allocating memory in function makeprofile.\n");
		exit(-1);
	}

	// initialize array
	for(i=0; i<G->num; i++)
		N[i] = 0;

	// count degrees
	for(i=0; i<G->num; i++)
		N[G->arr[i]->deg] += 1;

	return N;
}


// unit test for the functions of this header file
void unit_test_graph() {
	struct graph *G, *H;	
	int i;

	printf("        4 - 6\n");
	printf("      / \n");
	printf("    1 - 5 \n");
	printf("  / \n");
	printf("0 - 2 \n");
	printf("  \\ \n");
	printf("    3 - 7 \n");
	printf("     \\  \n");
	printf("        8\n");

	G = newgraph(9);
	addEdge(G->arr[0], G->arr[1]);	
	addEdge(G->arr[0], G->arr[2]);	
	addEdge(G->arr[0], G->arr[3]);	
	addEdge(G->arr[1], G->arr[4]);	
	addEdge(G->arr[1], G->arr[5]);	
	addEdge(G->arr[4], G->arr[6]);	
	addEdge(G->arr[3], G->arr[7]);	
	addEdge(G->arr[3], G->arr[8]);	

	print_graphml(G, stdout);

	G->dfs = dfsorder(G, G->arr[0]);
	G->bfs = bfsorder(G, G->arr[0], 1, 1);

	printf("DFS order: ");
	for(i=0; i<G->num; i++)
		printf("%"STR(FINT)", ", G->dfs[i]->id);
	printf("\n");

	
	printf("BFS order: ");
	for(i=0; i<G->num; i++)
		printf("%"STR(FINT)", ", G->bfs[i]->id);
	printf("\n");

	printf("Looptree: \n");
	H = looptree(G, G->arr[0]);
	print_graphml(H, stdout);

	free_graph(H);
	free_graph(G);
}
