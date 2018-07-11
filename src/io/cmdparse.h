const char *argp_program_version =
"grant 1.0";

const char *argp_program_bug_address =
"<benedikt.stufler@posteo.net>";


/* This structure is used by main to communicate with parse_opt. */
struct cmdarg
{
	int threads;				// number of threads we are going to launch
	unsigned long int seed;		// seed for the random generators
	INT size;					// target size of random tree
	int method;					// model of random trees
								// 1 = GW tree conditioned on number of vertices
	const gsl_rng_type *randgen;	// type of random generator

	DOUBLE beta;				// parameter for distribution
	int Tbeta;					// has value been set by the user?

	DOUBLE gamma;				// parameter for distribution
	int Tgamma;					// has value been set by the user?

	DOUBLE mu;					// paramter for distribution
	int Tmu;					// has value been set by the user?

	char *outfile;				// file to which we write tree 
	int Toutfile;				// has value been set by the user?

	char *loopfile;				// file to which we write looptree
	int Tloopfile;				// has value been set by the user?

	char *heightfile;			// file to which we write height profile
	int Theightfile;			// has value been set by the user?

	char *degfile;				// file to which we write 
	int Tdegfile;				// has value been set by the user?

	char *profile;				// file to which we write outdegree statistics
	int Tprofile;				// has value been set by the user?

	char *pdprofile;			// file to which we write degree statistics
	char Tpdprofile;			// has value been set by the user?

	char *centfile;				// file to which we write closeness centrality
								// of vertices
	int Tcentfile;				// has value been set by the user?

	char *infile;				// file from which we read the graph
	int Tinfile;				// has value been set by the user?

	char *vid;					// root vertex id

	unsigned int num;			// number of samples
	int Tnum;					// has value been set by the user?
};



/*
 * Functions that provide default values for parameters
 */


/*
 * use system timestamp (in seconds) as initial random seed
 */
unsigned int getseed(void) {
	return time(NULL);
}

/*
 * use number of cpu cores as default value for number of threads
 * adapted from code by Philip Willoughby
 * https://lists.gnu.org/archive/html/autoconf/2002-08/msg00126.html
 * 
 */
unsigned int getnumcores(void) {
	int nprocs_max = -1;
	
	#ifdef _WIN32
		#ifndef _SC_NPROCESSORS_ONLN
			SYSTEM_INFO info;
			GetSystemInfo(&info);
			#define sysconf(a) info.dwNumberOfProcessors
			#define _SC_NPROCESSORS_ONLN
		#endif
	#endif

	#ifdef _SC_NPROCESSORS_ONLN
		nprocs_max = sysconf(_SC_NPROCESSORS_CONF);
		if (nprocs_max < 1) {
			fprintf(stderr, "Could not determine number of CPUs configured:\n%s\n. Falling back to default value 1.\n", strerror (errno));
			return 1;
		}
		return nprocs_max;
	#else
		fprintf(stderr, "Could not determine number of CPUs. Falling back to default value 1.\n");
  		return 1;
	#endif
}


/*
	 OPTIONS.   Field 1 in ARGP.
	 Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
*/
static struct argp_option options[] =
{
	{"size",		's', "SIZE", 0, 	"Simulate a Galton--Watson tree conditioned on having SIZE vertices."},
	{"outfile",		'o', "OUTFILE", 0, 	"Output simulated random tree in the graphml format to OUTFILE."},
	{"mu", 			'm', "MU", 0, 		"Simulate with an offspring distribution that has average value MU."},
	{"num", 		'N', "NUM", 0, 		"Simulate NUM many samples. Requires the use of the % symbol in all specified output filenames. For example, --num=100 --outfile=tree%.graphml will create the files tree001.graphml, tree002.graphml, ..., tree100.graphml."},
	{"beta", 		'b', "BETA", 0, 	"Simulate a branching mechanism with a power law P(k) = const / k^{BETA}. Requires the -mu option."},
	{"gamma", 		'g', "GAMMA", 0, "Simulate a branching mechanism with distribution P(k) = const / ( k^2 * ln^GAMMA(k+1) ) for k >= 1. Requires the -mu option."},
	{"threads", 	't', "THREADS", 0,	"Distribute the workload on THREADS many threads. The default value is the number of CPU cores."}, 
	{"loopfile",  	'l', "LOOPFILE", 0, "Output the looptree associated to the simulated random tree to LOOPFILE."},
	{"centfile",  	'c', "CENTFILE", 0, "Output a list of the vertices' closeness centrality to CENTFILE."},
	{"degfile",  	'd', "DEGFILE", 0, 	"Output the degrees of the depth-first-search ordered list of vertices to DEGFILE."},
	{"heightfile",  'h', "HEIGHTFILE", 0, "Output the height profile to HEIGHTFILE."},
	{"inputfile",  'i', "INPUTFILE", 0, "Read a _connected_ graph from file INFILE (graphml format) instead of generating it at random."},
	{"profile",  	'p', "PROFILE", 0, 	"Output the degree profile to the file PROFILE."},
	{"vertex",  	'v', "VERTEX", 0, 	"Specify a root vertex. Used in conjunction with the --inputfile parameter. "},
	{"randgen",  	'r', "RANDGEN", 0, 	"Use the pseudo random generator RANDGEN. Available options are taus2, gfsr4, mt19937, ranlux, ranlxs0, ranlxs1, ranlxs2, ranlxd1, ranlxd2, mrg, cmrg, ranlux389. The default is taus2."},
	{"seed", 		'S', "SEED", 0, "Specify the seed of the random generator in the first thread. Thread number k will receive SEED + k - 1 as seed. The default is to set SEED to the systems timestamp (in seconds)."},
	{0}
};


