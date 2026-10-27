#!/bin/sh

set -eu

program=${1:-build/readline}
case $program in
    /*) ;;
    *) program="$(cd "$(dirname "$program")" && pwd)/$(basename "$program")" ;;
esac
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

if ! "$program" > "$test_dir/no-args-output"; then
    printf 'ÉCHEC : le programme sans argument devrait afficher l’aide\n' >&2
    exit 1
fi
"$program" --help > "$test_dir/help-output"
if ! cmp -s "$test_dir/no-args-output" "$test_dir/help-output"; then
    printf 'ÉCHEC : l’aide sans argument est différente de --help\n' >&2
    exit 1
fi
printf 'OK : aide sans argument\n'

if ! "$program" -h > "$test_dir/short-help-output"; then
    printf 'ÉCHEC : -h devrait afficher l’aide\n' >&2
    exit 1
fi
if ! cmp -s "$test_dir/short-help-output" "$test_dir/help-output"; then
    printf 'ÉCHEC : l’aide -h est différente de --help\n' >&2
    exit 1
fi
printf 'OK : option courte d’aide\n'

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

printf '\303' > "$test_dir/incomplete-utf8.txt"
if "$program" -m "$test_dir/incomplete-utf8.txt" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : un UTF-8 incomplet devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : UTF-8 incomplet signalé\n'

check_output "options courtes séparées" \
    "Lignes : 2
Mots : 3" \
    -l -w "$test_dir/multiple.txt"

check_output "options longues" \
    "Lignes : 2
Mots : 3
Octets : 14
Caractères : 14" \
    --lines --words --bytes --characters "$test_dir/multiple.txt"

# Le séparateur permet de traiter un chemin commençant par un tiret comme un nom.
cp "$test_dir/multiple.txt" "$test_dir/-input.txt"
(
    cd "$test_dir"
    check_output "séparateur d'options" "Lignes : 2" -l -- -input.txt
)

check_output "option longue de comptage seule" \
    "Lignes : 2" --lines "$test_dir/multiple.txt"

check_output "comptage des mots seul" \
    "Mots : 3" --words "$test_dir/multiple.txt"

check_output "comptage des caractères seul" \
    "Caractères : 14" --characters "$test_dir/multiple.txt"

cp "$test_dir/multiple.txt" "$test_dir/-"
(
    cd "$test_dir"
    check_output "nom de fichier tiret seul" "Lignes : 2" -l -
)

if "$program" --unknown "$test_dir/multiple.txt" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : une option longue inconnue devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : option longue inconnue signalée\n'

if ! "$program" -lh "$test_dir/multiple.txt" > "$test_dir/output"; then
    printf 'ÉCHEC : l’aide devrait primer sur les autres options\n' >&2
    exit 1
fi
if ! cmp -s "$test_dir/output" "$test_dir/help-output"; then
    printf 'ÉCHEC : l’aide combinée aux options est différente de --help\n' >&2
    exit 1
fi
printf 'OK : aide prioritaire sur les autres options\n'

if "$program" -l "$test_dir/multiple.txt" "$test_dir/empty.txt" \
    > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : plusieurs fichiers devraient produire une erreur\n' >&2
    exit 1
fi
printf 'OK : plusieurs fichiers signalés\n'

if ! "$program" "$test_dir/multiple.txt" > "$test_dir/output"; then
    printf 'ÉCHEC : un fichier sans opération devrait afficher l’aide\n' >&2
    exit 1
fi
if ! cmp -s "$test_dir/output" "$test_dir/help-output"; then
    printf 'ÉCHEC : l’aide sans opération est différente de --help\n' >&2
    exit 1
fi
printf 'OK : aide affichée si aucune opération n’est demandée\n'

if "$program" -l "$test_dir" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : une erreur de lecture devrait produire un échec\n' >&2
    exit 1
fi
printf 'OK : erreur de lecture signalée\n'

printf 'Tous les tests ont réussi.\n'
