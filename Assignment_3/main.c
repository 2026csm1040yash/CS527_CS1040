#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "memory.h"
#include "processor.h"

int main(int argc, char *argv[])
{
    const char *program_file;
    const char *data_file;

    if (argc != 3) {
        printf("Usage: %s <program.txt|program.byte> <data.byte>\n", argv[0]);
        return 1;
    }

    program_file = argv[1];
    data_file = argv[2];

    if (strstr(program_file, ".byte") == NULL) {
        compile(program_file, "program.byte");
        program_file = "program.byte";
    }

    initialize(program_file, data_file);
    reset();

    while (!end_of_simulation) {
        fetch();
        decode();
        execute();
    }

    finalize(data_file);
    return 0;
}
