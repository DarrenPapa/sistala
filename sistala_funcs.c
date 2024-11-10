#ifndef SISTALA_H
#define SISTALA_H

//includes
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

//typesnmore
#define STACK_SIZE 1024
#define MAX_PROGRAM_SIZE 1024
#define MAX_LINE_LEN 40
#define MAX_LABELS 128
#define MAX_LABEL_LENGTH 32
typedef struct {
    int data[STACK_SIZE];
    int top;
} Stack;
typedef struct {
    char name[MAX_LABEL_LENGTH];
    int position;
} Label;

//flags
Stack call_stack;
Label labels[MAX_LABELS];
int repl = 0;
int label_count = 0;
//func
// forward decs
int interpret(Stack *stack, const char *program[], int *pc);

// norm

void push(Stack *stack, int value) {
    if (stack->top < STACK_SIZE) {
        stack->data[stack->top++] = value;
    } else {
        fprintf(stderr, "Stack overflow\n");
        if (!repl) exit(1);
    }
}
int pop(Stack *stack) {
    if (stack->top > 0) {
        return stack->data[--stack->top];
    } else {
        fprintf(stderr, "Stack underflow\n");
        if (!repl) exit(1);
    }
    return 0;
}
int peek(Stack *stack) {
    if (stack->top > 0) {
        return stack->data[stack->top-1];
    } else {
        fprintf(stderr, "Stack underflow\n");
        if (!repl) exit(1);
    }
    return 0;
}
// Rotate
void rot(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, b); push(stack, a);
}
// Arithmetic operations
void add(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a + b);
}
void subtract(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a - b);
}
void multiply(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a * b);
}
void divide(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    if (b != 0) {
        push(stack, a / b);
    } else {
        fprintf(stderr, "Division by zero\n");
        if (!repl) exit(1);
    }
}
// Comparisons
void cmp(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a == b);
}
void is_gt(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a > b);
}
void is_lt(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a < b);
}
void is_ge(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a >= b);
}
void is_le(Stack *stack) {
    int b = pop(stack);
    int a = pop(stack);
    push(stack, a <= b);
}
// Character input/output operations
void pchar(Stack *stack) {
    if (stack->top > 0) {
        printf("%c", (char)pop(stack));
    } else {
        fprintf(stderr, "Stack underflow on outc\n");
        if (!repl) exit(1);
    }
}
void rchar(Stack *stack) {
    char ch;
#ifdef _WIN32
    ch = getch();
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    push(stack, ch);
}
// Label management functions
void add_label(const char *name, int position) {
    if (label_count < MAX_LABELS) {
        strncpy(labels[label_count].name, name, MAX_LABEL_LENGTH);
        labels[label_count].position = position;
        label_count++;
    } else {
        fprintf(stderr, "Label limit reached.\n");
    }
}
int find_label(const char *name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].position;
        }
    }
    fprintf(stderr, "Label not found: %s\n", name);
    if (!repl) exit(1);
    return -1; // return anyway (-.-)
}
int load_program(const char *filename, const char *program[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Could not open file %s\n", filename);
        if (!repl) exit(1);
    }
    char *line = (char*)malloc(sizeof(char) * MAX_LINE_LEN);
    int count = 0;
    while (fscanf(file, "%38s", line) == 1 && count < MAX_PROGRAM_SIZE) {
        // Detect labels and store their position
        if (line[0] == '.') {
            *line++;  // remove the dot
            add_label(line, count);
        } else {
            program[count] = strdup(line);  // Copy each token into the program
            count++;
        }
    }
    free(line);
    fclose(file);
    return count;
}
int load_from_string(const char *input, const char *program[]) {
    const char *ptr = input;
    char *line = (char*)malloc(sizeof(char) * MAX_LINE_LEN);
    int count = 0;

    while (sscanf(ptr, "%38s", line) == 1 && count < MAX_PROGRAM_SIZE) {
        ptr += strlen(line);
        while (*ptr == ' ' || *ptr == '\n' || *ptr == '\t') {
            ptr++;
        }
        if (line[0] == '.') {
            *line++;  // remove the dot
            add_label(line, count);
        } else {
            program[count] = strdup(line);
            count++;
        }
    }
    free(line);
    return count;
}
int run_from_string(Stack *stack, const char* filename) {
    const char *program[MAX_PROGRAM_SIZE] = {NULL};
    int program_size = load_from_string(filename, program);

    if (program_size < 0) {
        return program_size;
    }

    int pc = 0;
    while (pc < program_size)
    {
        if (!interpret(stack, program, &pc)) {
            for (int i = 0; i < program_size; i++) {
                free((void *)program[i]);
            }
            return 1;
        }
        pc++;
    }

    for (int i = 0; i < program_size; i++) {
        free((void *)program[i]);
    }
    return 0;
}
int run_from_file(Stack *stack, const char* code) {
    const char *program[MAX_PROGRAM_SIZE] = {NULL};
    int program_size = load_program(code, program);

    if (program_size < 0) {
        return program_size;
    }

    int pc = 0;
    while (pc < program_size)
    {
        if (!interpret(stack, program, &pc)) {
            for (int i = 0; i < program_size; i++) {
                free((void *)program[i]);
            }
            return 1;
        }
        pc++;
    }

    for (int i = 0; i < program_size; i++) {
        free((void *)program[i]);
    }
    return 0;
}
#ifdef _WIN32
    #include <windows.h>
    typedef int (*my_function_type)(Stack*);

    int load_and_call_library(const char *lib_name, const char *func_name, Stack *stack) {
        HINSTANCE hInstLibrary = LoadLibrary(lib_name);
        if (hInstLibrary == NULL) {
            return -1;
        }

        my_function_type my_function = (my_function_type)GetProcAddress(hInstLibrary, func_name);
        if (my_function == NULL) {
            FreeLibrary(hInstLibrary);
            return -2;
        }

        int result = my_function(stack);

        FreeLibrary(hInstLibrary);
        return result;
    }

