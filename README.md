# readline

Projet C utilisant le standard C17 et GNU Make.

## Prérequis

- Un compilateur C (par exemple GCC ou Clang)
- GNU Make

## Compiler

```sh
make
```

Le programme compilé sera placé dans `build/readline`.

## Compter les lignes d'un fichier

```sh
./build/readline chemin/vers/fichier.txt
```

La dernière ligne est comptée même si le fichier ne se termine pas par un
saut de ligne. La cible `run` accepte aussi le chemin du fichier :

```sh
make run FILE=chemin/vers/fichier.txt
```