#include <stdio.h>
#include "memory.h"

unsigned char inst[256];
unsigned char Data[256];

void initialize(const char *program_file, const char *data_file_name)
{
    FILE *program_file_ptr;
    FILE *data_file;

    int val;
    int index = 0;

    /* Load Instruction Memory */

    program_file_ptr = fopen(program_file, "r");

    if (program_file_ptr == NULL)
    {
        printf("Unable to open %s\n", program_file);
        return;
    }

    while (fscanf(program_file_ptr, "%d", &val) == 1 && index < 256)
    {
        inst[index++] = (unsigned char)val;
    }

    fclose(program_file_ptr);

    /* Load Data Memory */

    data_file = fopen(data_file_name, "r");

    if (data_file == NULL)
    {
        printf("Unable to open %s\n", data_file_name);
        return;
    }

    index = 0;

    while (fscanf(data_file, "%d", &val) == 1 && index < 256)
    {
        Data[index++] = (unsigned char)val;
    }

    fclose(data_file);
}
void finalize(const char *filename)
{
    FILE *data_file;
    int counter;

    data_file = fopen(filename, "w");

    if (data_file == NULL)
    {
        printf("Unable to write %s\n", filename);
        return;
    }

    for (counter = 0; counter < 256; counter++)
    {
        fprintf(data_file, "%d ", Data[counter]);
    }

    fclose(data_file);
}