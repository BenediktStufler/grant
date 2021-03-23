/*
 * We provide some offspring distributions for our simulations
 */

int zetalog(mpfr_t res, mpfr_t s, mpfr_t t) {
	INT i;

	mpfr_t con, z, zold, prod, mi, mip, po1, po2, lo, bound, diff, quot;


	mpfr_init2(con, PREC);
	mpfr_init2(z, PREC);
	mpfr_init2(zold, PREC);
	mpfr_init2(prod, PREC);
	mpfr_init2(mi, PREC);
	mpfr_init2(mip, PREC);
	mpfr_init2(po1, PREC);
	mpfr_init2(po2, PREC);
	mpfr_init2(lo, PREC);
	mpfr_init2(bound, PREC);
	mpfr_init2(diff, PREC);
	mpfr_init2(quot, PREC);
	

	mpfr_set_ld(con, 1.0, MPFR_RNDN);

	if(mpfr_less_p(s, con)) {
		fprintf(stderr, "Function zetalog called with invalid argument\n");
		exit(-1);
	}

	// set a reasonable precision bound	
	mpfr_set_ld(bound, 1.e-10, MPFR_RNDN);

	
	/*	
	i = 1;
	z = 0.0;
	zold = 0.0;
	do {
		zold = z;
		z += 1.0 / ( pow((DOUBLE) i, s) * pow(log( (DOUBLE) (i+1)), t));
		i++;
	} while(z - zold >= 1.e-10);
	*/
	i = 1;
	mpfr_set_ld(z, 0.0, MPFR_RNDN);
	mpfr_set_ld(zold, 0.0, MPFR_RNDN);
	do {
		mpfr_set(zold, z, MPFR_RNDN);

		mpfr_set_ui(mi,i, MPFR_RNDN);
		mpfr_pow(po1, mi, s, MPFR_RNDN);	

		mpfr_add(mip, mi, con, MPFR_RNDN);
		mpfr_log(lo, mip, MPFR_RNDN);
		mpfr_pow(po2, lo, t, MPFR_RNDN);	

		mpfr_mul(prod, po1, po2, MPFR_RNDN);
		mpfr_div(quot, con, prod, MPFR_RNDN);
		mpfr_add(z, z, quot, MPFR_RNDN);

		i++;
		mpfr_sub(diff, z, zold, MPFR_RNDN);	
	} while( mpfr_greater_p(diff, bound) );

	mpfr_set(res, z, MPFR_RNDN);

	// clean up
	mpfr_clear(con);
	mpfr_clear(z);
	mpfr_clear(zold);
	mpfr_clear(prod);
	mpfr_clear(mi);
	mpfr_clear(mip);
	mpfr_clear(po1);
	mpfr_clear(po2);
	mpfr_clear(lo);
	mpfr_clear(bound);
	mpfr_clear(diff);
	mpfr_clear(quot);

	return 0;
}

/*
 * Provides a triangulation type offspring law
 *		xi[i] = const * (i+1) * (i+2) * (1/4)**i
 */
