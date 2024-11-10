#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sistala_funcs.c"

char version[] = "v0.1";

int main(int argc, char *argv[]) {
    Stack *stack = init_sistala();
    char cmd[500] = {0};
    const char *program[MAX_PROGRAM_SIZE] = {NULL};
    int program_size = 0;
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("SiStaLa Help Menu (for %s)\nTo run a file use:\n  %s <filename>\n", version, argv[0]);
            exit(0);
        } else {
            if (!run_from_file(stack, argv[1])) {
                return 1;
            }
        }
    } else {
        repl = 1;
        printf("If you expected a help menu please run this as '%s --help'\nSiStaLa REPL (%s)\n", argv[0], version);
        while (1) {
            printf(">>> ");scanf("%[^\n]s", cmd);
            getchar();
            run_from_string(stack, cmd);
        }
    }
    return 0;
}
