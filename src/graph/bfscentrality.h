/*
 *	We calculate the closeness centrality of the vertices of a graph 
 *  using a simple breadth-first-search based algorithm
 *
 *  The workload is distributed on multiple threads
 *
 */




struct stat {
	INT status;		// determines if we already queued the vertex in the past
	INTD dist;		// the distance of the vertex in the current BFS
};

// data that gets passed to a thread
struct gsegment {
	struct graph *G;
	INT start;
	INT end;
	int success;
};


// calculate closeness centrality of vertices with ids start, start+1, ..., end-1
void *centrality(void *seg) {
	struct graph *G = ((struct gsegment *)seg)->G;
	INT start = ((struct gsegment *)seg)->start;
	INT end = ((struct gsegment *)seg)->end;

	struct stat *arr;		// array that holds status information
	struct vertex **queue;	// we realize the queue as a static array to
							// avoid repeated memory allocation & deallocation
	struct list *li;
	INT i,j;
	INT num = G->num;		// number of vertices in our graph
	INTD dist;

	INT pop;			// index for queue



	//check for sanity of arguments
	if(start < 0 || end > num) {
		fprintf(stderr, "Argument out of range error in function centrality\n");
		return (void *) -1;
	}
	if(end <= start) {
		return (void *) 0;
	}
	
	// build helper arrays
	// make sure to compile code with -pthreads to make these calls thread-safe
	arr = calloc(sizeof(struct stat), num);
	queue = calloc(sizeof(struct vertex *), num);
	if(arr == NULL  || queue == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function centrality\n");
		return (void *) -1;
	}

	// initialize helper arrays
	for(i=0; i<num; i++) {
		arr[i].status = -1;
	}



	// calculate closeness centrality
	// this is the part that needs to be as fast as possible
	for(i=start; i<end; i++) {

		dist=0;		// this variable will denote the sum of all distances
		queue[0] = G->arr[i];		// queue the starting vertex		
		arr[i].status = i;			// mark the starting vertex as queued
		arr[i].dist = 0;			// the starting vertex has distance zero
									// from itself

		// need j!= pop check for disconnected grpahs
		for(j=0, pop=1; j<num && j!=pop; j++) {		// iterate over the entire queue
			dist += arr[queue[j]->id].dist;	// add contribution of vertex to dist

			// queue all neighbours of our vertex that have not been queued yet
			for(li = queue[j]->qu->li; li != NULL; li = li->ne) {
				if( arr[li->ve->id].status != i) {	// vertex yet unqueued
					queue[pop] = li->ve;			// queue vertex
					pop++;	// increment index for queue location

					arr[li->ve->id].status = i;	// mark vertex as queued
					arr[li->ve->id].dist = arr[queue[j]->id].dist + 1; //dist
				}
			}
		}
			// save distance sum of vertex
			G->arr[i]->cent = dist;
	}

	// free helper arrays
	free(arr);
	free(queue);

	return (void *) 0;
}

int threadedcentrality(struct graph *G, INT start, INT end, INT numThreads) {
	INT chunkSize;				// roughly how many vertices each thread
								// has to take care of
	struct gsegment *segList;	// arguments for the separate threads
	pthread_t *th;				// array of threads
	INT i;
	void *ret;

	INT *boxes;


	/* sanity checks */
	if(numThreads <= 0) return -1;
	if(end <= start) return 0;
	if(G->num < end) return -1;
		   

	/* divide the workload */
	boxes = (INT *) calloc(numThreads, sizeof(INT));
	segList = (struct gsegment *) calloc(numThreads, sizeof(struct gsegment));
	if(boxes == NULL || segList == NULL) {
		fprintf(stderr, "Error allocating memory in function threadedcentrality.\n");
		exit(-1);
	}

	// end - start - numThreads + 1 <= numThreads * chunkSize <= end - start
	chunkSize = (end - start) / numThreads;

	// distribute (end - start) balls in numThreads boxes
	// so that each box has roughly the same number of balls
	for(i=0; i < numThreads; i++)
		boxes[i] = chunkSize;
	for(i=0; i < end - start - chunkSize*numThreads; i++)
		boxes[i] += 1;

	// "stack" the boxes
	boxes[0] += start;
	for(i=1; i < numThreads; i++)
		boxes[i] += boxes[i-1];

	// set segment
	segList[0].start = start;
	segList[0].end = boxes[0];
	segList[0].G = G;
	for(i=1; i<numThreads; i++) {
		segList[i].start = boxes[i-1];
		segList[i].end = boxes[i];
		segList[i].G = G;
	}

	free(boxes);

	/* launch threads */	
	th = calloc(sizeof(pthread_t), numThreads);
	if(th == NULL) {
		fprintf(stderr, "Error allocating memory in function threadedcentrality.\n");
		exit(-1);
	}

	for(i=0; i<numThreads; i++) {
		if(pthread_create(&th[i], NULL, &centrality, &segList[i] )) {
			fprintf(stderr, "Error launching thread number %"STR(FINT)"\n", i);
			return -1;
		}
	}


	/* wait for threads to finish */
	for(i=0; i<numThreads; i++) {
		pthread_join(th[i], &ret);
		if(ret) {
			fprintf(stderr, "Error executing thread number %"STR(FINT)"\n", i);
			return -1;
		}	
	}

	/* clean up */
	free(segList);
	free(th);	

	return 0;
}

