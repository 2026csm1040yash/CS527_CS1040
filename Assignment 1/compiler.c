#include <stdio.h>
#include <string.h>

#include "compiler.h"

void compile(const char *sourceFile, const char *bytecodeFile)
{
    FILE *src, *bytecode;
    char line[256];
    src = fopen(sourceFile, "r");
    if (src == NULL)
    {
        printf("Unable to open %s\n", sourceFile);
        return;
    }

    bytecode = fopen(bytecodeFile, "w");
    if (bytecode == NULL)
    {
        printf("Unable to create %s\n", bytecodeFile);
        fclose(src);
        return;
    }

    while (fgets(line, sizeof(line), src))
    {
        int destination, src_1, src_2, address;
        char op;

        /* Read instruction */

        if (sscanf(line, "Read x%d, %d", &destination, &address) == 2)
        {
            fprintf(bytecode, "5 %d %d 0\n", destination, address);
            printf("5 %d %d 0\n", destination, address);
        }

        /* Write instruction */

        else if (sscanf(line, "Write x%d, %d", &destination, &address) == 2)
        {
            fprintf(bytecode, "6 %d %d 0\n", destination, address);
            printf("6 %d %d 0\n", destination, address);
        }

        /* Arithmetic instruction */

        else if (sscanf(line,
                        "x%d = x%d %c x%d",
                        &destination,
                        &src_1,
                        &op,
                        &src_2) == 4)
        {
            int opcode = 0;

            switch (op)
            {
                case '+':
                    opcode = 1;
                    break;

                case '-':
                    opcode = 2;
                    break;

                case '*':
                    opcode = 3;
                    break;

                case '/':
                    opcode = 4;
                    break;
            }

            fprintf(bytecode,
                    "%d %d %d %d\n",
                    opcode,
                    destination,
                    src_1,
                    src_2);

            printf("%d %d %d %d\n",
                   opcode,
                   destination,
                   src_1,
                   src_2);
        }
    }

    /* End of program */

    fprintf(bytecode, "0 0 0 0\n");

    fclose(src);
    fclose(bytecode);
}