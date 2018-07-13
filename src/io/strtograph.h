/*
 *
 * We use a simple hashmap to build a graph structure out of a list of vertices
 * and edges that are given by strings and pairs of strings, respectively.
 *
 */



/*
 * Temporary and inefficient data structure for a graph
 * Vertices are given by a list of strings
 * Edges are given by a list of pairs of strings
 *
 * Use for I/O purposes only
 */

struct elist {
	char *source;
	char *target;
	struct elist *next;
};

struct vlist {
	INT id;
	char *ident;
	struct vlist *next;
};

struct strgraph {
	struct vlist *vstart;
	struct vlist *vend;
	struct elist *estart;
	struct elist *eend;
};


struct bucket {
	struct vlist *vstart;
	struct vlist *vend;
};

void push2bucket(struct bucket *B, struct vlist *vert) {
	struct vlist *v;

	// create copy of vertex
	v = (struct vlist *) malloc(sizeof(struct vlist));
	if(v == NULL) {
		fprintf(stderr, "Error allocating memory for hashmap");
		exit(-1);
	}
	v->next = NULL;
	v->ident = (char *) malloc(sizeof(char)*(strlen(vert->ident) + 1));
	if(v->ident == NULL) {
		fprintf(stderr, "Error allocating memory for hashmap.\n");
		exit(-1);
	}
	strcpy(v->ident, vert->ident);
	v->id = vert->id;


	// put vertex into bucket
	if(B->vstart == NULL) {
		B->vstart = v;
		B->vend = v;
	} else {
		B->vend->next = v;
		B->vend = v;
	}
}


unsigned long long djb2hash(char *str, INT modulo) {
	unsigned long long hash = 5381;
	int c;

	while( (c = *str++) ) {
		hash = ((hash << 5) + hash) + c; // hash*33 + c
	}

	return hash % modulo;
}

// this function returns id of the vertex with identifier string *str
// if such a vertex does not exist, it returns -1
int getid(char *str, struct bucket *bucketlist, INT len) {
	struct vlist *v;
	INT bucketnumber = djb2hash(str,len);

	if(bucketnumber >= len) {
		fprintf(stderr, "Error looking for a vertex in the hashmap.\n");	
		exit(-1);
	}

	for(v = bucketlist[bucketnumber].vstart; v != NULL; v = v->next)
		if( strcmp(v->ident, str) == 0) return v->id;

	return -1;		// vertex not found
}

void emptybuckets(struct bucket *bucketlist, INT len) {
	INT i;
	struct vlist *v;
	struct vlist *tmpv;

	for(i=0; i<len; i++) {
		v = bucketlist[i].vstart;
		while(v) {
			tmpv = v->next;
			free(v->ident);
			free(v);
			v = tmpv;
		}
	}
	free(bucketlist);
}

struct bucket *ini_buckets(struct strgraph *H) {
	INT i;
	INT len = H->vend->id + 1;
	struct vlist *v;
	struct bucket *bucketlist;

	// create and initialize list of buckets	
	bucketlist = (struct bucket *) calloc(sizeof(struct bucket), len);
	if(bucketlist == NULL) {
		fprintf(stderr, "Error allocating memory for hashmap.");
		exit(-1);
	}
	for(i=0; i<len; i++) {
		bucketlist[i].vstart = NULL;
		bucketlist[i].vend = NULL;
	}

	// push vertices into buckets
	for( v = H->vstart; v != NULL; v = v->next) {
		push2bucket(&bucketlist[djb2hash(v->ident, len)], v);
	}

	return bucketlist;
}

struct graph *ini_graph(struct strgraph *H, struct bucket *bucketlist, char *rootid) {
	struct graph *G;
	struct elist *e;
	INT source;
	INT target;
	INT len = H->vend->id + 1;


	// initialize graph
	G = newgraph(len);

	// add edges	
	for(e = H->estart; e!= NULL; e = e->next) {
		source = getid(e->source, bucketlist, len);
		target = getid(e->target, bucketlist, len);
		
		// we need to check if these vertices were actually found
		if( source != -1 && target != -1) {
			addDiEdge(G->arr[source], G->arr[target]);
		}
	}

	//specify root vertex
	if(rootid) {
		source = getid(rootid, bucketlist, len);
		if( source == -1) {
			fprintf(stderr, "Error: could not find specified root vertex id in input file.\n");
			exit(-1);
		}
		G->root = G->arr[source];
	}

	return G;
}


struct graph *make_graph(struct strgraph *H, char *rootid) {
	struct bucket *bucketlist;		// the hashmap
	struct graph *G;
	INT len = H->vend->id + 1;


	// sanity check: exit if graph is empty
	if( H == NULL || H->vstart == NULL) {
		fprintf(stderr, "Error: received empty graph\n");
		exit(-1);
	}

	// initialize hashmap
	bucketlist = ini_buckets(H);

	// create graph structure
	G = ini_graph(H, bucketlist, rootid);

	// free allocated memory for hashmap
	emptybuckets(bucketlist, len);

	return G;
}
