/*
 * We provide some offspring distributions for our simulations
 */

/*
 *
 * Estimate the Riemann Zeta function
 *                         1       1
 *        zeta(s) = 1  +  ---  +  ---  +  ...
 *                        2^s     3^s
 */

DOUBLE zeta(DOUBLE s) {
	INT i;
	DOUBLE z;
	DOUBLE zold;

	if(s <= 1.0) {
        fprintf(stderr, "Zeta function called with invalid argument\n");
        exit(-1);
    }

	i = 1;
	z = 0.0;
	zold = 0.0;
	do {
		zold = z;
		z += 1.0 / pow( (DOUBLE) i, s);
		i++;
	} while(z - zold >= 1.e-10);

	return z;
}

DOUBLE zetalog(DOUBLE s, DOUBLE t) {
	INT i;
	DOUBLE z;
	DOUBLE zold;

	if(s < 1.0) {
        fprintf(stderr, "Function zetalog called with invalid argument\n");
        exit(-1);
    }

	i = 1;
	z = 0.0;
	zold = 0.0;
	do {
		zold = z;
		z += 1.0 / ( pow((DOUBLE) i, s) * pow(log( (DOUBLE) (i+1)), t));
		i++;
	} while(z - zold >= 1.e-10);

	return z;
}

/*
 * Provides a power law offspring distribution
 *        xi[i] = const / i^beta	
 * with a given mean 
 *        mu > 0
 * and exponent
 *        beta > 2
 */
DOUBLE *xipow(INT n, DOUBLE beta, DOUBLE mu) {
    DOUBLE *xi;
    INT i;
	DOUBLE c = 0.0;
	DOUBLE norm = 0.0;


    xi = (DOUBLE *) calloc(n, sizeof(DOUBLE));
    if(xi == NULL) {
        // memory allocation error
        fprintf(stderr, "Memory allocation error in function xipow\n");
        exit(-1);
    }


	c = mu / zeta(beta - 1.0);
	xi[0] = 1.0 - c * zeta(beta);
    for(i=1; i<n; i++) {
        xi[i] = c / pow( (DOUBLE) i, beta);
	}

	// normalize 
	for(i=0, norm=0.0; i<n; i++)
		norm += xi[i];
	for(i=0; i<n; i++)
		xi[i] /= norm;

    return xi;
}


/*
 * Provides an offspring distribution that lies in the domain
 * of attraction of a Cauchy law
 *
 *                        const
 *        xi[i] = ------------------------
 *                  i * ( ln(i+1) )^gamma
 *
 * with mean mu and exponent gamma > 1.0
 */
DOUBLE *xicau(INT n, DOUBLE gamma, DOUBLE mu) {
    DOUBLE *xi;
    INT i;
	DOUBLE c = 0.0;
	DOUBLE norm = 0.0;


    xi = (DOUBLE *) calloc(n, sizeof(DOUBLE));
    if(xi == NULL) {
        // memory allocation error
        fprintf(stderr, "Memory allocation error in function xicau\n");
        exit(-1);
    }


	c = mu / zetalog(1.0, gamma);
	xi[0] = 1.0 - c * zetalog(2.0, gamma);
	for(i=1; i<n; i++) 
        xi[i] = c / ( pow( (DOUBLE) i, 2.0) * pow(log((DOUBLE) i+1), gamma));


	// normalize
	for(i=0, norm=0.0; i<n; i++)
		norm += xi[i];
	for(i=0; i<n; i++)
		xi[i] /= norm;

    return xi;
}



