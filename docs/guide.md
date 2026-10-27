# Guide du projet

Ce guide présente la bibliothèque C, son exécutable de démonstration, les
règles de compilation et quelques bases du langage.

## Utiliser la bibliothèque

Construire la bibliothèque statique :

```sh
make library
```

Le résultat est `build/libreadline_stats.a` ; l'API publique est déclarée dans
`include/readline_stats.h`. Exemple d'utilisation :

```c
#include <stdio.h>
#include <readline_stats.h>

FILE *file = fopen("fichier.txt", "rb");
if (file == NULL) {
    perror("fichier.txt");
    return 1;
}

struct readline_counts counts;
enum readline_status status =
    readline_count_stream(file, READLINE_COUNT_ALL, &counts);
fclose(file);
if (status != READLINE_STATUS_OK) {
    return 1;
}
```

Compile et lie un client en incluant le répertoire d'en-têtes et la bibliothèque
statique :

```sh
cc -std=c17 -Iinclude client.c build/libreadline_stats.a -o client
```

Installe l'archive et l'en-tête public sous `/usr/local` avec :

```sh
sudo make install
```

Le préfixe peut être ajusté, par exemple avec `make install PREFIX=/usr`.
L'archive se nomme `libreadline_stats.a` pour éviter toute confusion avec la
bibliothèque GNU Readline.

`readline_count_stream` lit un flux fourni par l'appelant, sans le fermer.
Passez une combinaison des options `READLINE_COUNT_LINES`,
`READLINE_COUNT_WORDS`, `READLINE_COUNT_BYTES` et
`READLINE_COUNT_CHARACTERS`, ou `READLINE_COUNT_ALL`. En cas de succès, les
compteurs demandés sont renseignés et les autres restent à zéro. En cas
d'erreur, les résultats ne sont pas modifiés ; le statut retourné identifie
une option invalide, une erreur de lecture ou un UTF-8 invalide/incomplet.

Un client complet est disponible dans `examples/count_file.c` :

```sh
make example
./build/count_file chemin/vers/fichier.txt
```

## Utiliser le CLI de démonstration

Afficher l'aide :

```sh
./build/readline --help
```

Chaque opération est demandée avec une option :

```sh
./build/readline -l chemin/vers/fichier.txt
./build/readline -w chemin/vers/fichier.txt
./build/readline -c chemin/vers/fichier.txt
./build/readline -m chemin/vers/fichier.txt
```

Les options courtes `-l`, `-w`, `-c` et `-m` comptent respectivement les
lignes, les mots, les octets et les caractères UTF-8. Les options longues
équivalentes sont `--lines`, `--words`, `--bytes` et `--characters`.

On peut demander plusieurs résultats à la fois, soit en regroupant les options
courtes, soit en les séparant :

```sh
./build/readline -lwc chemin/vers/fichier.txt
./build/readline --lines --words --bytes chemin/vers/fichier.txt
```

Sans option, l'exécutable affiche l'aide. `-h` et `--help` affichent également
l'aide. Pour un chemin qui commence par `-`, placez `--` avant le chemin.

La cible `run` transmet les arguments via `ARGS` :

```sh
make run ARGS="-lwc chemin/vers/fichier.txt"
```

Les lignes comptent les sauts `\n` et le contenu final s'il ne s'y termine
pas. Les mots sont séparés par les espaces reconnus par `isspace`. Les octets
comptent les unités de stockage du fichier ; les caractères comptent les
points de code UTF-8. Par exemple, `é` compte comme un caractère mais deux
octets. Un UTF-8 mal formé provoque une erreur si le comptage des caractères
est demandé. Un fichier vide produit zéro pour chaque compteur.

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
- `LIBRARY` désigne l'archive statique de la bibliothèque.
- `HEADERS` désigne son en-tête public.
- `TARGET` et `DEBUG_TARGET` donnent le chemin des exécutables CLI.

La syntaxe `$(NOM)` est remplacée par la valeur de la variable `NOM`.

### Cibles et dépendances

Une règle suit la forme `cible: dépendances`. `make` exécute les commandes
indentées par une tabulation lorsque la cible est absente ou plus ancienne que
ses dépendances.

