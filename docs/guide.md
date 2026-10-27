# Guide du projet

Ce guide présente l'utilisation du programme, les règles de compilation et
quelques bases du langage C.

## Utiliser le programme

Compiler et compter les lignes d'un fichier :

```sh
make
./build/readline chemin/vers/fichier.txt
```

Ou utiliser la cible `run` :

```sh
make run FILE=chemin/vers/fichier.txt
```

Le compteur compte chaque saut de ligne et la dernière ligne si elle ne se
termine pas par `\n`. Un fichier vide contient zéro ligne.

Exécuter les tests :

```sh
make test
```

## Comprendre le Makefile

### Variables

- `CC` choisit le compilateur C ; `cc` est souvent GCC ou Clang.
- `CFLAGS` regroupe les options de compilation : `-std=c17` sélectionne le
  standard C17, `-Wall -Wextra -Wpedantic` active les avertissements et `-O2`
  active les optimisations.
- `SOURCE` désigne le fichier C à compiler.
- `TARGET` et `DEBUG_TARGET` donnent le chemin des exécutables générés.

La syntaxe `$(NOM)` est remplacée par la valeur de la variable `NOM`.

### Cibles et dépendances

Une règle suit la forme `cible: dépendances`. `make` exécute les commandes
indentées par une tabulation lorsque la cible est absente ou plus ancienne que
ses dépendances.

- `all` est la cible par défaut et construit l'exécutable normal.
- `run` construit puis exécute le programme avec le fichier défini par `FILE`.
- `test` construit le programme et lance les tests shell.
- `debug` construit une version destinée au débogage.
- `clean` supprime les fichiers générés de `build/`.

`.PHONY` indique que ces noms représentent des commandes plutôt que des
fichiers à générer.

La commande de compilation normale ressemble à :

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -O2 src/main.c -o build/readline
```

`mkdir -p build` crée le dossier de sortie s'il n'existe pas. L'option `-o`
choisit le nom de l'exécutable. Le compilateur vérifie le code C, le traduit,
puis édite les liens pour créer le programme.

### Compilation et édition des liens

La compilation transforme un fichier C en fichier objet. L'édition des liens
combine les fichiers objets et bibliothèques nécessaires pour produire un
exécutable. La commande directe ci-dessus effectue les deux étapes ensemble.
Pour les distinguer :

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -c src/main.c -o build/main.o
cc build/main.o -o build/readline
```

`-c` demande uniquement de produire le fichier objet. Une erreur de syntaxe se
produit pendant la compilation ; une erreur `undefined reference` signale
généralement une définition manquante pendant l'édition des liens.

Pour suivre une erreur, cherchez le premier diagnostic qui indique un fichier
et un numéro de ligne. L'erreur peut se trouver juste avant cette ligne. Les
derniers messages de `make` résument souvent seulement l'échec de la commande.

## Déboguer avec GDB

GDB exécute un programme pas à pas, permet de poser des points d'arrêt et
d'inspecter les variables. Sur Ubuntu ou Debian :

```sh
sudo apt update
sudo apt install gdb
```

Construire la version de débogage et la lancer avec un fichier :

```sh
make debug
gdb --args ./build/readline-debug chemin/vers/fichier.txt
```

Cette version utilise `-g` pour inclure les informations de débogage et `-O0`
pour désactiver les optimisations. Dans GDB :

```text
break main       placer un point d'arrêt au début de main
run              démarrer le programme
next             exécuter la ligne suivante sans entrer dans une fonction
step             exécuter la ligne suivante en entrant dans une fonction
print lines      afficher la valeur d'une variable
continue         reprendre jusqu'au prochain point d'arrêt ou la fin
quit             quitter GDB
```

La commande `list` affiche le code source autour de la ligne courante.

## Rappels rapides sur le C

### Variables et types

Une variable a un type et une valeur :

```c
int age = 20;
double prix = 3.5;
char initiale = 'A';
```

`int` représente un entier, `double` un nombre décimal et `char` un caractère.

### Fonctions

Une fonction regroupe des instructions. Son type indique ce qu'elle renvoie ;
`void` signifie qu'elle ne renvoie pas de valeur :

```c
int doubler(int nombre)
{
    return nombre * 2;
}
```

L'appel `doubler(4)` renvoie `8`.

### Conditions et boucles

`if` et `else` choisissent un bloc suivant une condition :

```c
if (age >= 18) {
    puts("Majeur");
} else {
    puts("Mineur");
}
```

`=` affecte une valeur ; `==` compare deux valeurs. Une boucle `for` ou
`while` répète des instructions :

```c
for (int i = 0; i < 3; i++) {
    printf("%d\n", i);
}
```

### Tableaux et chaînes

Un tableau contient plusieurs valeurs de même type. Ses indices commencent à
zéro :

```c
int notes[3] = {12, 15, 18};
printf("%d\n", notes[0]);
```

C ne vérifie pas automatiquement les limites d'un tableau. Une chaîne C est un
tableau de caractères terminé par `'\0'` :

```c
char nom[] = "Ada";
printf("Bonjour %s\n", nom);
```

### Pointeurs et mémoire

Un pointeur contient une adresse. `&` prend l'adresse d'une variable et `*`
permet d'accéder à la valeur pointée :

```c
int nombre = 5;
int *adresse = &nombre;
*adresse = 8;
```

Les variables locales existent pendant l'exécution de leur bloc. La mémoire
dynamique, souvent appelée tas, reste allouée jusqu'à ce qu'elle soit libérée.
`malloc` réserve cette mémoire et `free` la libère :

```c
#include <stdlib.h>

int *valeur = malloc(sizeof *valeur);
if (valeur == NULL) {
    return 1;
}
*valeur = 42;
free(valeur);
```

Vérifiez le résultat de `malloc`, libérez chaque bloc une seule fois et ne
réutilisez pas un pointeur après sa libération.

### Structures

Une structure rassemble des membres de types potentiellement différents :

```c
struct Point {
    int x;
    int y;
};

struct Point origine = {0, 0};
printf("%d\n", origine.x);
```

L'opérateur `.` accède à un membre ; `->` s'utilise avec un pointeur vers une
structure.

### Fichiers

`fopen` ouvre un fichier et renvoie `NULL` en cas d'erreur. Fermez un fichier
ouvert avec `fclose` :

```c
FILE *file = fopen("notes.txt", "r");
if (file == NULL) {
    perror("notes.txt");
    return 1;
}

int character = fgetc(file);
fclose(file);
```

Le mode `"r"` ouvre en lecture. `fgets`, `fprintf` et `fputc` sont d'autres
fonctions standard de lecture et d'écriture.

## Compilation séparée

Un projet peut répartir son code dans plusieurs fichiers `.c`. Chaque fichier
est compilé en objet `.o`, puis l'éditeur de liens les assemble. Les fichiers
`.h` exposent les déclarations partagées :

```c
/* calcul.h */
int doubler(int nombre);
```

Un fichier `.c` définit la fonction :

```c
/* calcul.c */
#include "calcul.h"

int doubler(int nombre)
{
    return nombre * 2;
}
```

Le fichier appelant inclut lui aussi le `.h`. Les gardes d'inclusion
`#ifndef`, `#define` et `#endif` empêchent un en-tête d'être traité plusieurs
fois.
