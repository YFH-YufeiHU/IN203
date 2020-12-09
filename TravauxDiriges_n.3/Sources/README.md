

# TP2 de HU Yufei

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`





## lscpu

```
Thread(s) per core:  1
Core(s) per socket:  2
Socket(s):           4
NUMA node(s):        1
Vendor ID:           GenuineIntel
CPU family:          6
Model:               158
Model name:          Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
Stepping:            10
CPU MHz:             2208.005
BogoMIPS:            4416.01
Hypervisor vendor:   VMware
Virtualization type: full
L1d cache:           32K
L1i cache:           32K
L2 cache:            256K
L3 cache:            9216K
NUMA node0 CPU(s):   0-7
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon nopl xtopology tsc_reliable nonstop_tsc cpuid pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch cpuid_fault invpcid_single pti ssbd ibrs ibpb stibp fsgsbase tsc_adjust bmi1 avx2 smep bmi2 invpcid rdseed adx smap clflushopt xsaveopt xsavec xsaves arat md_clear flush_l1d arch_capabilities
```

*Des infos utiles s'y trouvent : nb core, taille de cache*



## Produit scalaire 

*Expliquer les paramètres, les fichiers, l'optimisation de compil, NbSamples, ...*

OMP_NUM    | samples=1024 |  speed up
-----------|--------------|----------
séquentiel | 0.122061     | 1  
1          | 0.058605     | 2.08
2          | 0.058474     | 2.09
3          | 0.059237     | 2.06
4          | 0.057697     | 2.11
8          | 0.058978     | 2.07
Lorsque le nombre de threads augmente, le temps de calcul diminue d'abord, puis augmente. En effet, lorsque le nombre de threads augmente, des commutations fréquentes entre les threads ralentissent le temps de calcul.


*Discuter sur ce qu'on observe, la logique qui s'y cache.*


1.3&1.4
qd threads=1,Le temps produit scalaire=0.691s. speedup=1
qd threads=2,Le temps produit scalaire=0.462s. speedup=1.496
qd threads=4,Le temps produit scalaire=0.372s. speedup=1.858
qd threads=8,Le temps produit scalaire=0.746s. speedup=0.926
qd threads=16,Le temps produit scalaire=1.013s.speedup=0.682
qd threads=32,Le temps produit scalaire=1.822s.speedup=0.379
qd threads=64,Le temps produit scalaire=3.321s.speedup=0.208

Le calcul de la version Openmp est plus rapide.

1.5
Parce que le nombre de cœurs dans le système est fixe. L'activation d'un trop grand nombre de threads entraînera de fréquents changements de thread. L'ouverture et la fermeture des threads est la principale raison de la mouvais performance.


## Produit matrice-matrice



### Permutation des boucles

*Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :*

`make TestProduct.exe && ./TestProduct.exe 1024`
`make all && ./TestProduct.exe 1023&& ./TestProduct.exe 1024&& ./TestProduct.exe 1025`

2.1
qd la dimension est égal à 1023, Temps CPU produit matrice-matrice naif : 1.34964 secondes et MFlops -> 1586.49

qd la dimension est égal à 1024, Temps CPU produit matrice-matrice naif : 4.24723 secondes et MFlops -> 643.706

qd la dimension est égal à 1025, Temps CPU produit matrice-matrice naif : 1.38742 secondes et MFlops -> 1552.37

  ordre           | time    | MFlops  | MFlops(n=2048) 
------------------|---------|---------|----------------
i,j,k (origine)   | 2.73764 | 782.476 | 265.217          
j,i,k             | 3.25932 | 658.875 | 242.909   
i,k,j             | 14.5277 | 147.82  | 115.344   
k,i,j             | 15.1051 | 142.17  | 115.815   
j,k,i             | 0.677193| 3171.16 | 2977.28   
k,j,i             | 0.723124| 2969.73 | 2574.65   


*Discussion des résultats*
Cela est probablement dû à l'emplacement de la mémoire. Lors de la réorganisation des boucles, la mémoire requise par la boucle la plus interne sera plus proche et peut être mise en cache, de sorte que le temps d'accès global est le plus bas et l'efficacité est la plus élevée. Dans la version inefficace, vous devez accéder à la mémoire à partir de l'ensemble de données.


### OMP sur la meilleure boucle 

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

Pour la situation que l'ordre est j,k,i, je utile la command (#progma omp parallel for schedule(dynamic) ) pour accélération.
  OMP_NUM         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
1                 | 3008.69 | 2726.46        |3175.32         |2883.67
2                 | 5711.63 | 5348.32        |5313.84         |5489.12
3                 | 7148.66 | 5985.93        |6448.73         |6695.89
4                 | 8961.58 | 8848.46        |8111.35         |6851.54
5                 | 10179.3 | 9515.12        |7931.67         |9714.7
6                 | 10472.5 | 10401.7        |9170.28         |9389.12
7                 | 12196   | 10019.5        |10382.3         |8967.33
8                 | 11919   | 10511.3        |8756.86         |9654.79


Analyse: l'augmentation du nombre de threads accélérera le calcul, mais augmentera en même temps la surcharge de calcul

### Produit par blocs

`make TestProduct.exe && ./TestProduct.exe 1024`
l'ordre est j,k,i
  szBlock         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
origine (=max)    |  |
32                | 3035.64 |3010.09         |3431.41         |2763.48
64                | 3052.01 |2919.53         |3395.19         |2811.64
128               | 2934.67 |2925.05         |3355.12         |2803.73
256               | 3057.2  |2883.32         |3523.4          |2764.83
512               | 3035.37 |2952.98         |3501.66         |2710.42
1024              | 2966.42 |2914.24         |3441.24         |2750.9

Conclusion: le calcul par blocs n'augmentera pas la charge de calcul globale.En fait, augmenter la taille du bloc peut réduire la quantité globale de calcul.


### Bloc + OMP

l'ordre est j,k,i

  szBlock      | OMP_NUM | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
---------------|---------|---------|------------------------------------------------
A.nbCols(512?) |  1      |2978.62  | 2757.39        |3003.94         |2798.01
512            |  8      |13828.5  | 12438          |11313.8         |8826.45



En général, nous pouvons utiliser plus de threads et optimiser la séquence de boucles pour accélérer les opérations de multiplication matricielle. En même temps, le bloc peut être encore optimisé sans augmenter la charge de calcul.



# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
