# readline

[![CI](https://github.com/CharlesVinois/readline/actions/workflows/ci.yml/badge.svg)](https://github.com/CharlesVinois/readline/actions/workflows/ci.yml)

Bibliothèque C17 pour compter les lignes, les mots, les octets et les caractères
UTF-8 d'un flux, avec un exécutable CLI de démonstration.

## Prérequis

- Un compilateur C (par exemple GCC ou Clang)
- GNU Make
- `pkg-config` pour les tests d'intégration et l'utilisation simplifiée par les
  clients

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

L'installation inclut également `readline_stats.pc` sous
`$(PREFIX)/lib/pkgconfig`. Après installation, un client peut obtenir les
options nécessaires avec `pkg-config` :

```sh
cc -std=c17 $(pkg-config --cflags readline_stats) client.c \
  $(pkg-config --libs readline_stats) -o client
```

Si la bibliothèque est installée dans un préfixe non standard, ajoutez son
répertoire pkg-config à `PKG_CONFIG_PATH`, par exemple :

```sh
export PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig"
```

Le fichier pkg-config annonce la version `0.1.0` par défaut ; elle peut être
ajustée lors de l'installation avec `VERSION`, par exemple
`make install PREFIX=/usr VERSION=1.0.0`.

Pour vérifier l'intégration depuis un répertoire temporaire, sans installer
dans le système :

```sh
make test-package
```

L'API lit un `FILE *` lisible, ouvert en mode binaire, de sa position courante
jusqu'à EOF. Elle ne rembobine ni ne ferme le flux : l'appelant en garde la
propriété et doit le fermer. En cas de succès, les champs demandés sont
renseignés, les autres valent zéro et le flux est à EOF. En cas d'erreur, la
structure de résultats reste inchangée, mais le flux peut avoir avancé.

Les options `READLINE_COUNT_LINES`, `READLINE_COUNT_WORDS`,
`READLINE_COUNT_BYTES` et `READLINE_COUNT_CHARACTERS` sont combinables ;
`READLINE_COUNT_ALL` les active toutes. Les lignes comptent les `\n` ainsi
qu'une dernière ligne non vide sans saut de ligne final. Les mots sont des
séquences d'octets séparées par les espaces reconnus par `isspace` dans la
locale courante. Les caractères sont des points de code Unicode en UTF-8
valide, pas des graphèmes : une lettre suivie d'un accent combinant compte
comme deux caractères. Le statut distingue les arguments invalides, les
erreurs de lecture et les séquences UTF-8 invalides ou incomplètes.

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

Pour détecter des erreurs mémoire et des comportements indéfinis :

```sh
make clean sanitize
```

Cette cible reconstruit les programmes et les trois suites dans
`build/sanitize/` avec AddressSanitizer et UndefinedBehaviorSanitizer, sans
remplacer les artefacts ordinaires.

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
La CI exécute aussi les trois suites avec GCC et Clang pour vérifier la
portabilité du code et des tests.

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
