#!/bin/sh

set -eu

program=${1:-build/readline}
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM

# Lance le programme et compare sa sortie à la valeur attendue.
check_count() {
    name=$1
    expected=$2
    file=$3

    actual=$("$program" "$file")
    if [ "$actual" != "$expected" ]; then
        printf 'ÉCHEC : %s (attendu : "%s", obtenu : "%s")\n' \
            "$name" "$expected" "$actual" >&2
        exit 1
    fi

    printf 'OK : %s\n' "$name"
}

# Un fichier vide ne contient aucune ligne.
: > "$test_dir/empty.txt"
check_count "fichier vide" "0 ligne(s)" "$test_dir/empty.txt"

# Vérifie le comptage d'une ligne terminée par un saut de ligne.
printf 'première ligne\n' > "$test_dir/one-line.txt"
check_count "une ligne terminée par un saut de ligne" \
    "1 ligne(s)" "$test_dir/one-line.txt"

# Vérifie plusieurs lignes terminées par un saut de ligne.
printf 'première ligne\ndeuxième ligne\ntroisième ligne\n' \
    > "$test_dir/multiple-lines.txt"
check_count "plusieurs lignes terminées par un saut de ligne" \
    "3 ligne(s)" "$test_dir/multiple-lines.txt"

# La dernière ligne compte également sans saut de ligne final.
printf 'première ligne\ndeuxième ligne' > "$test_dir/no-final-newline.txt"
check_count "dernière ligne sans saut de ligne final" \
    "2 ligne(s)" "$test_dir/no-final-newline.txt"

# Un chemin inexistant doit être signalé avec un code de sortie non nul.
if "$program" "$test_dir/missing.txt" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : un fichier inexistant devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : fichier inexistant signalé\n'

# L'absence du chemin d'entrée doit également produire une erreur.
if "$program" > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : l’absence d’argument devrait produire une erreur\n' >&2
    exit 1
fi
printf 'OK : argument manquant signalé\n'

printf 'Tous les tests ont réussi.\n'