mpfr_t *xitria(INT n) {
	mpfr_t *xi;
	INT i;
	mpfr_t norm, mpi, con1, con2, base, ip1, ip2, fac1, fac2;

	// initialize (warning: mpfr sets to NaN, gmp sets to 0.0)
	mpfr_init2(norm, PREC);
	mpfr_init2(mpi, PREC);
	mpfr_init2(con1, PREC);
	mpfr_init2(con2, PREC);
	mpfr_init2(base, PREC);
	mpfr_init2(ip1, PREC);
	mpfr_init2(ip2, PREC);
	mpfr_init2(fac1, PREC);
	mpfr_init2(fac2, PREC);

	xi = (mpfr_t *) calloc(n, sizeof(mpfr_t));
	if(xi == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function xitria\n");
		exit(-1);
	}
	for(i=0; i<n; i++)
		mpfr_init2(xi[i], PREC);

	/*
	for(i=0; i<n; i++) {
		xi[i] = (i+1) * (i+2) * (1/4)**i
	}
	*/
	mpfr_set_ld(con1, 1.0, MPFR_RNDN);
	mpfr_set_ld(con2, 2.0, MPFR_RNDN);
	mpfr_set_ld(base, 0.25, MPFR_RNDN);

	mpfr_set_ld(xi[0], 2.0, MPFR_RNDN);
	for(i=1; i<n; i++) {
		mpfr_set_ui(mpi, i, MPFR_RNDN);
		mpfr_add(ip1, mpi, con1, MPFR_RNDN);
		mpfr_add(ip2, mpi, con2, MPFR_RNDN);
		mpfr_mul(fac1, ip1, ip2, MPFR_RNDN); 
		mpfr_pow(fac2, base, mpi, MPFR_RNDN);
		mpfr_mul(xi[i], fac1, fac2, MPFR_RNDN); 
	}

	// normalize 
	/*
	for(i=0, norm=0.0; i<n; i++)
		norm += xi[i];
	for(i=0; i<n; i++)
		xi[i] /= norm;
	*/

	mpfr_set_ld(norm, 0.0, MPFR_RNDN);	
	for(i=0; i<n; i++)
		mpfr_add(norm, norm, xi[i], MPFR_RNDN);

	for(i=0; i<n; i++)
		mpfr_div(xi[i], xi[i], norm, MPFR_RNDN);

	//DEBUG
	/*
	DOUBLE tmp;
	for(i=0; i<n; i++) {
		tmp = mpfr_get_ld(xi[i], MPFR_RNDN);
		printf("p[%" STR(FINT) "] = %" STR(FDOUBLE) "\n", i, tmp);
	}
	*/
	

	// clean up
	mpfr_clear(norm);
	mpfr_clear(mpi);
	mpfr_clear(con1);
	mpfr_clear(con2);
	mpfr_clear(base);
	mpfr_clear(ip1);
	mpfr_clear(ip2);
	mpfr_clear(fac1);
	mpfr_clear(fac2);


	return xi;
}




/*
 * Provides a power law offspring distribution
 *		xi[i] = const / i^beta	
 * with a given mean 
 *		mu > 0
 * and exponent
 *		beta > 2
 */
mpfr_t *xipow(INT n, DOUBLE beta, DOUBLE mu) {
	mpfr_t *xi;
	INT i;
	mpfr_t c, norm, mpmu, mpbeta, zet, con, prod, expo, po, mpi;

	// initialize (warning: mpfr sets to NaN, gmp sets to 0.0)
	mpfr_init2(c, PREC);
	mpfr_init2(norm, PREC);
	mpfr_init2(mpmu, PREC);
	mpfr_init2(mpbeta, PREC);
	mpfr_init2(zet, PREC);
	mpfr_init2(con, PREC);
	mpfr_init2(prod, PREC);
	mpfr_init2(expo, PREC);
	mpfr_init2(po, PREC);
	mpfr_init2(mpi, PREC);

	xi = (mpfr_t *) calloc(n, sizeof(mpfr_t));
	if(xi == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function xipow\n");
		exit(-1);
	}
	for(i=0; i<n; i++)
		mpfr_init2(xi[i], PREC);

	//c = mu / gsl_sf_zeta(beta - 1.0);
	mpfr_set_ld(mpbeta, beta, MPFR_RNDN);
	mpfr_set_ld(con, 1.0, MPFR_RNDN);
	mpfr_sub(expo, mpbeta, con, MPFR_RNDN);
	mpfr_zeta(zet, expo, MPFR_RNDN);
	mpfr_set_ld(mpmu, mu, MPFR_RNDN);
	mpfr_div(c, mpmu, zet, MPFR_RNDN);

	/*
	xi[0] = 1.0 - c * gsl_sf_zeta(beta);
	for(i=1; i<n; i++) {
		xi[i] = c / pow( (DOUBLE) i, beta);
	}
	*/
	mpfr_zeta(zet, mpbeta, MPFR_RNDN);
	mpfr_mul(prod, c, zet, MPFR_RNDN); 
	mpfr_sub(xi[0], con, prod, MPFR_RNDN);

	for(i=1; i<n; i++) {
		mpfr_set_ui(mpi, i, MPFR_RNDN);
		mpfr_pow(po, mpi, mpbeta, MPFR_RNDN);
		mpfr_div(xi[i], c, po, MPFR_RNDN);
	}

	// normalize 
	/*
	for(i=0, norm=0.0; i<n; i++)
		norm += xi[i];
	for(i=0; i<n; i++)
		xi[i] /= norm;
	*/

	mpfr_set_ld(norm, 0.0, MPFR_RNDN);	
	for(i=0; i<n; i++)
		mpfr_add(norm, norm, xi[i], MPFR_RNDN);

	for(i=0; i<n; i++)
		mpfr_div(xi[i], xi[i], norm, MPFR_RNDN);


	// clean up
	mpfr_clear(c);
	mpfr_clear(norm);
	mpfr_clear(mpmu);
	mpfr_clear(mpbeta);
	mpfr_clear(zet);
	mpfr_clear(con);
	mpfr_clear(prod);
	mpfr_clear(expo);
	mpfr_clear(po);
	mpfr_clear(mpi);


	return xi;
}


