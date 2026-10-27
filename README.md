# readline

[![CI](https://github.com/CharlesVinois/readline/actions/workflows/ci.yml/badge.svg)](https://github.com/CharlesVinois/readline/actions/workflows/ci.yml)

Bibliothèque C17 pour compter les lignes, les mots, les octets et les caractères
UTF-8 d'un flux, avec un exécutable CLI de démonstration.

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

## Construire la bibliothèque

```sh
make library
```

Cela produit la bibliothèque statique `build/libreadline_stats.a` et l'en-tête
public `include/readline_stats.h`. Un client C peut utiliser l'API ainsi :

```c
#include <stdio.h>
#include <readline_stats.h>

FILE *file = fopen("fichier.txt", "rb");
if (file != NULL) {
    struct readline_counts counts;
    enum readline_status status =
        readline_count_stream(file, READLINE_COUNT_ALL, &counts);
    fclose(file);
}
```

Compile et lie le client avec la bibliothèque :

```sh
cc -std=c17 -Iinclude client.c build/libreadline_stats.a -o client
```

Pour installer l'archive et l'en-tête sous `/usr/local` :

```sh
sudo make install
```

Le préfixe peut être personnalisé avec `make install PREFIX=/usr`.
Le nom `libreadline_stats` évite une collision avec la bibliothèque GNU
Readline, qui porte déjà le nom `libreadline`.

L'API lit un `FILE *` ouvert en mode binaire. L'appelant reste propriétaire du
flux et doit le fermer. Les options `READLINE_COUNT_LINES`, `READLINE_COUNT_WORDS`,
`READLINE_COUNT_BYTES` et `READLINE_COUNT_CHARACTERS` permettent de choisir les
compteurs ; `READLINE_COUNT_ALL` les active tous. Le statut retourné indique
une erreur d'argument, de lecture ou d'encodage UTF-8.

## Utiliser le CLI de démonstration

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

Un client complet qui utilise la bibliothèque est disponible dans
`examples/count_file.c`. Construis-le avec `make example`, puis lance
`./build/count_file chemin/vers/fichier.txt`.

## Tests

```sh
make test
```

La suite est séparée en trois catégories, exécutables indépendamment :

```sh
make test-unit
make test-functional
make test-integration
```

- Les tests unitaires Unity vérifient l'API de comptage, les compteurs, les
  limites UTF-8 et les erreurs de flux.
- Les tests fonctionnels lancent le CLI pour vérifier les options, les sorties
  et les erreurs observables, avec des fautes contrôlées pour les chemins
  d'erreur difficiles à déclencher naturellement.
- Les tests d'intégration construisent et exécutent un client distinct lié à la
  bibliothèque statique.

`make test` exécute les trois catégories. Unity est référencé dans
`vendor/unity/` par un commit précis.

Pour exécuter les tests avec l'instrumentation de couverture et générer un
rapport de lignes et de branches :

```sh
make clean coverage
```

Les rapports détaillés sont générés sous `build/coverage/`. GitHub Actions
effectue également ce calcul à chaque push et pull request. Le statut des tests
est visible dans les checks GitHub, le résumé de l'exécution affiche les
résultats et les pourcentages de couverture, et les rapports détaillés sont
publiés comme artefact téléchargeable. Avec GCC/gcov, les tests couvrent 100 %
des lignes et des branches de `src/main.c` et `src/count.c` ; `make coverage`
échoue si ce seuil n'est plus atteint.

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
