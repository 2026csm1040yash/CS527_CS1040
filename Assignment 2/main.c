#include <stdio.h>
#include "compiler.h"
#include "memory.h"
#include "processor.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <program_file> <data_file>\n", argv[0]);
        return 1;
    }

    compile(argv[1], "program.byte");
    initialize("program.byte", argv[2]);
    reset();

    while (!end_of_simulation) {
        fetch();
        decode();
        execute();
    }

    printf("Writing updated data to file...\n");
    finalize(argv[2]);
    printf("Simulation completed successfully.\n");

    return 0;
}
/*ERROR  WHEN WE DIVIDE BY ZERO IN EXECUTE FUNCTION, IT SHOULD BE HANDLED,BUT IT IS NOT.
 To handle division by zero, we can modify the execute function in processor.c to check 
for a zero divisor before performing the division operation. If a division by zero is 
detected, we can print an error message and set end_of_simulation to 1 to terminate the
 simulation gracefully.*/

 /*THERE IS ANOTHER ERROR ie. THERE IS NO REGISTER REGISTER DATA MOVEMENT IN EXECUTE FUNCTION, 
 IT SHOULD BE PRESENT ,BUT IT IS NOT. I HAVE TO ADD IT IN EXECUTE FUNCTION. 
 I WILL ADD IT IN EXECUTE FUNCTION.*/
 
