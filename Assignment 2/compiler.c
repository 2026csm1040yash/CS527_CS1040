#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

#define MAX_LINES 256
#define MAX_LABELS 256
#define MAX_LINE 256

typedef struct {
    char name[64];
    int instruction_index;
} Label;

typedef struct {
    char text[MAX_LINE];
    int instruction_index;
} SourceLine;

static Label labels[MAX_LABELS];
static int label_count;
static SourceLine lines[MAX_LINES];
static int line_count;

static char *trim(char *s)
{
    char *end;
    while (isspace((unsigned char)*s)) ++s;
    end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) --end;
    *end = '\0';
    return s;
}

static void remove_comment(char *s)
{
    char *p = strchr(s, '%');
    if (p) *p = '\0';
}

static int parse_reg(const char *s, int *r)
{
    char extra;
    if (sscanf(s, " x%d %c", r, &extra) == 1 ||
        sscanf(s, "x%d%c", r, &extra) == 1) {
        return *r >= 0 && *r <= 255;
    }
    return 0;
}

static int parse_operand(const char *s, int *is_const, int *value)
{
    char buf[64];
    char extra;
    int r;

    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *p = trim(buf);

    if (*p == 'x' && sscanf(p, "x%d %c", &r, &extra) == 1) {
        if (r >= 0 && r <= 255) {
            *is_const = 0;
            *value = r;
            return 1;
        }
    }

    if (sscanf(p, "%d %c", value, &extra) == 1 && *value >= 0 && *value <= 255) {
        *is_const = 1;
        return 1;
    }

    return 0;
}

static int add_label(const char *name, int index)
{
    int i;
    for (i = 0; i < label_count; ++i)
        if (strcmp(labels[i].name, name) == 0)
            return 0;

    if (label_count >= MAX_LABELS)
        return 0;

    strncpy(labels[label_count].name, name, sizeof(labels[label_count].name) - 1);
    labels[label_count].name[sizeof(labels[label_count].name) - 1] = '\0';
    labels[label_count].instruction_index = index;
    ++label_count;
    return 1;
}

static int find_label(const char *name)
{
    int i;
    for (i = 0; i < label_count; ++i)
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].instruction_index;
    return -1;
}

static void print_bytecode(FILE *out, int op, int d, int s1, int s2)
{
    fprintf(out, "%X %X %X %X\n", op, d, s1, s2);
    printf("%X %X %X %X\n", op, d, s1, s2);
}

static int branch_code(const char *mnemonic)
{
    static const char *names[] = {
        "BEQ", "BNE", "BCS", "BCC", "BMI", "BPL", "BVS", "BVC",
        "BHI", "BLS", "BGE", "BLT", "BGT", "BLE", "BAL"
    };
    int i;
    for (i = 0; i < 15; ++i)
        if (strcmp(mnemonic, names[i]) == 0)
            return i;
    return -1;
}