/*
 * Provides an offspring distribution that lies in the domain
 * of attraction of a Cauchy law
 *
 *						const
 *		xi[i] = ------------------------
 *				  i^2 * ( ln(i+1) )^gamma
 *
 * with mean mu and exponent gamma > 1.0
 */
mpfr_t *xicau(INT n, DOUBLE gamma, DOUBLE mu) {
	mpfr_t *xi;
	INT i;
	mpfr_t c, norm, mpmu, mpgamma, con1, con2, zet, prod, mi, mip, po1, po2, lo;


	mpfr_init2(c, PREC);
	mpfr_init2(norm, PREC);
	mpfr_init2(mpmu, PREC);
	mpfr_init2(mpgamma, PREC);
	mpfr_init2(zet, PREC);
	mpfr_init2(prod, PREC);
	mpfr_init2(con1, PREC);
	mpfr_init2(con2, PREC);
	mpfr_init2(mi, PREC);
	mpfr_init2(mip, PREC);
	mpfr_init2(po1, PREC);
	mpfr_init2(po2, PREC);
	mpfr_init2(lo, PREC);
	
	xi = (mpfr_t *) calloc(n, sizeof(mpfr_t));
	if(xi == NULL) {
		// memory allocation error
		fprintf(stderr, "Memory allocation error in function xipow\n");
		exit(-1);
	}
	for(i=0; i<n; i++)
		mpfr_init2(xi[i], PREC);




	/*
	c = mu / zetalog(1.0, gamma);
	xi[0] = 1.0 - c * zetalog(2.0, gamma);
	for(i=1; i<n; i++) 
		xi[i] = c / ( pow( (DOUBLE) i, 2.0) * pow(log((DOUBLE) i+1), gamma));
	*/
	mpfr_set_ld(con1, 1.0, MPFR_RNDN);
	mpfr_set_ld(mpmu, mu, MPFR_RNDN);
	mpfr_set_ld(mpgamma, gamma, MPFR_RNDN);
	zetalog(zet, con1, mpgamma);
	mpfr_div(c, mpmu, zet, MPFR_RNDN);
	
	mpfr_set_ld(con2, 2.0, MPFR_RNDN);
	zetalog(zet, con2, mpgamma);
	mpfr_mul(prod, c, zet, MPFR_RNDN);
	mpfr_sub(xi[0], con1, prod, MPFR_RNDN);

	for(i=1; i<n; i++) {
		mpfr_set_ui(mi, i, MPFR_RNDN);
		mpfr_add(mip, mi, con1, MPFR_RNDN);
		mpfr_pow(po1, mi, con2, MPFR_RNDN);	
		mpfr_log(lo, mip, MPFR_RNDN);
		mpfr_pow(po2, lo, mpgamma, MPFR_RNDN);	
		mpfr_mul(prod, po1, po2, MPFR_RNDN);
		mpfr_div(xi[i], c, prod, MPFR_RNDN);
	}


	// normalize 
	/*
	for(i=0, norm=0.0; i<n; i++)
		norm += xi[i];
	for(i=0; i<n; i++)
		xi[i] /= norm;
	*/
	
	mpfr_set_ld(norm, 0.0, MPFR_RNDN);	
	for(i=0; i<n; i++)
		mpfr_add(norm, norm, xi[i], MPFR_RNDN);

	for(i=0; i<n; i++)
		mpfr_div(xi[i], xi[i], norm, MPFR_RNDN);


	// clean up
	mpfr_clear(c);
	mpfr_clear(norm);
	mpfr_clear(mpmu);
	mpfr_clear(mpgamma);
	mpfr_clear(zet);
	mpfr_clear(prod);
	mpfr_clear(con1);
	mpfr_clear(con2);
	mpfr_clear(mi);
	mpfr_clear(mip);
	mpfr_clear(po1);
	mpfr_clear(po2);
	mpfr_clear(lo);
	


	return xi;
}



