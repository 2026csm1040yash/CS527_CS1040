#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

#define MAX_LINES 512
#define MAX_LABELS 256
#define MAX_LINE 256

typedef struct { char name[64]; int instruction_index; } Label;
typedef struct { char text[MAX_LINE]; int instruction_index; } SourceLine;
static Label labels[MAX_LABELS]; static int label_count;
static SourceLine lines[MAX_LINES]; static int line_count;

static char *trim(char *s){ char *e; while(isspace((unsigned char)*s))s++; e=s+strlen(s); while(e>s&&isspace((unsigned char)e[-1]))--e; *e='\0'; return s; }
static void remove_comment(char *s){ char *p=strchr(s,'%'); if(p)*p='\0'; }
static int parse_reg(const char *s,int *r){ char extra; return (sscanf(s," x%d %c",r,&extra)==1 || sscanf(s,"x%d%c",r,&extra)==1) && *r>=0&&*r<=255; }
static int parse_vector(const char *s,int *r){ char extra; return (sscanf(s," v%d %c",r,&extra)==1 || sscanf(s,"v%d%c",r,&extra)==1) && *r>=0&&*r<=31; }
static int parse_operand(const char *s,int *is_const,int *value){ char b[64],extra; int r; strncpy(b,s,63);b[63]='\0';char*p=trim(b); if(*p=='x'&&sscanf(p,"x%d %c",&r,&extra)==1&&r>=0&&r<=255){*is_const=0;*value=r;return 1;} if(sscanf(p,"%d %c",value,&extra)==1&&*value>=0&&*value<=255){*is_const=1;return 1;}return 0; }
static int add_label(const char*n,int idx){int i;for(i=0;i<label_count;i++)if(!strcmp(labels[i].name,n))return 0;if(label_count>=MAX_LABELS)return 0;strncpy(labels[label_count].name,n,63);labels[label_count].name[63]='\0';labels[label_count].instruction_index=idx;label_count++;return 1;}
static int find_label(const char*n){int i;for(i=0;i<label_count;i++)if(!strcmp(labels[i].name,n))return labels[i].instruction_index;return -1;}
static void emit(FILE*out,int op,int d,int s1,int s2){fprintf(out,"%X %X %X %X\n",op,d,s1,s2);printf("%X %X %X %X\n",op,d,s1,s2);}
static int branch_code(const char*m){const char*n[]={"BEQ","BNE","BCS","BCC","BMI","BPL","BVS","BVC","BHI","BLS","BGE","BLT","BGT","BLE","BAL"};int i;for(i=0;i<15;i++)if(!strcmp(m,n[i]))return i;return -1;}

