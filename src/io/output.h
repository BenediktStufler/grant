/*
 * Provides functions that output the simulated data
 */


/*
 * Write vertex outdegree profile to file
 */
int outdegprofile(INT *N, INT size, char *outfile) {
	FILE *outstream;	
	INT i, j;

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			return(-1);
		}
	}

	// output vertex degree profile
	fprintf(outstream, "{\n");
	for(i=0; i<size; i++) {
		if(N[i]>0) {
			fprintf(outstream, "N[%" STR(FINT) "] = %"STR(FINT), i, N[i]);
			for(j = i+1; j<size; j++)
				if(N[j]>0) fprintf(outstream, ",\nN[%" STR(FINT) "] = %"STR(FINT), j, N[j]);
			break;
		}
	}
	fprintf(outstream, "\n}\n");

	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}

/*
 * Write maximal outdegree to file
 */
int outmdeg(INT *N, INT size, char *outfile) {
	FILE *outstream;	
	INT i;

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			return(-1);
		}
	}

	// output maximal degree
	for(i=size; i > 0; i--) {
		if(N[i-1]>0) {
			fprintf(outstream, "%"STR(FINT)"\n", i-1);
			break;
		}
	}

	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}





// counts the number of digits of an integer
unsigned int count_digits(int arg) {
	return snprintf(NULL, 0, "%d", arg) - (arg < 0);
}

// make string out of integer
char *uint2string(unsigned int p) {
	unsigned int digits;
	char *res;
	digits = count_digits(p);

	res = (char *) calloc(digits + 1, sizeof(char));
	if(res == NULL) {
		fprintf(stderr, "Memory allocation error in function unit2string.\n");
		exit(-1);
	}

	snprintf(res, digits + 1, "%u", p);

	return res;
}

// make string out of integer and fill with zeros
char *uint2zstring(unsigned int p, unsigned int digits) {
	char *res;
	char *format;
	char *tmp;

	if(digits < count_digits(p)) {
		// undefined behaviour
		return NULL;
	}

	format = (char *) calloc(4 + count_digits(digits) , sizeof(char));
	res = (char *) calloc(digits + 1, sizeof(char));
	if(format == NULL || res == NULL) {
		fprintf(stderr, "Memory allocation error in function uint2zstring.\n");
		exit(-1);
	}

	// build format string
	// example: format = "%u0555" if digits = 555
	strcpy(format, "%0");
	tmp = uint2string(digits);
	strcat(format, tmp);
	free(tmp);
	strcat(format, "u");

	// build string representation
	snprintf(res, digits+1, format, p);

	free(format);

	return res;
}

// returns a dynamically allocated string with the correct filename
char *convname(char *outfile, unsigned int counter, unsigned int num, int Tnum) {
	unsigned int digispace, loc;
	char *paddednumber, *fname;
	unsigned int len;

	len=strlen(outfile);

	// check for empty string
	if(len == 0) {
		fprintf(stderr, "Error. Received empty filename.\n");
		exit(-1);
	}

	// find location of % symbol 
	for(loc=0; loc<len; loc++) {
		if( outfile[loc] == '%' ) break;
	}

	if( Tnum && loc < len ) {
		// number of digits required to display num
		digispace = count_digits(num);

		// save to unique filenames
		fname = (char *) calloc(len + digispace + 1, sizeof(char)); 
		if(fname == NULL) {
			fprintf(stderr, "Memory allocation error in function convname.\n");
			exit(-1);
		}

		strncpy(fname, outfile, loc);
		strcpy(fname + loc + digispace, outfile + loc + 1);
		paddednumber = uint2zstring(counter, digispace);
		strncpy(fname + loc, paddednumber, digispace);
		free(paddednumber);
	} else {
		fname = (char *) calloc(len + 1, sizeof(char)); 
		if(fname == NULL) {
			fprintf(stderr, "Memory allocation error in function convname.\n");
			exit(-1);
		}
		strcpy(fname, outfile);
	}
	
	return fname;
}

/*
 * Output graph to graphml format
 */
int outgraph(struct graph *G, char *outfile) {
	FILE *outstream;	

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			exit(-1);
		}
	}

	print_graphml(G, outstream);
	
	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}


/*
 * Output degree sequence
 */
