#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory.h"

unsigned char Instruction[INSTRUCTION_MEMORY_SIZE];
unsigned char Data[DATA_MEMORY_SIZE];

static int read_hex_byte(FILE *fp, unsigned char *value)
{
    unsigned int x;
    if (fscanf(fp, "%x", &x) != 1)
        return 0;
    if (x > 255)
        return 0;
    *value = (unsigned char)x;
    return 1;
}

void initialize(const char *program_file, const char *data_file)
{
    FILE *fp;
    int i = 0;
    unsigned char value;

    for (i = 0; i < INSTRUCTION_MEMORY_SIZE; ++i)
        Instruction[i] = 0;
    for (i = 0; i < DATA_MEMORY_SIZE; ++i)
        Data[i] = 0;

    fp = fopen(program_file, "r");
    if (!fp) {
        printf("Unable to open %s\n", program_file);
        return;
    }
    i = 0;
    while (i < INSTRUCTION_MEMORY_SIZE && read_hex_byte(fp, &value))
        Instruction[i++] = value;
    fclose(fp);

    fp = fopen(data_file, "r");
    if (!fp) {
        printf("Unable to open %s\n", data_file);
        return;
    }
    i = 0;
    while (i < DATA_MEMORY_SIZE && read_hex_byte(fp, &value))
        Data[i++] = value;
    fclose(fp);
}

void finalize(const char *filename)
{
    FILE *fp = fopen(filename, "w");
    int i;

    if (!fp) {
        printf("Unable to write %s\n", filename);
        return;
    }

    /* Four bytes per line, as required by the lab specification. */
    for (i = 0; i < DATA_MEMORY_SIZE; i += 4) {
        fprintf(fp, "%02X %02X %02X %02X\n",
                Data[i], Data[i + 1], Data[i + 2], Data[i + 3]);
    }
    fclose(fp);
}
