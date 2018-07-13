GRANT - Generate RANdom Trees 

* 
* 1. Introduction
* 

This project implements efficient samplers for random structures. 

The current version supports the simulation of Galton-Watson trees conditioned on having a fixed number of vertices. The simulation uses a multithreaded version of an algorithm by Devroye (2012, SIAM Journal of Computing). The idea is to sample multinomially distributed random variates many times until a certain stopping condition is reached.

The program may be instructed to output lists of vertex parameters in depth-first-search order (degree profile, height profile, closeness centrality) and calculate the looptree corresponding to the simulated random tree (with identical vertex ids).



*
* 2. Usage
*

Usage: grant [-?V] [-b BETA] [-c CENTFILE] [-d DEGFILE] [-g GAMMA]
            [-h HEIGHTFILE] [-i INPUTFILE] [-l LOOPFILE] [-m MU] [-N NUM]
            [-o OUTFILE] [-p PROFILE] [-r RANDGEN] [-s SIZE] [-S SEED]
            [-t THREADS] [-v VERTEX] [--beta=BETA] [--centfile=CENTFILE]
            [--degfile=DEGFILE] [--gamma=GAMMA] [--heightfile=HEIGHTFILE]
            [--inputfile=INPUTFILE] [--loopfile=LOOPFILE] [--mu=MU] [--num=NUM]
            [--outfile=OUTFILE] [--profile=PROFILE] [--randgen=RANDGEN]
            [--size=SIZE] [--seed=SEED] [--threads=THREADS] [--vertex=VERTEX]
            [--help] [--usage] [--version] 



  -b, --beta=BETA            Simulate a branching mechanism with a power law
                             P(k) = const / k^{BETA}. Requires the -mu option.
  -c, --centfile=CENTFILE    Output a list of the vertices' closeness
                             centrality to CENTFILE.
  -d, --degfile=DEGFILE      Output the degrees of the depth-first-search
                             ordered list of vertices to DEGFILE.
  -g, --gamma=GAMMA          Simulate a branching mechanism with distribution
                             P(k) = const / ( k^2 * ln^GAMMA(k+1) ) for k >= 1.
                             Requires the -mu option.
  -h, --heightfile=HEIGHTFILE   Output the height profile to HEIGHTFILE.
  -i, --inputfile=INPUTFILE  Read a _connected_ graph from file INFILE (graphml
                             format) instead of generating it at random.
  -l, --loopfile=LOOPFILE    Output the looptree associated to the simulated
                             random tree to LOOPFILE.
  -m, --mu=MU                Simulate with an offspring distribution that has
                             average value MU.
  -N, --num=NUM              Simulate NUM many samples. Requires the use of the
                             % symbol in all specified output filenames. For
                             example, --num=100 --outfile=tree%.graphml will
                             create the files tree001.graphml, tree002.graphml,
                             ..., tree100.graphml.
  -o, --outfile=OUTFILE      Output simulated random tree in the graphml format
                             to OUTFILE.
  -p, --profile=PROFILE      Output the degree profile to the file PROFILE.
  -r, --randgen=RANDGEN      Use the pseudo random generator RANDGEN. Available
                             options are taus2, gfsr4, mt19937, ranlux,
                             ranlxs0, ranlxs1, ranlxs2, ranlxd1, ranlxd2, mrg,
                             cmrg, ranlux389. The default is taus2.
  -s, --size=SIZE            Simulate a Galton--Watson tree conditioned on
                             having SIZE vertices.
  -S, --seed=SEED            Specify the seed of the random generator in the
                             first thread. Thread number k will receive SEED +
                             k - 1 as seed. The default is to set SEED to the
                             systems timestamp (in seconds).
  -t, --threads=THREADS      Distribute the workload on THREADS many threads.
                             The default value is the number of CPU cores.
  -v, --vertex=VERTEX        Specify a root vertex. Used in conjunction with
                             the --inputfile parameter. 
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version




*
* 3. Examples
*

3.1 Simulating Galton-Watson trees:


3.1.1 Choice of offspring distributions

The following command simulates a Galton--Watson tree conditioned on having 10000 vertices with an offspring distribution that follows a power law 
			P(k) ~ const / k^2.5 
with expected value 1.0:

grant --size 100000 --mu 1.0 --beta 2.5 --outfile gwtree__100k.graphml


The next example simulates a critical offspring distribution 
			P(k) ~ const / ( k^2 * ln^5.0(k+1) ) 
that lies in the domain of attraction of a Cauchy law:

grant --size 100000 --mu 1.0 --gamma 5.0 --outfile gwtree_crit_condensation_10k.graphml


3.1.2 Generating looptrees:

Use the --loopfile option to generate the looptree. The vertex order and ids in the graphml files for the tree and looptree are kept consistent.

grant --size 10000 --mu 1.0 --beta 2.5 --outfile gwtree__100k.graphml

Note that, using the present sampling method, generating subcritical, heavy tailed Galton-Watson trees conditioned to be large typically takes longer than critical Galton-Watson trees.


3.1.3 Sampling multiple trees and generating statistics

The following command generates the random trees 
			tree1.graphml, tree2.graphml, ..., tree5.graphml 
together with lists
			cen1.dat, cen2.dat, ..., cen5.dat
			deg1.dat, deg1.dat, ..., deg5.dat
			hei1.dat, hei2.dat, ..., hei5.dat
that contain the closeness centrality, outdegree, and height of each vertex, listed in depth-first-search order. The ids of the nodes in the graphml files correspond to their location in depth-first-search order.

grant -N 5 --size 10000 --mu 1.0 --beta 2.5 --outfile tree%.graphml --centfile cen%.dat --degfile deg%.dat --heightfile hei%.dat

A word of caution: Use this option to generate multiple trees with a single command. Do not call GRANT multiple times within 1 second without specifying the seed for the random number generators. The programs default behaviour is to use the systems timestamp as seed, and this state only changes once per second. See section 3.3 below for further info.



3.2 Reading files

GRANT also supports reading graphml files as input. For example, the command 

grant --input tree.graphml --loopfile looptree.graphml

transforms the tree T from a file tree.graphml

        4 - 6
      / 
    1 - 5 
  / 
0 - 2 
  \
    3 - 7 
      \
        8

to its looptree L(T)

        4 - 6
      / |
    1 - 5 
  / |
0   2 
  \ |
    3 - 7 
      \ |
        8


The looptree L(T) depends on the location of the root in T. If no root is specified, GRANT uses the first vertex it encounters in the graphml file. 

A root vertex may also specified explicitly by passing the vertex id to the --vertex option:

grant --vertex 0 --input tree.graphml --loopfile looptree.graphml



3.3 The random number generators


3.3.1 Specifying a generator

The --randgen option allows us to select a random number generator. For example,

grant -r ranlux --size 100000 --mu 1.0 --beta 2.5 --outfile tree.graphml

uses the ranlux generator. The default is taus2. 


3.3.1 Specifying the generators seed

The seed may be set using the -S option:

grant -S 0 -r ranlux --size 100000 --mu 1.0 --beta 2.5 --outfile tree.graphml

TWO words of caution:

First: Specifiying the seed for the random number generators does not guarantee that you will always get the same result, unless you set the number of threads to 1 via --threads=1. We cannot predict which thread is going to be fastest in finding an admissible tree, and the output may vary accordingly. This behaviour is called a racing condition.

Second: The default behaviour is to use the systems timestamp (in seconds) as seed. This value only changes once per second, hence calling GRANT multiple times within 1 second is likely to produce unwanted results.

