#include <stdio.h>

int main(int argc, char *argv[])
{
    /* Le programme attend exactement un chemin de fichier en argument. */
    if (argc != 2) {
        /* Écrit le mode d'emploi sur la sortie réservée aux erreurs. */
        fprintf(stderr, "Utilisation : %s <fichier>\n", argv[0]);
        return 1;
    }

    /* Ouvre le fichier en lecture ; fopen renvoie NULL si l'ouverture échoue. */
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        /* Affiche le message système associé à l'erreur d'ouverture. */
        perror(argv[1]);
        return 1;
    }

    unsigned long long lines = 0;
    int character;
    /* Sert à repérer une dernière ligne qui n'aurait pas de '\n' final. */
    int last_character = '\n';

    /* fgetc lit un caractère ; EOF marque la fin du fichier ou une erreur. */
    while ((character = fgetc(file)) != EOF) {
        /* Chaque saut de ligne termine une ligne complète. */
        if (character == '\n') {
            lines++;
        }
        last_character = character;
    }

    /* ferror distingue une erreur de lecture d'une fin de fichier normale. */
    if (ferror(file)) {
        /* Signale l'erreur système, puis ferme le fichier avant de quitter. */
        perror(argv[1]);
        fclose(file);
        return 1;
    }

    /* Le contenu final compte comme une ligne même sans saut de ligne. */
    if (last_character != '\n') {
        lines++;
    }

    /* fclose ferme le flux et renvoie EOF si la fermeture échoue. */
    if (fclose(file) == EOF) {
        /* Affiche la cause système de l'erreur de fermeture. */
        perror(argv[1]);
        return 1;
    }

    /* printf affiche le résultat sur la sortie standard. */
    printf("%llu ligne(s)\n", lines);
    return 0;
}