#elif __linux__
    #include <dlfcn.h>
    typedef int (*my_function_type)(Stack*);

    int load_and_call_library(const char *lib_name, const char *func_name, Stack *stack) {
        void *handle = dlopen(lib_name, RTLD_LAZY);
        if (!handle) {
            return -1;
        }

        my_function_type my_function = (my_function_type)dlsym(handle, func_name);
        if (!my_function) {
            dlclose(handle);
            return -2;
        }

        int result = my_function(stack);

        dlclose(handle);
        return result;
    }

#else
    #error "Unsupported platform"
#endif

// Interpreter with named subroutine support
int interpret(Stack *stack, const char *program[], int *pc) {
    const char *token = program[*pc];
    int value;
    char ch;

    if (sscanf(token, "%d", &value) == 1) {
        push(stack, value);
    } else if (sscanf(token, "'%c'", &ch) == 1) {
        push(stack, ch);
    } else if (strcmp(token, "+") == 0) {
        add(stack);
    } else if (strcmp(token, "-") == 0) {
        subtract(stack);
    } else if (strcmp(token, "*") == 0) {
        multiply(stack);
    } else if (strcmp(token, "/") == 0) {
        divide(stack);
    } else if (strcmp(token, "outc") == 0) {
        pchar(stack);
    } else if (strcmp(token, "outv") == 0) {
        printf("%i", pop(stack));
    } else if (strcmp(token, "getc") == 0) {
        rchar(stack);
    } else if (strcmp(token, "rot") == 0) {
        rot(stack);
    } else if (strcmp(token, "call") == 0) {
        (*pc)++;
        int target = find_label(program[*pc]);
        if (target != -1) {
            push(&call_stack, *pc + 1);
            *pc = target - 1;
        }
    } else if (strcmp(token, "ciz") == 0) {
        (*pc)++;
        if (peek(stack) == 0) {
            int target = find_label(program[*pc]);
            if (target != -1) {
                push(&call_stack, *pc + 1);
                *pc = target - 1;
            }
        }
    } else if (strcmp(token, "((") == 0) {
        while (strcmp(program[*pc], "))") != 0)
            (*pc)++;
    } else if (strcmp(token, "cnz") == 0) {
        (*pc)++;
        if (peek(stack) != 0) {
            int target = find_label(program[*pc]);
            if (target != -1) {
                push(&call_stack, *pc + 1);
                *pc = target - 1;
            }
        }
    } else if (strcmp(token, "goto") == 0) {
        (*pc)++;
        int target = find_label(program[*pc]);
        if (target != -1) {
            *pc = target - 1;
        }
    } else if (strcmp(token, "giz") == 0) {
        (*pc)++;
        if (peek(stack) == 0) {
            int target = find_label(program[*pc]);
            if (target != -1) {
                *pc = target - 1;
            }
        }
    } else if (strcmp(token, "gnz") == 0) {
        (*pc)++;
        if (peek(stack) != 0) {
            int target = find_label(program[*pc]);
            if (target != -1) {
                *pc = target - 1;
            }
        }
    } else if (strcmp(token, "return") == 0) {
        if (call_stack.top > 0) {
            *pc = pop(&call_stack) - 1;
        } else {
            fprintf(stderr, "Call stack underflow on return\n");
        }
    } else if (strcmp(token, "halt") == 0) {
        exit(0);
    } else if (strcmp(token, "dupe") == 0) {
        push(stack, peek(stack));
    } else if (strcmp(token, "pop") == 0) {
        pop(stack);
    } else if (strcmp(token, "cmp") == 0) {
        cmp(stack);
    } else if (strcmp(token, "igt") == 0) {
        is_gt(stack);
    } else if (strcmp(token, "ilt") == 0) {
        is_lt(stack);
    } else if (strcmp(token, "ige") == 0) {
        is_ge(stack);
    } else if (strcmp(token, "ile") == 0) {
        is_le(stack);
    } else if (strcmp(token, "exit") == 0) {
        exit(0);
    } else if (strcmp(token, "sysexit") == 0) {
        exit(pop(stack));
    } else if (strcmp(token, "sfcall") == 0) {
        char *df_file, *df_func;
        df_file = strdup(program[++(*pc)]);
        df_func = strdup(program[++(*pc)]);
        switch (load_and_call_library(df_file, df_func, stack))
        {
            case -1:
                fprintf(stderr, "file '%s' was not found!\n", df_file);
                if (!repl) exit(1);
                break;
            case -2:
                fprintf(stderr, "function '%s' was not in file '%s'\n", df_func, df_file);
                if (!repl) exit(1);
                break;
        }
            
        free(df_file); free(df_func);
    }
    // Ignore labels during execution
    else if (strchr(token, ':') != NULL) {
    } else {
        fprintf(stderr, "Unknown command: %s\n", token);
        return 0;
    }
    return 1;
}

Stack* init_sistala(void) {
    call_stack.top = 0;
    Stack stack = {.top=0};
    return &stack;
}

#endif
