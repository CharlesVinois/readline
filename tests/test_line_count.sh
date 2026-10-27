#!/bin/sh

set -eu

program=${1:-build/readline}
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM

# Lance le programme avec ses arguments et compare sa sortie attendue.
check_output() {
    name=$1
    expected=$2
    shift 2

    actual=$("$program" "$@")
    if [ "$actual" != "$expected" ]; then
        printf 'ÉCHEC : %s (attendu : "%s", obtenu : "%s")\n' \
            "$name" "$expected" "$actual" >&2
        exit 1
    fi

    printf 'OK : %s\n' "$name"
}

# Un fichier vide produit zéro pour chacun des compteurs.
: > "$test_dir/empty.txt"
check_output "fichier vide" \
    "Lignes : 0
Mots : 0
Octets : 0
Caractères : 0" \
    -l -w -c -m "$test_dir/empty.txt"

# Vérifie toutes les opérations en une seule lecture et avec options groupées.
printf 'one two\nthree\n' > "$test_dir/multiple.txt"
check_output "options combinées" \
    "Lignes : 2
Mots : 3
Octets : 14
Caractères : 14" \
    -lwcm "$test_dir/multiple.txt"

# Un caractère accentué compte pour plusieurs octets mais un caractère.
printf 'été\n' > "$test_dir/utf8.txt"
check_output "comptage UTF-8" \
    "Octets : 6
Caractères : 4" \
    --bytes --characters "$test_dir/utf8.txt"

# Le contenu final compte comme une ligne même sans '\n'.
printf 'one\ntwo' > "$test_dir/no-final-newline.txt"
check_output "dernière ligne sans saut de ligne final" \
    "Lignes : 2" -l "$test_dir/no-final-newline.txt"

# Sans opération demandée, le programme affiche l'aide.
check_output "aide sans option" \
    "Utilisation : $program [OPTIONS] <fichier>
Compte différentes caractéristiques du fichier.
Sans option, affiche cette aide.

Options :
  -l, --lines       compter les lignes
  -w, --words       compter les mots
  -c, --bytes       compter les octets
  -m, --characters  compter les caractères UTF-8
  -h, --help        afficher cette aide

Les options de comptage peuvent être combinées." \
    "$test_dir/multiple.txt"

# L'option d'aide explicite affiche le même mode d'emploi.
check_output "option d'aide" \
    "Utilisation : $program [OPTIONS] <fichier>
Compte différentes caractéristiques du fichier.
Sans option, affiche cette aide.

Options :
  -l, --lines       compter les lignes
  -w, --words       compter les mots
  -c, --bytes       compter les octets
  -m, --characters  compter les caractères UTF-8
  -h, --help        afficher cette aide

Les options de comptage peuvent être combinées." \
    --help

# Un chemin absent ou inexistant doit provoquer un échec.
if "$program" -l > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : un fichier manquant devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : fichier manquant signalé\n'

if "$program" -l "$test_dir/missing.txt" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : un fichier inexistant devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : fichier inexistant signalé\n'

# Les options inconnues et les séquences UTF-8 mal formées sont des erreurs.
if "$program" -x "$test_dir/multiple.txt" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : une option inconnue devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : option inconnue signalée\n'

printf '\377' > "$test_dir/invalid-utf8.txt"
if "$program" -m "$test_dir/invalid-utf8.txt" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : un fichier UTF-8 invalide devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : UTF-8 invalide signalé\n'

printf 'Tous les tests ont réussi.\n'
