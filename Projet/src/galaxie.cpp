#include <stdlib.h>
#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>
#include <SDL2/SDL_image.h>
#include "galaxie.hpp"
#define THREADS 16
#define CHUNK 100

//_ ______________________________________________________________________________________________ _
galaxie::galaxie(int width, int height)
    :   m_width(width),
        m_height(height),
        m_planetes(width*height, habitable)
{}
//_ ______________________________________________________________________________________________ _
galaxie::galaxie(int width, int height, double chance_habitee)
    :   m_width(width),
        m_height(height),
        m_planetes(width*height)
{
    int i,j;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    
#   pragma omp parallel for private(j) shared(m_planetes) num_threads(THREADS)
    for ( i = 0; i < height; ++i )
        for ( j = 0; j < width; ++j )
        {
            unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count() + 173501*omp_get_thread_num();
            // printf("i= %d:OpenMP Test, id: %d\n", i,omp_get_thread_num());
            std::minstd_rand0 generator (seed1); 

            double val = distribution(generator);
            // double val = std::rand()/(1.*RAND_MAX);
            if (val < chance_habitee)
            {
                m_planetes[i*width+j] = habitee;
            }
            else
                m_planetes[i*width+j] = habitable;
        }
}
//_ ______________________________________________________________________________________________ _
void 
galaxie::rend_planete_habitee(int x, int y)
{
    m_planetes[y*m_width + x] = habitee;
}
//_ ______________________________________________________________________________________________ _
void
galaxie::rend_planete_inhabitable(int x, int y)
{
    m_planetes[y*m_width + x] = inhabitable;
}
//_ ______________________________________________________________________________________________ _
void
galaxie::rend_planete_inhabitee(int x, int y)
{
    m_planetes[y*m_width + x] = habitable;
}
//_ ______________________________________________________________________________________________ _
void
galaxie::swap(galaxie& g)
{
    g.m_planetes.swap(this->m_planetes);
}
//_ ______________________________________________________________________________________________ _
void
galaxie::setValue(std::vector<char> buff, int start_index, int loc_height)
{
#   pragma omp parallel for shared(m_planetes,buff)
    for(int i=start_index;i<start_index+loc_height;++i)
        for(int j=0;j<m_width;++j){
            m_planetes[i*m_width+j] = buff[(i-start_index)*m_width+j];
        }
}
void galaxie::SetValue(std::vector<char> v, int start_index, int height_pas)
{
    #   pragma omp parallel for shared(m_planetes,v)
    for(int i = 0; i < height_pas; ++i){
        for(int j = 0; j < m_width; ++j){
            m_planetes[start_index+i*m_width+j] = v[i*m_width+j];
        }
    }
}
//# ############################################################################################## #
galaxie_renderer::galaxie_renderer(SDL_Window* win)
{
    m_renderer = SDL_CreateRenderer(win, -1, 0);
    IMG_Init(IMG_INIT_JPG);
    m_texture = IMG_LoadTexture(m_renderer, "data/galaxie.jpg");
}
//_ ______________________________________________________________________________________________ _
galaxie_renderer::~galaxie_renderer()
{
    SDL_DestroyTexture(m_texture);
    IMG_Quit();
    SDL_DestroyRenderer(m_renderer);
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::rend_planete_habitee(int x, int y)
{
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 64);// Couleur verte
    SDL_RenderDrawPoint(m_renderer, x, y);
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::rend_planete_inhabitable(int x, int y)
{
    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 64);// Couleur rouge
    SDL_RenderDrawPoint(m_renderer, x, y);
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::rend_planete_inhabitee(int x, int y)
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);// Couleur noire
    SDL_RenderDrawPoint(m_renderer, x, y);    
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::render(const galaxie& g)
{
    int i, j;
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    const char* data   = g.data();
    int   width  = g.width();
    int   height = g.height();

#   pragma omp parallel for private(i,j) shared(data) num_threads(THREADS)
    for (i = 0; i < height; ++i )
        for (j = 0; j < width; ++j )
        {
            if (data[i*width+j] == habitee){
                #pragma omp critical
                rend_planete_habitee(j, i);
                // printf("i= %d:OpenMP Test, id: %d\n", i,omp_get_thread_num());
            }
            else if (data[i*width+j] == inhabitable){
                #pragma omp critical
                rend_planete_inhabitable(j, i);
                // printf("i= %d:OpenMP Test, id: %d\n", i,omp_get_thread_num());
            }
        }

    SDL_RenderPresent(m_renderer);
}
