#!/bin/sh

set -eu

program=${1:-build/count_file}
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM

printf 'one two\nthree\n' > "$test_dir/input.txt"
expected='Lignes : 2
Mots : 3
Octets : 14
Caractères : 14'
actual=$("$program" "$test_dir/input.txt")

if [ "$actual" != "$expected" ]; then
    printf 'ÉCHEC : le client de bibliothèque a produit un résultat inattendu\n' >&2
    printf 'Attendu :\n%s\nObtenu :\n%s\n' "$expected" "$actual" >&2
    exit 1
fi

printf 'OK : intégration d’un client lié à la bibliothèque\n'
