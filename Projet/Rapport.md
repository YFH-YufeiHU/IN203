#1 Parallélisation en mémoire partagée
On parallélisera le code dans un premier temps en mémoire partagée, en mettant en œuvre un parallélisme de la boucle de calcul. On s'assurera que si il y a condition de data race, bien expliquer quelles sont ces data races dans le rapport du projet, et bien s'assurer que le cas échéant, elles ne porteront pas préjudice au déroulement du code et aux résultats.

Temps passe :      56100 années  |  CPU(ms) : calcul 46.521  affichage(ms) 0.645
Temps passe :     318450 années  |  CPU(ms) : calcul 5.291  affichage(ms) 0.445

#2 Parallélisation en mémoire distribuée


export OMP_NUM_THREADS=16
mpirun -np 17 ./colonisation.exe 

-np=3 Temps passe :     132000 années  |  CPU(ms) : calcul 26.946  affichage(ms) 0.48793
-np=5 Temps passe :    1692075 années  |  CPU(ms) : calcul 14.488  affichage(ms) 0.5531

mpi&omp
-np=3 Temps passe :    1066725 années  |  CPU(ms) : calcul 7.362  affichage(ms) 0.52960
-np=5 Temps passe :    2267100 années  |  CPU(ms) : calcul 8.597  affichage(ms) 0.50318