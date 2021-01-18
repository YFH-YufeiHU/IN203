#include <cstdlib>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>        
#include <SDL2/SDL_image.h>
#include <fstream>
#include <ctime>
#include <iomanip>      // std::setw
#include <chrono>
#include <vector>
#include <mpi.h>


#include "parametres.hpp"
#include "galaxie.hpp"
 
int main(int argc, char ** argv)
{
    //inital MPI 
    MPI_Init(&argc,&argv);
    int nbp,rank;
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_rank(globComm,&rank);
    MPI_Comm_size(globComm,&nbp);
    MPI_Status status;
    MPI_Request send_req[nbp-1],rec_req[nbp-1];
    int tag=0;

    char commentaire[4096];
    int width, height;
    SDL_Event event;
    SDL_Window   * window;

    parametres param;


    std::ifstream fich("parametre.txt");
    fich >> width;
    fich.getline(commentaire, 4096);
    fich >> height;
    fich.getline(commentaire, 4096);
    fich >> param.apparition_civ;
    fich.getline(commentaire, 4096);
    fich >> param.disparition;
    fich.getline(commentaire, 4096);
    fich >> param.expansion;
    fich.getline(commentaire, 4096);
    fich >> param.inhabitable;
    fich.getline(commentaire, 4096);
    fich.close();

    int loc_height = height/(nbp-1);

    if(rank==0){
      std::cout<<"loc_height ==>"<<loc_height<<std::endl;
      std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
      std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
      std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
      std::cout << "\t Chance expansion : " << param.expansion << std::endl;
      std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
      std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
      std::srand(std::time(nullptr));

      SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

      window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, SDL_WINDOW_SHOWN);

      galaxie g(width, height, param.apparition_civ);
      galaxie g_next(width, height);
      galaxie_renderer gr(window);

      int deltaT = (20*52840)/width;
      std::cout << "Pas de temps : " << deltaT << " années" << std::endl;

      std::cout << std::endl;

      gr.render(g);
      unsigned long long temps = 0;

      std::chrono::time_point<std::chrono::system_clock> start, end1, end2;
      std::vector<std::vector<char>> send_buff(nbp-1,std::vector<char>(width*(loc_height+2),0));
      std::vector<std::vector<char>> rec_buff(nbp-1,std::vector<char>(width*loc_height,0));
        
      while(1){
          start = std::chrono::system_clock::now();
     
          send_buff[0].resize(width*(loc_height+1));
#         pragma omp parallel for shared(send_buff) 
          for(int i=0;i<loc_height+1;++i)
            for(int j=0;j<width;++j){
              send_buff[0][i*width+j]=*(g.data()+i*width+j);
            }
           
          send_buff[nbp-2].resize(width*(loc_height+1));
#         pragma omp parallel for shared(send_buff) 
          for(int i=loc_height*(nbp-2)-1;i<height;i++)
            for(int j=0;j<width;++j){
              send_buff[nbp-2][(i-(loc_height*(nbp-2)-1))*width+j]=*(g.data()+i*width+j);
            }

          // when the nbp>3
          if(nbp>3){
            for(int k=2;k<nbp-1;++k){
#             pragma omp parallel for shared(send_buff) 
              for(int i=(k-1)*loc_height-1;i<k*loc_height+1;++i){
                for(int j=0;j<width;++j){
                  send_buff[k-1][(i-((k-1)*loc_height-1))*width+j]=*(g.data()+i*width+j);
                }
              }
            }
          }

          //send the data
          for(int k=1;k<nbp;++k){
            if((k==1)||(k==nbp-1)){
              MPI_Isend(send_buff[k-1].data(),width*(loc_height+1),MPI_CHAR,k,tag,globComm,&send_req[k-1]);
            }
            else{
              MPI_Isend(send_buff[k-1].data(),width*(loc_height+2),MPI_CHAR,k,tag,globComm,&send_req[k-1]);
            }
          }
          MPI_Wait(send_req, &status);

          //receive the data 
          for(int k=1;k<nbp;++k){
            MPI_Irecv(rec_buff[k-1].data(),width*loc_height,MPI_CHAR,k,tag,globComm,&rec_req[k-1]);
          }
          MPI_Wait(rec_req, &status);

          //update the g_next
          for(int k=1;k<nbp;++k){
            g_next.setValue(rec_buff[k-1],loc_height*(k-1),loc_height);
          }

          end1 = std::chrono::system_clock::now();
          g_next.swap(g);
          gr.render(g);
          end2 = std::chrono::system_clock::now();
          
          std::chrono::duration<double> elaps1 = end1 - start;
          std::chrono::duration<double> elaps2 = end2 - end1;
          
          temps += deltaT;
          std::cout << "Temps passe : "
                    << std::setw(10) << temps << " années"
                    << std::fixed << std::setprecision(3)
                    << "  " << "|  CPU(ms) : calcul " << elaps1.count()*1000
                    << "  " << "affichage(ms) " << elaps2.count()*1000
                    << "\r" << std::flush;
          //_sleep(1000);
          if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            std::cout << std::endl << "The end" << std::endl;
            break;}
      }
      SDL_DestroyWindow(window);
      SDL_Quit();
      MPI_Finalize();
    }

    else if(rank==1){
      std::vector<char> send_buff(width*loc_height);
      std::vector<char> rec_buff(width*(loc_height+1));
      std::vector<char> map_next(width*(loc_height+1),0);
      while(1){
        MPI_Recv(rec_buff.data(),width*(loc_height+1),MPI_CHAR,0,tag,globComm,&status);
        mise_a_jour(param, width, loc_height+1, rec_buff.data(), map_next.data());
#       pragma omp parallel for shared(send_buff) 
        for(int i=0;i<loc_height;++i)
          for(int j=0;j<width;++j){
            send_buff[i*width+j]=*(map_next.data()+i*width+j);
          }
        MPI_Send(send_buff.data(),width*loc_height,MPI_CHAR,0,tag,globComm);
      }
    } 
        
    else if(rank==nbp-1){
      std::vector<char> send_buff(width*loc_height);
      std::vector<char> rec_buff(width*(loc_height+1));
      std::vector<char> map_next(width*(loc_height+1),0);
      while(1){
        MPI_Recv(rec_buff.data(),width*(loc_height+1),MPI_CHAR,0,tag,globComm,&status);
        mise_a_jour(param, width, loc_height+1, rec_buff.data(), map_next.data());
#       pragma omp parallel for shared(send_buff) 
        for(int i=1;i<loc_height+1;++i)
          for(int j=0;j<width;++j){
            send_buff[(i-1)*width+j]=*(map_next.data()+i*width+j);
          }
        MPI_Send(send_buff.data(),width*loc_height,MPI_CHAR,0,tag,globComm);
      }
    }

    else{
      std::vector<char> send_buff(width*loc_height);
      std::vector<char> rec_buff(width*(loc_height+2));
      std::vector<char> map_next(width*(loc_height+2),0);
      while(1){
        MPI_Recv(rec_buff.data(),width*(loc_height+2),MPI_CHAR,0,tag,globComm,&status);
        mise_a_jour(param, width, loc_height+2, rec_buff.data(), map_next.data());
#       pragma omp parallel for shared(send_buff) 
        for(int i=1;i<loc_height+1;++i)
          for(int j=0;j<width;++j){
            send_buff[(i-1)*width+j]=*(map_next.data()+i*width+j);
          }
        MPI_Send(send_buff.data(),width*loc_height,MPI_CHAR,0,tag,globComm);
      }
    }
    
    return EXIT_SUCCESS;
}