/*
	 PARSER. Field 2 in ARGP.
	 Order of parameters: KEY, ARG, STATE.
*/
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct cmdarg *arguments = state->input;
	const char *strgens[] = {"taus2", "gfsr4", "mt19937", "ranlux", "ranlxs0", "ranlxs1", "ranlxs2", "ranlxd1", "ranlxd2", "mrg", "cmrg", "ranlux389"};
	int num;

	switch (key) {
		case 't':
			// the number of threads we are going to launch
			arguments->threads = (int) strtoimax(arg, NULL, 10);
			if(arguments->threads <= 0) {
				fprintf(stderr, "Error: the number of threads must be positive.\n");
        		exit(-1);
    		}
			break;
		case 's':
			// size parameter for random tree
			arguments->size = (INT) strtoimax(arg, NULL, 10);
			if(arguments->size <= 0) {
				fprintf(stderr, "Error: the SIZE command line argument must be a positive integer.\n");
        		exit(-1);
    		}
			break;
		case 'S':
			// seed for random generator
			arguments->seed = (unsigned int) strtoimax(arg, NULL, 10);
			break;
		case 'r':
			// select random generator
			for(num=0; num<12; num++) {
				if( strcmp(strgens[num],arg) == 0) {
					if(num==0) {
						arguments->randgen = gsl_rng_taus2;
						break;
					} else if (num==1) {
						arguments->randgen = gsl_rng_gfsr4;
						break;
					} else if (num==2) {
						arguments->randgen = gsl_rng_mt19937;
						break;
					} else if (num==3) {
						arguments->randgen = gsl_rng_ranlux;
						break;
					} else if (num==4) {
						arguments->randgen = gsl_rng_ranlxs0;
						break;
					} else if (num==5) {
						arguments->randgen = gsl_rng_ranlxs1;
						break;
					} else if (num==6) {
						arguments->randgen = gsl_rng_ranlxs2;
						break;
					} else if (num==7) {
						arguments->randgen = gsl_rng_ranlxd1;
						break;
					} else if (num==8) {
						arguments->randgen = gsl_rng_ranlxd2;
						break;
					} else if (num==9) {
						arguments->randgen = gsl_rng_mrg;
						break;
					} else if (num==10) {
						arguments->randgen = gsl_rng_cmrg;
						break;
					} else if (num==11) {
						arguments->randgen = gsl_rng_ranlux389;
						break;
					}
				}
			}
			if(num>11) {
				fprintf(stderr, "Error: Invalid value for random number generator.\n");
				exit(-1);
			}

			break;
		case 'b':
			arguments->beta = (DOUBLE) atof(arg);
			arguments->Tbeta = 1;
			break;
		case 'g':
			arguments->gamma = (DOUBLE) atof(arg);
			arguments->Tgamma = 1;
			break;
		case 'm':
			arguments->mu = (DOUBLE) atof(arg);
			arguments->Tmu = 1;
			break;
		case 'o':
			arguments->outfile = arg;
			arguments->Toutfile = 1;
			break;
		case 'i':
			arguments->infile = arg;
			arguments->Tinfile = 1;
			arguments->method = 2;
			break;
		case 'd':
			arguments->degfile = arg;
			arguments->Tdegfile = 1;
			break;
		case 'p':
			arguments->profile = arg;
			arguments->Tprofile = 1;
			break;
		case 'l':
			arguments->loopfile = arg;
			arguments->Tloopfile = 1;
			break;
		case 'c':
			arguments->centfile = arg;
			arguments->Tcentfile = 1;
			break;
		case 'h':
			arguments->heightfile = arg;
			arguments->Theightfile = 1;
			break;
		case 'v':
			arguments->vid = arg;
			break;
		case 'N':
			// the number of samples
			arguments->num = (unsigned int) strtoimax(arg, NULL, 10);
			arguments->Tnum = 1;
			if( arguments->num <= 0 ) {
				fprintf(stderr, "Error: The --num parameter has to be a positive integer.\n");
				exit(-1);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}


/*
	 ARGS_DOC. Field 3 in ARGP.
	 A description of the non-option command-line arguments
		 that we accept.
*/
static char args_doc[] = "";

/*
	DOC.	Field 4 in ARGP.
	Program documentation.
*/
static char doc[] = "grant -- generate random trees";

/*
	 The ARGP structure itself.
*/
static struct argp argp = {options, parse_opt, args_doc, doc};



int getcmdargs(struct cmdarg *comarg, int argc, char **argv) {
	char *c;
	int flag;

	/* set command line arguments defaults */

	// default filenames are NULL and have not been set by the user yet
	comarg->outfile = NULL;
	comarg->Toutfile = 0;

	comarg->degfile = NULL;
	comarg->Tdegfile = 0;

	comarg->loopfile = NULL;
	comarg->Tloopfile = 0;

	comarg->heightfile = NULL;
	comarg->Theightfile = 0;

	comarg->profile = NULL;
	comarg->Tprofile = 0;

	comarg->centfile = NULL;
	comarg->Tcentfile = 0;

	comarg->infile = NULL;
	comarg->Tinfile = 0;

	comarg->vid = NULL;
	
	comarg->size = 1000;
	comarg->num = 1;
	comarg->Tnum = 0;

	comarg->beta = -1.0;
	comarg->Tbeta = 0;
	comarg->gamma = -1.0;
	comarg->Tgamma = 0;
	comarg->mu = -1.0;
	comarg->Tmu = 0;



	// default method is to simulate a Galton-Watson tree conditioned on
	// its size
	comarg->method = 1;

	// the default random generator is the taus2 algorithm
	comarg->randgen = gsl_rng_taus2;

	// default value for seed of random generator
	comarg->seed = getseed();

	// default number of threads is the number of cpu cores
	comarg->threads = getnumcores();

	/* read command line arguments and perform some sanity checks*/
	argp_parse (&argp, argc, argv, 0, 0, comarg);

	/* further sanity checks concerning cross dependencies among parameters */

	// if the --num parameter was set then each output filename 
	// needs to contain the % symbol.
	if(comarg->Tnum) {
		if( comarg->Toutfile ) {
			flag=1;
			for(c = comarg->outfile; *c != '\0'; c++) {
				if( *c == '\045' ) {
					flag=0;
					break;
				}
			}
			if( flag ) {
				fprintf(stderr, "Error: Setting the --num parameter requires you to include the %% symbol in the --outfile parameter (and all other output filenames). It will be replaced by the number of the sample.\n");
				exit(-1);
			}
		}

		if( comarg->Tprofile ) {
			flag=1;
			for(c = comarg->profile; *c != '\0'; c++) {
				if( *c == '\045' ) {
					flag=0;
					break;
				}
			}
			if( flag ) {
				fprintf(stderr, "Error: Setting the --num parameter requires you to include the %% symbol in the --profile parameter (and all other output filenames). It will be replaced by the number of the sample.\n");
				exit(-1);
			}
		}

		if( comarg->Tcentfile ) {
			flag=1;
			for(c = comarg->centfile; *c != '\0'; c++) {
				if( *c == '\045' ) {
					flag=0;
					break;
				}
			}
			if( flag ) {
				fprintf(stderr, "Error: Setting the --num parameter requires you to include the %% symbol in the --centfile parameter (and all other output filenames). It will be replaced by the number of the sample.\n");
				exit(-1);
			}
		}

		if( comarg->Tdegfile ) {
			flag=1;
			for(c = comarg->degfile; *c != '\0'; c++) {
				if( *c == '\045' ) {
					flag=0;
					break;
				}
			}
			if( flag ) {
				fprintf(stderr, "Error: Setting the --num parameter requires you to include the %% symbol in the --degfile parameter (and all other output filenames). It will be replaced by the number of the sample.\n");
				exit(-1);
			}
		}

		if( comarg->Theightfile ) {
			flag=1;
			for(c = comarg->heightfile; *c != '\0'; c++) {
				if( *c == '\045' ) {
					flag=0;
					break;
				}
			}
			if( flag ) {
				fprintf(stderr, "Error: Setting the --num parameter requires you to include the %% symbol in the --heightfile parameter (and all other output filenames). It will be replaced by the number of the sample.\n");
				exit(-1);
			}
		}

		if( comarg->Tloopfile ) {
			flag=1;
			for(c = comarg->loopfile; *c != '\0'; c++) {
				if( *c == '\045' ) {
					flag=0;
					break;
				}
			}
			if( flag ) {
				fprintf(stderr, "Error: Setting the --num parameter requires you to include the %% symbol in the --loopfile parameter (and all other output filenames). It will be replaced by the number of the sample.\n");
				exit(-1);
			}
		}

	}

	return 0;
}

