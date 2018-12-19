/* 
The following is taken from the graphml reference:

"Elements an application can't handle are ignored, i.e. a GraphML document is interpreted as if consisting only of those elements known and visible to the application. In particular:

	The elements <port>, <hyperedge>, <endpoint>, and <locator> are simply ignored by applications not knowing some, or all of them. The parser may give a warning that an unknown element has been encountered.
   
	There is no canonical way how applications, capable of handling only one <graph> per document, should handle the case of several <graph>s. So, these applications can choose, either processing only the first <graph>, or taking the union of all <graph>s in the document, or using another fallback policy. In either case, these applications must give a warning and inform the user.

	There is no canonical way how applications, not capable of handling nested <graph>s, should deal with a document using this feature. So, these applications can choose, either ignoring all graph information in deeper levels, or lifting deeper levels to the top level, or using another fallback policy. In either case, these applications must give a warning and inform the user."

In accordance with these rules, this parser takes only the first <graph> object into account, and ignores all nested <graph>s. A warning is written to stderr if multiple or nested <graph>s are encountered. The elements <port>, <hyperedge>, <endpoints>, <locator> are ignored without warning the user.

*/


// the status of the parser
struct pstat {
	struct strgraph *H; // temporary storage for vertices and edges
	int depth;			// depth of current element
	int numgraph;		// how many graph objects at depth 1
	int edefault;		// edgedefault (0 = directed, 1 = undirected)
						// default value is 0
	const char *str_graph;
	const char *str_node;
	const char *str_edge;
	const char *str_hyperedge;
	const char *str_edgedefault;
	const char *str_directed;
	const char *str_undirected;
	const char *str_true;
	const char *str_false;
	const char *str_source;
	const char *str_target;
	const char *str_id;
};

void pushvertex(struct strgraph *H, const char *ident) {
	struct vlist *v;

	// build new list entry
	v = (struct vlist *) malloc(sizeof(struct vlist));
	if(v == NULL) {
		fprintf(stderr, "Error allocating memory for temporary graph structure");
		exit(-1);
	}
	v->next = NULL;

	// set string identifier of vertex
	v->ident = (char *) malloc(sizeof(char)*(strlen(ident) + 1));
	if(v->ident == NULL) {
		fprintf(stderr, "Error allocating memory for temporary graph structure");
		exit(-1);
	}
	strcpy(v->ident, ident);

	

	// set id and insert vertex in list
	if(H->vstart == NULL) {
		v->id = 0;		// vertex is the first in the list
		H->vstart = v;
		H->vend = v;
	} else {
		v->id = H->vend->id +1;
		H->vend->next = v;
		H->vend = v;
	}
}

void pushdiedge(struct strgraph *H, const char *source, const char *target) {
	struct elist *e;

	// build new list entry
	e = (struct elist *) malloc(sizeof(struct elist));
	if(e == NULL) {
		fprintf(stderr, "Error allocating memory for temporary graph structure");
		exit(-1);
	}

	// set source, target, and id of edge
	e->source = (char *) malloc(sizeof(char)*(strlen(source) + 1));
	e->target = (char *) malloc(sizeof(char)*(strlen(target) + 1));
	if(e->source == NULL || e->target == NULL) {
		fprintf(stderr, "Error allocating memory for temporary graph structure");
		exit(-1);
	}
	strcpy(e->source, source);
	strcpy(e->target, target);

	e->next = NULL;
	

	// insert vertex in list
	if(H->estart == NULL) {
		H->estart = e;
		H->eend = e;
	} else {
		H->eend->next = e;
		H->eend = e;
	}

	
}

void pushedge(struct strgraph *H, const char *source, const char *target) {
	pushdiedge(H, source, target);
	pushdiedge(H, target, source);
}

void print_strgraph(struct strgraph *H) {
	struct vlist *v;
	struct elist *e;

	printf("Vertices: \n");
	v =  H->vstart;
	while(v) {
		printf("%"STR(FINT)" - \t%s\n", v->id, v->ident);
		v = v->next;
	}

	printf("Edges: \n");
	e =  H->estart;
	while(e) {
		printf("\t%s -> %s\n", e->source, e->target);
		e = e->next;
	}
}

void free_strgraph(struct strgraph *H) {
	struct vlist *v, *tmpv;
	struct elist *e, *tmpe;

	v = H->vstart;
	while(v) {
		tmpv = v->next;
		free(v->ident);
		free(v);
		v = tmpv;
	}
	
	e = H->estart;
	while(e) {
		tmpe = e->next;
		free(e->source);
		free(e->target);
		free(e);
		e = tmpe;
	}

	free(H);
}

