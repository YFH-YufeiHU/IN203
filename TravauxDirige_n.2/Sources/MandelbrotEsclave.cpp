# include <iostream>
# include <cstdlib>
# include <string>
# include <chrono>
# include <cmath>
# include <vector>
# include <fstream>
# include <mpi.h>


/** Une structure complexe est définie pour la bonne raison que la classe
 * complex proposée par g++ est très lente ! Le calcul est bien plus rapide
 * avec la petite structure donnée ci--dessous
 **/
struct Complex
{
    Complex() : real(0.), imag(0.)
    {}
    Complex(double r, double i) : real(r), imag(i)
    {}
    Complex operator + ( const Complex& z )
    {
        return Complex(real + z.real, imag + z.imag );
    }
    Complex operator * ( const Complex& z )
    {
        return Complex(real*z.real-imag*z.imag, real*z.imag+imag*z.real);
    }
    double sqNorm() { return real*real + imag*imag; }
    double real,imag;
};

std::ostream& operator << ( std::ostream& out, const Complex& c )
{
  out << "(" << c.real << "," << c.imag << ")" << std::endl;
  return out;
}

/** Pour un c complexe donné, calcul le nombre d'itérations de mandelbrot
 * nécessaires pour détecter une éventuelle divergence. Si la suite
 * converge, la fonction retourne la valeur maxIter
 **/
int iterMandelbrot( int maxIter, const Complex& c)
{
    Complex z{0.,0.};
    // On vérifie dans un premier temps si le complexe
    // n'appartient pas à une zone de convergence connue :
    // Appartenance aux disques  C0{(0,0),1/4} et C1{(-1,0),1/4}
    if ( c.real*c.real+c.imag*c.imag < 0.0625 )
        return maxIter;
    if ( (c.real+1)*(c.real+1)+c.imag*c.imag < 0.0625 )
        return maxIter;
    // Appartenance à la cardioïde {(1/4,0),1/2(1-cos(theta))}    
    if ((c.real > -0.75) && (c.real < 0.5) ) {
        Complex ct{c.real-0.25,c.imag};
        double ctnrm2 = sqrt(ct.sqNorm());
        if (ctnrm2 < 0.5*(1-ct.real/ctnrm2)) return maxIter;
    }
    int niter = 0;
    while ((z.sqNorm() < 4.) && (niter < maxIter))
    {
        z = z*z + c;
        ++niter;
    }
    return niter;
}

/**
 * On parcourt chaque pixel de l'espace image et on fait correspondre par
 * translation et homothétie une valeur complexe c qui servira pour
 * itérer sur la suite de Mandelbrot. Le nombre d'itérations renvoyé
 * servira pour construire l'image finale.
 
 Sortie : un vecteur de taille W*H avec pour chaque case un nombre d'étape de convergence de 0 à maxIter
 MODIFICATION DE LA FONCTION :
 j'ai supprimé le paramètre W étant donné que maintenant, cette fonction ne prendra plus que des lignes de taille W en argument.
 **/
void 
computeMandelbrotSetRow( int W, int H, int maxIter, int num_ligne, int* pixels)
{
    // Calcul le facteur d'échelle pour rester dans le disque de rayon 2
    // centré en (0,0)
    double scaleX = 3./(W-1);
    double scaleY = 2.25/(H-1.);
    //
    // On parcourt les pixels de l'espace image :
    for ( int j = 0; j < W; ++j ) {
       Complex c{-2.+j*scaleX,-1.125+ num_ligne*scaleY};
       pixels[j] = iterMandelbrot( maxIter, c );
    }
}

std::vector<int>
computeMandelbrotSet( int W, int H, int rank, int nbp, int locH, int maxIter )
{
    int startRow,endRow;

    startRow = (nbp-rank-1)*locH;
    endRow = (nbp-rank)*locH;

    std::vector<int> pixels(W*locH);
    for ( int i = startRow; i < endRow; ++i ) {
        computeMandelbrotSetRow(W, H, maxIter, i, pixels.data() + W*(locH-(i-startRow)-1) );
    }
    return pixels;
}

