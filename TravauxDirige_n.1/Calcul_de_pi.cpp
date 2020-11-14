# include <chrono>
# include <random>
# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>
# include <time.h>
# include <numeric>      // std::accumulate
# include <vector>

using namespace std;

// Attention , ne marche qu'en C++ 11 ou supérieur :
double approximate_pi( unsigned long nbSamples ) 
{
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now();
    myclock::duration d = beginning.time_since_epoch();
    unsigned seed = d.count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution <double> distribution ( -1.0 ,1.0);
    unsigned long nbDarts = 0;
    // Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
    for ( unsigned sample = 0 ; sample < nbSamples ; ++ sample ) {
        double x = distribution(generator);
        double y = distribution(generator);
        // Test if the dart is in the unit disk
        if ( x*x+y*y<=1 ) nbDarts ++;
    }
    // Number of nbDarts throwed in the unit disk
    double ratio = double(nbDarts)/double(nbSamples);
    return 4*ratio;
}

double sumArray(double* arr, int length){
	double sum=0.;
	for (int i=0;i<length;++i){
		sum+=arr[i];
	}
	return (double)sum/length;
}

int main( int nargs, char* argv[] )
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...

	MPI_Init( &nargs, &argv );
	// Pour des raisons de portabilité qui débordent largement du cadre
	// de ce cours, on préfère toujours cloner le communicateur global
	// MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour connaître le nombre de processus
	// qui ont été lancés par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour connaître l'identifiant qui
	// m'a été attribué ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);
	// Création d'un fichier pour ma propre sortie en écriture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output( fileName.str().c_str() );

	// Rajout de code....
	clock_t start,end;
	unsigned long nbSamples = 100000;
	double portion = 1./(nbp-1);
	int tag = 0;
	MPI_Status status;

	if(rank == 0)
	{	
		// le premiere version
		// start = clock();
		// double pi[nbp-1];
		// for (int i=0;i<nbp-1;i++){
		// 	MPI_Recv(&pi[i],1,MPI_DOUBLE,i+1,tag,globComm,&status);
		// 	// cout<<"le pi de processus:"<<pi[i]<<endl;
		// }
		// output<<"La premiere version"<<endl;
		// output<<"nbSamples = "<<nbSamples<<","<<"nbp = "<<nbp<<endl;
		// output<<"The mean pi = "<< sumArray(pi,nbp-1)<<endl;
		// end = clock();
		// output<<"Le temps de l'execution de premiere version = "<<(double)(end-start)/CLOCKS_PER_SEC*1000<<"ms"<<endl;

		// le deuxieme version
		start = clock();
		
		// cout<<"pi"<<approximate_pi(nbSamples)<<endl;
		MPI_Request requests[nbp-1];
		double pi[nbp-1];
		for (int i=0;i<nbp-1;i++){
			MPI_Irecv(&pi[i],1,MPI_DOUBLE,i+1,tag,globComm,&requests[i]);
			// cout<<"le pi de processus:"<<pi[i]<<endl;
		}
		MPI_Waitall(nbp-1,requests,&status);
		output<<"La deuxieme version"<<endl;
		output<<"nbSamples = "<<nbSamples<<","<<"nbp = "<<nbp<<endl;
		output<<"The mean pi = "<< sumArray(pi,nbp-1)<<endl;
		end = clock();
		output<<"Le temps de l'execution de premiere version = "<<(double)(end-start)/CLOCKS_PER_SEC*1000<<"ms"<<endl;

	}
	else{
		// cout<< nbSamples*portion<<endl;
		double pi = approximate_pi((unsigned long)(nbSamples*portion));
		cout<<"Le pi de processus n°"<<rank<<" = "<<pi<<endl;
		MPI_Send(&pi,1,MPI_DOUBLE,0,tag,globComm);
	}
	
	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
