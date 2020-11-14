# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>
# include <stdlib.h>
# include <time.h>
# include <math.h>
# include <bitset>

using namespace std;

int generate_rand(int a, int b)
{
	//return a random number of int type belonging to [a,b)

	srand((unsigned)time(NULL));
	return rand()%(b-a)+a;
	
}
int main( int nargs, char* argv[] )
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	int tag=0;
	MPI_Status status;
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

	// Rajout du programme ici...
	double start = MPI_Wtime();
	int jeton;
	int dimension = log2(nbp);

	bitset<6> transformRank(rank);
	if(transformRank>>(dimension-1)==0){
		jeton = generate_rand(0,100);
		// cout<<transformRank.set(dimension-1)<<endl;
		cout<<"La processus n°"<<rank<<"est source"<<" et jeton="<<jeton<<endl;
		int neighbor = static_cast<int>(transformRank.set(dimension-1).to_ullong());
		MPI_Send(&jeton,1,MPI_INT,neighbor,tag,globComm);
	}
	else{
		MPI_Recv(&jeton,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,globComm,&status);
		cout<<"La processus n°"<<rank<<" et jeton="<<jeton<<endl;
	}
	double end = MPI_Wtime();
	if(rank==0){cout<<"le temps d'execution"<<(end-start)*1000<<"ms"<<endl;}
	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
