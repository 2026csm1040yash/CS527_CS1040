#include <stdio.h>
#include "compiler.h"
#include "memory.h"
#include "processor.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <program_file> <data_file>\n", argv[0]);
        return 1;
    }

    compile(argv[1], "program.byte");
    initialize("program.byte", argv[2]);
    reset();

    while (!end_of_simulation)
    {
        fetch();
        execute();
    }

    printf("Writing updated data to file...\n");
    finalize(argv[2]);

    printf("Simulation completed successfully.\n");

    return 0;
}