- `all` construit la bibliothèque et l'exécutable de démonstration.
- `library` construit uniquement l'archive `build/libreadline_stats.a`.
- `install` installe l'archive et son en-tête sous `$(PREFIX)`.
- `example` compile le client démonstratif contre l'archive publique.
- `run` construit puis exécute le programme avec les arguments définis par
  `ARGS`.
- `test` exécute les trois catégories de tests.
- `test-unit`, `test-functional` et `test-integration` exécutent séparément les
  tests de l'API, du CLI et du client externe.
- `$(UNIT_TEST_TARGET)` compile Unity et lie les tests unitaires à la
  bibliothèque statique sous `vendor/unity/`. Si le sous-module n'est pas
  initialisé, `make` indique la commande nécessaire.
- `debug` construit une version destinée au débogage.
- `clean` supprime les fichiers générés de `build/`.

`.PHONY` indique que ces noms représentent des commandes plutôt que des
fichiers à générer.

Le CLI utilise l'archive statique. La construction correspond à ces étapes :

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -O2 -Iinclude -c src/count.c -o build/count.o
ar rcs build/libreadline_stats.a build/count.o
cc -std=c17 -Wall -Wextra -Wpedantic -O2 -Iinclude -c src/main.c -o build/main.o
cc build/main.o build/libreadline_stats.a -o build/readline
```

`ar` rassemble le fichier objet du module de comptage dans une archive
statique. Les consommateurs incluent l'en-tête public puis lient cette archive
à leur programme.

### Compilation et édition des liens

La compilation transforme un fichier C en fichier objet. L'édition des liens
combine les fichiers objets et bibliothèques nécessaires pour produire un
exécutable. La commande directe ci-dessus effectue les deux étapes ensemble.
Pour les distinguer :

`-c` demande uniquement de produire un fichier objet. Une erreur de syntaxe se
produit pendant la compilation ; une erreur `undefined reference` signale
généralement une définition manquante pendant l'édition des liens.

Pour suivre une erreur, cherchez le premier diagnostic qui indique un fichier
et un numéro de ligne. L'erreur peut se trouver juste avant cette ligne. Les
derniers messages de `make` résument souvent seulement l'échec de la commande.

Pour compiler séparément un client de bibliothèque :

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -Iinclude -c examples/count_file.c -o build/count_file.o
cc build/count_file.o build/libreadline_stats.a -o build/count_file
```

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
gdb --args ./build/readline-debug -l chemin/vers/fichier.txt
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

## Comprendre les tests

`make test` exécute trois catégories. Chacune peut également être lancée
indépendamment :

```sh
make test-unit
make test-functional
make test-integration
```

- `tests/unit/test_count.c` utilise Unity pour vérifier directement l'API :
  compteurs, limites UTF-8, arguments invalides et erreurs de lecture.
- `tests/functional/test_cli.sh` vérifie le comportement observable du CLI :
  options, aide, sorties et erreurs.
- `tests/functional/test_cli_faults.sh` vérifie les erreurs de statut et de
  fermeture grâce à des wrappers réservés aux exécutables de test.
- `tests/integration/test_library_example.sh` lance un client séparé qui a été
  compilé et lié contre l'archive statique.

Les scripts fonctionnels et d'intégration créent puis nettoient leurs fichiers
temporaires.

Unity est le framework C open source de ThrowTheSwitch, version 2.6.1. Le
sous-module `vendor/unity/` pointe vers un commit précis de son dépôt ; la
licence MIT est disponible dans ce sous-module. Pour le mettre à jour,
déplacez le pointeur du sous-module vers le commit souhaité et validez ce
changement dans le dépôt principal.

## Mesurer la couverture

La cible `coverage` recompile le programme, les tests unitaires et le client
d'intégration avec `--coverage`, puis exécute les trois catégories :

```sh
make clean coverage
```

`gcov` utilise les compteurs collectés pendant ces exécutions pour produire
les rapports de lignes et de branches pour les fichiers de `src/`. Les fichiers
`.gcov` sont écrits dans `build/coverage/`. Dans GitHub Actions, ces rapports
sont publiés comme artefact du workflow. La couverture des lignes et celle des
branches sont distinctes : les rapports servent à repérer les chemins encore
non exercés et ne remplacent pas la vérification des comportements. La suite
actuelle atteint 100 % des lignes et des branches dans `src/main.c` et
`src/count.c` avec GCC/gcov ; la cible échoue si ce seuil n'est plus atteint.

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