void compile(const char *sourceFile, const char *bytecodeFile)
{
    FILE *src = fopen(sourceFile, "r");
    FILE *out;
    char raw[MAX_LINE];
    int instruction_index = 0, i;

    label_count = 0;
    line_count = 0;

    if (!src) { printf("Unable to open %s\n", sourceFile); return; }

    while (fgets(raw, sizeof(raw), src) && line_count < MAX_LINES) {
        char line[MAX_LINE], *p;
        strncpy(line, raw, MAX_LINE - 1); line[MAX_LINE - 1] = '\0';
        remove_comment(line);
        p = trim(line);
        if (*p == '\0') continue;
        if (*p == '.') {
            char label[64];
            if (sscanf(p, "%63s", label) == 1) add_label(label, instruction_index);
            continue;
        }
        strncpy(lines[line_count].text, p, MAX_LINE - 1);
        lines[line_count].text[MAX_LINE - 1] = '\0';
        lines[line_count++].instruction_index = instruction_index++;
    }
    fclose(src);

    out = fopen(bytecodeFile, "w");
    if (!out) { printf("Unable to create %s\n", bytecodeFile); return; }

    for (i = 0; i < line_count; ++i) {
        char line[MAX_LINE], mnemonic[32], left[64], right[64], rhs[64], addr[64];
        char op;
        int d, a, b, c1, branch;

        strncpy(line, lines[i].text, MAX_LINE - 1); line[MAX_LINE - 1] = '\0';

        /* Vector memory read: vD = [xA] / vD = [constant]. */
        if (sscanf(line, "v%d = [%63[^]]]", &d, addr) == 2) {
            if (d < 0 || d > 31 || !parse_operand(addr, &c1, &a)) {
                printf("Invalid vector memory read: %s\n", line); continue;
            }
            emit(out, c1 ? 0x2C : 0x25, d, 0, a);
            continue;
        }

        /* Vector memory write: [xA] = vD / [constant] = vD. */
        if (sscanf(line, "[%63[^]]] = v%d", addr, &d) == 2) {
            if (d < 0 || d > 31 || !parse_operand(addr, &c1, &a)) {
                printf("Invalid vector memory write: %s\n", line); continue;
            }
            if (c1) emit(out, 0x2E, 0, a, d);
            else    emit(out, 0x26, a, 0, d);
            continue;
        }

        /* Vector arithmetic: vD = vA op vB, vD = vA op xB, or vD = vA op constant. */
        if (sscanf(line, "%63[^=] = %63[^ ] %c %63s", left, right, &op, rhs) == 4 &&
            parse_vector(trim(left), &d) && parse_vector(trim(right), &a)) {
            int rv, rx, constant;
            int vector_opcode = (op == '+') ? 0x21 : (op == '-') ? 0x22 : (op == '*') ? 0x23 : -1;
            int constant_opcode = (op == '+') ? 0x29 : (op == '-') ? 0x2A : (op == '*') ? 0x2B : -1;

            if (vector_opcode < 0) { printf("Invalid vector operator: %c\n", op); continue; }
            if (parse_vector(rhs, &rv)) { emit(out, vector_opcode, d, a, rv); continue; }
            if (parse_reg(rhs, &rx)) { emit(out, vector_opcode, d, a, rx); continue; }
            if (sscanf(rhs, "%d", &constant) == 1 && constant >= 0 && constant <= 255) {
                emit(out, constant_opcode, d, a, constant); continue;
            }
            printf("Invalid vector operand: %s\n", rhs);
            continue;
        }

        /* Legacy integer Read/Write. */
        if (sscanf(line, "Read x%d , %d", &d, &a) == 2 || sscanf(line, "Read x%d %d", &d, &a) == 2) {
            emit(out, 0x05, d, 0, a); continue;
        }
        if (sscanf(line, "Write x%d , %d", &d, &a) == 2 || sscanf(line, "Write x%d %d", &d, &a) == 2) {
            emit(out, 0x06, d, 0, a); continue;
        }

        /* Branch. */
        if (sscanf(line, "%31s %63s", mnemonic, rhs) == 2 && (branch = branch_code(mnemonic)) >= 0) {
            int target = find_label(trim(rhs));
            int offset;
            if (target < 0) { printf("Unknown label: %s\n", rhs); continue; }
            offset = target - lines[i].instruction_index;
            if (offset < -128 || offset > 127) { printf("Branch offset out of range: %d\n", offset); continue; }
            emit(out, 0x10 + branch, 0, 0, offset & 0xFF);
            continue;
        }

        /* Integer memory write: [xA] = xB or [constant] = xB. */
        if (sscanf(line, "[%63[^]]] = %63s", addr, rhs) == 2) {
            int address_is_const, value_reg;
            if (!parse_operand(addr, &address_is_const, &a) || !parse_reg(rhs, &value_reg)) {
                printf("Invalid memory write: %s\n", line); continue;
            }
            if (address_is_const) emit(out, 0x0E, 0, a, value_reg);
            else                  emit(out, 0x06, a, 0, value_reg);
            continue;
        }

        /* Integer memory read: xD = [xA] or xD = [constant]. */
        if (sscanf(line, "x%d = [%63[^]]]", &d, addr) == 2) {
            int address_is_const;
            if (!parse_operand(addr, &address_is_const, &a)) {
                printf("Invalid memory read: %s\n", line); continue;
            }
            emit(out, address_is_const ? 0x0D : 0x05, d, 0, a);
            continue;
        }

        /* Integer arithmetic. */
        if (sscanf(line, "%63[^=] = %63[^ ] %c %63s", left, right, &op, rhs) == 4) {
            int rhs_is_const, lhs_is_const, opcode;
            if (!parse_reg(trim(left), &d) || !parse_operand(trim(right), &lhs_is_const, &a) ||
                lhs_is_const || !parse_operand(rhs, &rhs_is_const, &b)) {
                printf("Invalid arithmetic instruction: %s\n", line); continue;
            }
            switch (op) {
                case '+': opcode = rhs_is_const ? 0x09 : 0x01; break;
                case '-': opcode = rhs_is_const ? 0x0A : 0x02; break;
                case '*': opcode = rhs_is_const ? 0x0B : 0x03; break;
                case '/': opcode = rhs_is_const ? 0x0C : 0x04; break;
                default: printf("Invalid operator: %c\n", op); continue;
            }
            emit(out, opcode, d, a, b);
            continue;
        }

        /* Integer data movement: xD = constant. */
        if (sscanf(line, "x%d = %d", &d, &a) == 2 && d >= 0 && d <= 255 && a >= 0 && a <= 255) {
            emit(out, 0x0F, d, 0, a);
            continue;
        }

        printf("Unable to compile line: %s\n", line);
    }

    fclose(out);
}
