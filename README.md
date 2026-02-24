# Projet développement d'un mini système d'exploitation pour PC x86

## Objectifs

Développer les éléments de base d'un système d'exploitation

Ce que nous verrons :

- Gestion d'entrées/sorties de base : le clavier et l'écran
- Gestion des interruptions
- Gestion des processus
- Gestion de la mémoire virtuelle pour les processus

Ce que nous ne verrons pas :

- Gestion des fichiers
- Partage de ressources et communication entre processus
  
## Organisation

- 14 séances encadrées
- Pas de cours
- Programmation en C, un peu d'assembleur
- Evaluation : code commenté
  
## Au menu

### Entrée

- De l'affichage à la console
- S'il vous plaît ? Je peux vous interrompre ?
- Il y a des manières, monsieur ! Utilisez l'appel système !

> - Mise en oeuvre de la console
> - Appel système write

### Le plat

- Tic Tac Tic Tac, respectez le Timer !
- Des processus ? Comment tu définis ça ?
- Alors toi, tu crées des processus et tu les détruis.
- Il faudrait organiser tout ce beau monde, non ?
  - Et hop ! Tout le monde en file !
  - Laissez un peu la place aux autres ! Revenez dans la file ! Respectez le tourniquet !

> - Interruption Timer
> - Ordonnancement et gestions des processus

### Le dessert

- C'est bien fichu ici : tu peux commander depuis la table avec un clavier.

> - Lecture au clavier et appel système read
> - Interpréteur de commandes simple

### Avec ta fourchette !

Il nous faut des couverts !

- Compilation : GCC 
  
  ```sudo apt-get install build-essentials```

- Exécution : QEMU
   
  ```sudo apt-get install qemu```

- Mise au point : GDB
    
    - ```sudo apt-get install gdb```
    - GDB sera connecté à QEMU et permet d'afficher les problèmes potentiels

> [!NOTE]
> Les commandes données sont pour environnement Debian et dérivés (j'utilise Ubuntu). A adapter en fonction de votre distribution Linux.   
> Sous Mac : Utiliser les outils `gcc` pour x86 disponible via les `macports` (paquet `i386-elf-gcc`).

## Les amuses-bouches

### Fichiers fournis 

Le répertoire fourni contient :

- `/boot` 
  - répertoire d'entrée du système ;
  - `crt0.S` initialise le matériel et lance le programme principal du système (`kernel_start`)
- `/kernel`
  - répertoire sources du noyau
  - c'est ici que tout (ou presque) va se passer
- `/lib`
  - quelques outils utiles (par ex.: `printf`)
- `/include`
  - pour les `.h` c'est ici

### Prise en main de l'environnement

- La compilation s'effectue via la commande `make`
  - si tout va bien, résultat : `kernel.bin`
- Exécution : `make run`
  - Une fenêtre QEMU doit apparaître
  - Le système est exécuté
- Mise au point 
  - Lancer : `make dbg`
  - Mettre un point d'arrêt au début du système : `b kernel_start`
  - Lancer l'exécution : `cont` ou `r`
  - Afficher un variable : `display` nom de la variable
  - `n`: Next, `s` : Step


TODO: 
 - Appels systemes a faire pour les procs
 - Passer l'horloge en process 