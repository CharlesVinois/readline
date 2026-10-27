# readline

Petit programme C17 pour compter les lignes, les mots, les octets et les
caractères UTF-8 d'un fichier.

## Prérequis

- Un compilateur C (par exemple GCC ou Clang)
- GNU Make

Unity est fourni comme sous-module Git. Après un clonage classique, initialise
le sous-module avec :

```sh
git submodule update --init --recursive
```

Ou clone le projet avec ses sous-modules :

```sh
git clone --recurse-submodules <URL-du-depot>
```

## Compiler et analyser un fichier

```sh
make
./build/readline -l chemin/vers/fichier.txt
```

Les options peuvent être combinées :

```sh
./build/readline -lwc chemin/vers/fichier.txt
```

- `-l` ou `--lines` : compter les lignes ;
- `-w` ou `--words` : compter les mots ;
- `-c` ou `--bytes` : compter les octets ;
- `-m` ou `--characters` : compter les caractères UTF-8 ;
- `-h` ou `--help` : afficher l'aide.

Sans option, le programme affiche l'aide. Les caractères sont validés comme
UTF-8 ; une séquence invalide provoque une erreur. La dernière ligne est
comptée même sans saut de ligne final.

La cible `run` accepte les mêmes arguments :

```sh
make run ARGS="-lwc chemin/vers/fichier.txt"
```

## Tests

```sh
make test
```

Les tests couvrent les différentes opérations, leur combinaison, le comptage
UTF-8 et les erreurs d'argument, de fichier ou d'encodage.
Les tests unitaires utilisent Unity, référencé dans `vendor/unity/` par un
commit précis.

## Débogage

```sh
make debug
gdb --args ./build/readline-debug -l chemin/vers/fichier.txt
```

Pour les explications détaillées sur les concepts C, la compilation, les tests
et le débogage, consulte [le guide du projet](docs/guide.md).

Pour supprimer les fichiers générés :

```sh
make clean
```
