#!/bin/sh

set -eu

program=${1:-build/readline-faults}
case $program in
    /*) ;;
    *) program="$(cd "$(dirname "$program")" && pwd)/$(basename "$program")" ;;
esac
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
printf 'content\n' > "$test_dir/input.txt"

if READLINE_TEST_INVALID_ARGUMENT=1 "$program" -l "$test_dir/input.txt" \
    > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : le statut argument invalide devrait échouer\n' >&2
    exit 1
fi
if ! grep -Fq 'argument invalide' "$test_dir/output"; then
    printf 'ÉCHEC : le statut argument invalide devrait être signalé\n' >&2
    exit 1
fi
printf 'OK : statut argument invalide signalé\n'

if READLINE_TEST_CLOSE_FAILURE=1 "$program" -l "$test_dir/input.txt" \
    > "$test_dir/output" 2>&1; then
    printf 'ÉCHEC : une erreur de fermeture devrait échouer\n' >&2
    exit 1
fi
if ! grep -Fq "$test_dir/input.txt" "$test_dir/output"; then
    printf 'ÉCHEC : l’erreur de fermeture devrait identifier le fichier\n' >&2
    exit 1
fi
printf 'OK : erreur de fermeture signalée\n'