static void XMLCALL start(void *data, const char *el, const char **attr)
{
	int i;
	struct pstat *mydata = (struct pstat *)data;
	int undirected=0;

	const char *source;
	const char *target;

	// we only care about vertices and edges of the first graph (+ no nesting)
	if( (mydata->depth == 2) &&  (mydata->numgraph == 1) ) {
		if( strcmp(el, mydata->str_edge) == 0 ) {
			undirected = mydata->edefault;
			source=NULL;
			target=NULL;

			// check if directed attribute is set
			for (i = 0; attr[i]; i += 2) {
				if( strcmp(attr[i], mydata->str_directed) == 0 ) {
					if( strcmp(attr[i+1], mydata->str_true) == 0) {
						undirected = 0;	
					} else if ( strcmp(attr[i+1], mydata->str_false) == 0) {
						undirected = 1;
					}
				} else if( strcmp(attr[i], mydata->str_source) == 0) {
					source = attr[i+1];
				} else if( strcmp(attr[i], mydata->str_target) == 0) {
					target = attr[i+1];
				}
			}
			
			// add directed edge or undirected edge
			if(undirected == 1) {
				pushedge(mydata->H, source, target);
			} else {
				pushdiedge(mydata->H, source, target);
			}
		} else if ( strcmp(el, mydata->str_node) == 0 ) {
			// check for id
			for (i = 0; attr[i]; i += 2) {
				if( strcmp(attr[i], mydata->str_id) == 0 ) {
					pushvertex(mydata->H, attr[i+1]);
				}
			}
		}
	} 

	// increment number of <graph>s at depth level 1	
	if( (mydata->depth == 1) && (strcmp(el, mydata->str_graph) == 0) ) {
		mydata->numgraph++;

		// read attribute edgedefault
		if(mydata->numgraph == 1) {
			for (i = 0; attr[i]; i += 2)
				if( strcmp(attr[i], mydata->str_edgedefault) == 0 )
					if( strcmp(attr[i+1], mydata->str_undirected) == 0) 
						mydata->edefault = 1;
		}
	
	}
		
	mydata->depth++;
}

static void XMLCALL end(void *data, const char *el)
{
	struct pstat *mydata = (struct pstat *)data;

	mydata->depth--;
}


// should also add functionality to read from file instead of stdin
// struct graph *parsegraphml(FILE *fpointer)
struct graph *parsegraphml(char *infile, char *rootid)
{
	char Buff[8192];
	int done;
	int len;
	FILE *instream;
	struct graph *G;

	struct pstat status = {
		NULL,
		0,
		0,
		0,
		"graph",
		"node",
		"edge",
		"hyperedge",
		"edgedefault",
		"directed",
		"undirected",
		"true",
		"false",
		"source",
		"target",
		"id"
	};
						;

	status.H = (struct strgraph *) malloc(sizeof(struct strgraph));
	if(status.H == NULL) {
		fprintf(stderr, "Couldn't allocate memory for parser\n");
		exit(-1);
	}
	status.H->vstart=NULL;
	status.H->vend=NULL;
	status.H->estart=NULL;
	status.H->eend=NULL;


	XML_Parser parser = XML_ParserCreate(NULL);
	if (!parser) {
		fprintf(stderr, "Couldn't allocate memory for parser\n");
		exit(-1);
	}


	// open input file if necessary
	if(infile == NULL) {
		instream = stdin;
	} else {
		instream = fopen(infile, "r");
		if(instream == NULL) {
			fprintf(stderr, "Error opening input file.\n");
			exit(-1);
		}
	}


	// the function start is called whenever a new element starts
	// the function end is called whenever an element ends
	// the address of pstat is passed to these functions in those events
	XML_SetUserData(parser, &status);
	XML_SetElementHandler(parser, start, end);

	while(1) {
		len = (int)fread(Buff, 1, 8192, instream);
		if (ferror(instream)) {
			fprintf(stderr, "Read error\n");
			exit(-1);
		}
		done = feof(instream);

		if (XML_Parse(parser, Buff, len, done) == XML_STATUS_ERROR) {
			fprintf(stderr, "Parse error at line %lu: %s \n",
							XML_GetCurrentLineNumber(parser),
							XML_ErrorString(XML_GetErrorCode(parser)));
			exit(-1);
		}

		if (done) break;
	}

	XML_ParserFree(parser);


	// close input file if necessary
	if(infile != NULL) fclose(instream);

	/*
	 * At this point, status.H is a pointer to a temporary graph structure
	 * that holds a list of vertex identifiers (strings) and edges (pairs 
	 * of strings).
	 *
	 * The following builds a more efficient graph structure.
	 * (I guess this could still be improved a lot by making it
	 * cache-friendly. I still have much to learn... :-) )
	 */
	G = make_graph(status.H, rootid);	// recall that some edges of H may point to
 								// nowhere, if an endpoint lies in a 
 								// nested <graph>
	free_strgraph(status.H);
	return G;

}