int outdegseq(struct graph *G, char *outfile) {
	FILE *outstream;	
	INT i;

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			exit(-1);
		}
	}

	// output degree sequence
	fprintf(outstream, "{");
	for(i=0; i<G->num-1; i++) {
		fprintf(outstream, "%"STR(FINT)", ", G->arr[i]->deg);	
	}
	if(G->num>0) fprintf(outstream, "%"STR(FINT), G->arr[G->num - 1]->deg);	
	fprintf(outstream, "}\n");
	
	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}

/*
 * Output maximal degree
 */
int outmdeggraph(struct graph *G, char *outfile) {
	FILE *outstream;	
	INT i;
	INT max;

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			exit(-1);
		}
	}

	// output maximal degree sequence
	for(i=0, max=0; i<G->num; i++) {
		if(G->arr[i]->deg > max) max = G->arr[i]->deg;
	}
	fprintf(outstream, "%"STR(FINT)"\n", max);	
	
	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}


/*
 * Output height sequence
 */
int outheightseq(struct graph *G, char *outfile) {
	FILE *outstream;	
	INT i;

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			exit(-1);
		}
	}

	fprintf(outstream, "{");
	for(i=0; i<G->num-1; i++) {
		fprintf(outstream, "%"STR(FINT)", ", G->arr[i]->height);	
	}
	if(G->num>0) fprintf(outstream, "%"STR(FINT), G->arr[G->num - 1]->height);	
	fprintf(outstream, "}\n");
	
	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}




/*
 * Write a sequence of values to a file or stdout
 */
int outseq(void *seq, INT size, char *outfile, int format) {
	FILE *outstream;	
	INT i;

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file %s.\n", outfile);
			exit(-1);
		}
	}


	// output values
	fprintf(outstream, "{");
	for(i=0; i<size-1; i++) {
		switch(format) {
			case 1:
				// INT
				fprintf(outstream, "%" STR(FINT) ", ", ((INT *) seq)[i]);
				break;
			case 2:
				// INTD
				fprintf(outstream, "%" STR(FINTD) ", ", ((INTD *) seq)[i]);
				break;
			case 3:
				// DOUBLE
				fprintf(outstream, "%" STR(FDOUBLE) ", ", ((DOUBLE *) seq)[i]);
				break;
			default:
				fprintf(stderr, "Error, unknown format in function outseq.\n");
				exit(-1);
		}
	}
	if(size>0) {
		// print last element without comma
		switch(format) {
			case 1:
				// INT
				fprintf(outstream, "%" STR(FINT), ((INT *) seq)[i]);
				break;
			case 2:
				// INTD
				fprintf(outstream, "%" STR(FINTD), ((INTD *) seq)[i]);
				break;
			case 3:
				// DOUBLE
				fprintf(outstream, "%" STR(FDOUBLE), ((DOUBLE *) seq)[i]);
				break;
			default:
				fprintf(stderr, "Error, unknown format in function outseq.\n");
				exit(-1);
		}
	}
	fprintf(outstream, "}\n");

	// close file if necessary
	if(outfile != NULL) fclose(outstream);

	return 0;
}



// output closeness centrality 
int outcent(struct graph *G, char *outfile) {
	INT i;
	double num = (double) (G->num-1);
	FILE *outstream;
	INT start = 0;
	INT end = G->num;

	//check for sanity of arguments
	if(start < 0 || end > G->num) {
		fprintf(stderr, "Argument out of range error in function outcent\n");
		return -1;
	}
	if(end <= start) {
		return 0;
	}

	// open output file if necessary
	if(outfile == NULL || strlen(outfile) == 0) {
		outstream = stdout;
	} else {
		outstream = fopen(outfile, "a");
		if(outstream == NULL) {
			fprintf(stderr, "Error opening output file.\n");
			exit(-1);
		}
	}

	// output closeness centrality of vertices
	fprintf(outstream, "{");
	for(i=start; i<end-1; i++) {
		fprintf(outstream, "%17.17f, ", num / (double) G->arr[i]->cent);	
	}
	if(end>0) fprintf(outstream, "%17.17f", num / (double) G->arr[end-1]->cent);	
	fprintf(outstream, "}\n");

	// close file if necessary
	if(outfile != NULL) fclose(outstream);	

	return 0;
}