void compile(const char *sourceFile, const char *bytecodeFile)
{
    FILE *src = fopen(sourceFile, "r");
    FILE *out;
    char raw[MAX_LINE];
    int instruction_index = 0;
    int i;

    label_count = 0;
    line_count = 0;

    if (!src) {
        printf("Unable to open %s\n", sourceFile);
        return;
    }

    /* First pass: save source lines and resolve label addresses. */
    while (fgets(raw, sizeof(raw), src) && line_count < MAX_LINES) {
        char line[MAX_LINE];
        char *p;

        strncpy(line, raw, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';
        remove_comment(line);
        p = trim(line);
        if (*p == '\0') continue;

        if (*p == '.') {
            char label[64];
            if (sscanf(p, "%63s", label) == 1) {
                if (strchr(label, ':') == NULL)
                    add_label(label, instruction_index);
            }
            continue;
        }

        strncpy(lines[line_count].text, p, MAX_LINE - 1);
        lines[line_count].text[MAX_LINE - 1] = '\0';
        lines[line_count].instruction_index = instruction_index++;
        ++line_count;
    }
    fclose(src);

    out = fopen(bytecodeFile, "w");
    if (!out) {
        printf("Unable to create %s\n", bytecodeFile);
        return;
    }

    /* Second pass: generate bytecode. */
    for (i = 0; i < line_count; ++i) {
        char line[MAX_LINE];
        char mnemonic[32];
        int d, a, b, is_const;
        char op;
        char left[64], right[64];
        char address_text[64], value_text[64];
        int branch;

        strncpy(line, lines[i].text, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';

        /* Legacy Read/Write: first operand is now zero. */
        if (sscanf(line, "Read x%d , %d", &d, &a) == 2 ||
            sscanf(line, "Read x%d %d", &d, &a) == 2) {
            print_bytecode(out, 0x05, d, 0, a);
            continue;
        }

        if (sscanf(line, "Write x%d , %d", &d, &a) == 2 ||
            sscanf(line, "Write x%d %d", &d, &a) == 2) {
            print_bytecode(out, 0x06, d, 0, a);
            continue;
        }

        /* Branch instruction. */
        if (sscanf(line, "%31s %63s", mnemonic, address_text) == 2 &&
            (branch = branch_code(mnemonic)) >= 0) {
            int target = find_label(trim(address_text));
            int offset;
            if (target < 0) {
                printf("Unknown label: %s\n", address_text);
                continue;
            }
            /* Lab example uses target instruction index - current instruction index. */
            offset = target - lines[i].instruction_index;
            if (offset < -128 || offset > 127) {
                printf("Branch offset out of range: %d\n", offset);
                continue;
            }
            print_bytecode(out, 0x10 + branch, 0, 0, offset & 0xFF);
            continue;
        }

        /* [address] = value */
        if (sscanf(line, "[%63[^]]] = %63s", address_text, value_text) == 2) {
            if (!parse_operand(address_text, &is_const, &a) ||
                !parse_operand(value_text, &is_const, &b)) {
                printf("Invalid memory write: %s\n", line);
                continue;
            }

            /* Re-parse address to know whether it is constant. */
            {
                int address_is_const;
                if (!parse_operand(address_text, &address_is_const, &a))
                    continue;
                if (address_is_const)
                    print_bytecode(out, 0x0E, 0, a, b);
                else
                    print_bytecode(out, 0x06, a, 0, b);
            }
            continue;
        }

        /* xD = [address] */
        if (sscanf(line, "x%d = [%63[^]]]", &d, address_text) == 2) {
            if (!parse_operand(address_text, &is_const, &a)) {
                printf("Invalid memory read: %s\n", line);
                continue;
            }
            if (is_const)
                print_bytecode(out, 0x0D, d, 0, a);
            else
                print_bytecode(out, 0x05, d, 0, a);
            continue;
        }

        /* Arithmetic: xD = xA op xB or xD = xA op constant. */
        if (sscanf(line, "%63[^=] = %63[^ ] %c %63s", left, right, &op, value_text) == 4) {
            if (!parse_reg(trim(left), &d) || !parse_operand(trim(right), &is_const, &a)) {
                printf("Invalid arithmetic instruction: %s\n", line);
                continue;
            }
            if (!parse_operand(value_text, &is_const, &b)) {
                printf("Invalid arithmetic operand: %s\n", value_text);
                continue;
            }

            /* Determine whether second operand is a constant by parsing value_text. */
            {
                int rhs_const;
                if (!parse_operand(value_text, &rhs_const, &b))
                    continue;

                int opcode = 0;
                switch (op) {
                    case '+': opcode = rhs_const ? 0x09 : 0x01; break;
                    case '-': opcode = rhs_const ? 0x0A : 0x02; break;
                    case '*': opcode = rhs_const ? 0x0B : 0x03; break;
                    case '/': opcode = rhs_const ? 0x0C : 0x04; break;
                    default: opcode = 0; break;
                }
                if (!opcode) {
                    printf("Invalid operator: %c\n", op);
                    continue;
                }
                print_bytecode(out, opcode, d, a, b);
            }
            continue;
        }

        /* Data movement: xD = constant.
           The reference bytecode example uses 0x0F (displayed as F)
           for constant-to-register movement. */
        if (sscanf(line, "x%d = %d", &d, &a) == 2 && d >= 0 && d <= 255 && a >= 0 && a <= 255) {
            print_bytecode(out, 0x0F, d, 0, a);
            continue;
        }

        printf("Unable to compile line: %s\n", line);
    }

    /* Do not append a synthetic HALT instruction. The instruction memory is
       zero-initialized, and opcode 0 terminates execution when the processor
       reaches the first unused instruction slot. This also matches the
       supplied reference bytecode example exactly. */
    fclose(out);
}