/** Construit et sauvegarde l'image finale **/
void savePicture( const std::string& filename, int W, int H, const std::vector<int>& nbIters, int maxIter )
{
    double scaleCol = 1./maxIter;//16777216
    std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
    ofs << "P6\n"
        << W << " " << H << "\n255\n";
    for ( int i = 0; i < W * H; ++i ) {
        double iter = scaleCol*nbIters[i];
        unsigned char r = (unsigned char)(256 - (unsigned (iter*256.) & 0xFF));
        unsigned char b = (unsigned char)(256 - (unsigned (iter*65536) & 0xFF));
        unsigned char g = (unsigned char)(256 - (unsigned( iter*16777216) & 0xFF));
        ofs << r << g << b;
    }
    ofs.close();
}

int main(int argc, char *argv[] ) 
 { 
    MPI_Init(&argc,&argv);
    const int W = 800;
    const int H = 600;
    const int nb_tasks = 200;
    // Normalement, pour un bon rendu, il faudrait le nombre d'itérations
    // ci--dessous :
    double start,end;
    start = MPI_Wtime();
    //const int maxIter = 16777216;
    const int maxIter = 8*65536;
    int nbp,rank;
    int tag=0;
    MPI_Status status;
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_rank(globComm,&rank);
    MPI_Comm_size(globComm,&nbp);
    int locH = H/nb_tasks;

    //matrice-esclave
    if(rank==0){
        std::vector<int>recvbuff(W*locH+1);
        std::vector<int>globPixels(W*H);
        int count_task=0;
        for(int i=1;i<nbp;++i){
            MPI_Send(&count_task,1,MPI_INT,i,tag,globComm);
            count_task+=1;
        }
        while(count_task<nb_tasks){
            // MPI_Recv(&sub_task,1,MPI_INT,MPI_ANY_SOURCE,tag,globComm,&status);
            MPI_Recv(recvbuff.data(),recvbuff.size(),MPI_INT,MPI_ANY_SOURCE,tag,globComm,&status);
            MPI_Send(&count_task,1,MPI_INT,status.MPI_SOURCE,tag,globComm);
            std::copy(recvbuff.begin()+1,recvbuff.end(),globPixels.begin()+recvbuff.at(0)*(recvbuff.size()-1));
            count_task+=1;
        }
        //receive the left data!
        for(int i=1;i<nbp;++i){
            MPI_Recv(recvbuff.data(),W*locH+1,MPI_INT,MPI_ANY_SOURCE,tag,globComm,&status);
            std::copy(recvbuff.begin()+1,recvbuff.end(),globPixels.begin()+recvbuff.at(0)*(recvbuff.size()-1));
        }
        //send the end signal
        count_task=-1;
        for(int i=1;i<nbp;++i){
            MPI_Send(&count_task,1,MPI_INT,i,tag,globComm);
        }
        savePicture("mandelbortEsclave.tga",W,H,globPixels,maxIter);
        end = MPI_Wtime();
        std::cout<<"temps total :"<<end-start<<"s"<<std::endl;
    }
    if(rank>0)
    {
        int num_task=0;  //recebuf of count_task
        std::vector<int> locPixels(W*locH+1);
        while (num_task!=-1)
        {
            MPI_Recv(&num_task,1,MPI_INT,0,tag,globComm,&status);
            locPixels[0]=num_task;
            if(num_task>=0){
                auto iters = computeMandelbrotSet( W, H, num_task, nb_tasks, locH, maxIter);
                // MPI_Send(&num_task,1,MPI_INT,0,tag,globComm);
                std::copy(iters.begin(),iters.end(),locPixels.begin()+1);
                MPI_Send(locPixels.data(),locPixels.size(),MPI_INT,0,tag,globComm);
            }
        }     
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
 }
    
