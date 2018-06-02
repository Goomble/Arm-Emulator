#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int sum_array_a(int *x, int len);
int find_max_a(int *x, int len);
int fib_iter_a(int n);
int fib_rec_a(int n);
int find_str_a(char *s, char *sub);


#define NREGS 16
#define SP 13
#define LR 14 
#define PC 15 
#define STACK_SIZE 1024

struct arm_state {
    unsigned int regs[NREGS];
    unsigned int cpsr;
    unsigned char *stack;
    unsigned int computational_count;
    unsigned int memory_count;
    unsigned int branch_count;
};

struct arm_state *arm_state_new(unsigned int *func,
                                unsigned int arg0, unsigned int arg1,
                                unsigned int arg2, unsigned int arg3)
{
    struct arm_state *as;
    int i;

    as = (struct arm_state *) malloc(sizeof(struct arm_state));
    if (as == NULL) {
        printf("malloc() failed, exiting.\n");
        exit(-1);
    }

    as->stack = (unsigned char *) malloc(STACK_SIZE);

    if (as->stack == NULL) {
        printf("malloc() failed, exiting.\n");
        exit(-1);
    }
    

    /* Initialize all registers to zero. */
    as->cpsr = 0;
    for (i = 0; i < NREGS; i++) {
        as->regs[i] = 0;
    }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) as->stack + STACK_SIZE;
    
    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
    as->regs[3] = arg3;

    // Inititalize all the counts to 0
    as->computational_count = 0;
    as->memory_count = 0;
    as->branch_count = 0;

    return as;
}

void arm_state_free(struct arm_state *as)
{
    free(as->stack);
    free(as);
}

void arm_state_print(struct arm_state *as, unsigned int emu_result, unsigned int assembler_result)
{
    int i;
    
    printf("stack size = %d\n", STACK_SIZE);
    printf("Register values after execution:\n");
    for (i = 0; i < NREGS; i++) {
        printf("r%d = (%X) %d\n", i, as->regs[i], (int) as->regs[i]);
    }
    printf("cpsr: 0x%x\n", as->cpsr);
    printf("Total Instructions Executed: %d\n", (as->computational_count+as->memory_count+as->branch_count));
    printf("Total Computational Instructions Executed: %d\n", as->computational_count);
    printf("Total Memory Instructions Executed: %d\n", as->memory_count);
    printf("Total Branch Instructions Executed: %d\n", as->branch_count);
    printf("ARM Emulator Result: %d\n", emu_result);
    printf("Assembler Result: %d\n", assembler_result);

}

void set_cpsr_flags(struct arm_state *as, int result, long long result_long) 
{
    //setting v flag
    if (result > 2147483647 || result < -2147483648) {
        as->cpsr = as->cpsr | 0x10000000; //setting v to 1
        // printf("v here\n");

    }
    else {
        as->cpsr = as->cpsr & 0xEFFFFFFF; //setting v to 0
        // printf("c here\n");
    }

    //setting c flag
    if (result_long < 0 && result_long > 4294967295) {
        as->cpsr = as->cpsr | 0x20000000; //setting C to 1 
    }
    else {
        as->cpsr = as->cpsr & 0xDFFFFFFF; //setting C to 0
    }

    //setting z flag and n flag
    if (result < 0) {
        as->cpsr = as->cpsr | 0x80000000; //setting N to 1
        as->cpsr = as->cpsr & 0xBFFFFFFF; //setting Z to 0
        // printf("less\n");
    }
    else if (result == 0) {
        as->cpsr = as->cpsr | 0x40000000; //setting Z to 1
        as->cpsr = as->cpsr & 0x7FFFFFFF; //setting N to 0
    }
    else {
        as->cpsr = as->cpsr & 0xBFFFFFFF; //setting Z to 0
        as->cpsr = as->cpsr & 0x7FFFFFFF; //setting N to 0
    }
}

int check_cpsr_flags(struct arm_state *as, unsigned int iw) 
{
    unsigned int condition = (iw >> 28) & 0b1111;

    //check if the operation can be executed
    switch (condition) {
        case 0: //EQ
            return ((as->cpsr >> 30) & 0b1) == 1;
        case 1: //NE
            return ((as->cpsr >> 30) & 0b1) == 0;
        case 11: //LT
            return ((as->cpsr >> 28) & 0b1) != ((as->cpsr >> 31) & 0b1);
        case 12: //GT
            return (((as->cpsr >> 30) & 0b1) == 0) && (((as->cpsr >> 31) & 0b1) == ((as->cpsr >> 28) & 0b1));
        case 14: //AL
            return true;
    }
}

bool iw_is_data_processing_instruction(unsigned int iw)
{
    return ((iw >> 26) & 0b11) == 0;
}

void execute_data_processing_instruction(struct arm_state *as, unsigned int iw)
{    
    unsigned int rm_value;
    unsigned int i_bit = (iw>>25) & 0b1;
    unsigned int opcode = (iw >> 21) & 0xF;
    unsigned int s_bit = (iw >> 20) & 0b1;
    unsigned int rd = (iw >> 12) & 0xF;
    unsigned int rn = (iw >> 16) & 0xF;
    int result;
    long long result_long;

    if (i_bit == 1) {
        rm_value = iw & 0xFF;
    }
    else {
        rm_value = as->regs[(iw & 0xF)];
    }

    switch(opcode) {
        case 2: //sub
            as->regs[rd] = as->regs[rn] - rm_value;
            result_long = (long long) as->regs[rn] - (long long) rm_value;
            result = as->regs[rd];
            break;
        case 4: //add
            as->regs[rd] = as->regs[rn] + rm_value;
            result_long = (long long) as->regs[rn] + (long long) rm_value;
            result = as->regs[rd];
            break;
        case 10: //cmp
            result = as->regs[rn] - rm_value;
            // printf("cmp %d to %d and the result is: %d\n", as->regs[rn], rm_value, result);
            result_long = (long long) as->regs[rn] - (long long) rm_value;
            break;
        case 11: //cmn
            result = as->regs[rn] + rm_value;
            result_long = (long long) as->regs[rn] + (long long) rm_value;  
            break;
        case 13: //mov
            as->regs[rd] = rm_value;
            result = as->regs[rd];
            result_long = (long long) as->regs[rd];
            break;
        case 15: //mvn
            as->regs[rd] = ~(rm_value);
            result = ~(rm_value);
            result_long = (long long) as->regs[rd];
            break;
    }

    if (s_bit == 1) {
        set_cpsr_flags(as, result, result_long);
    }
}

bool iw_is_bx_instruction(unsigned int iw)
{
    return ((iw >> 4) & 0xFFFFFF) == 0b000100101111111111110001;
}

void execute_bx_instruction(struct arm_state *as, unsigned int iw)
{
    unsigned int rn = iw & 0b1111;

    as->regs[PC] = as->regs[rn];
}

bool iw_is_branch_instruction(unsigned int iw) 
{
    return ((iw >> 25) & 0b111) == 0b101;
}

void execute_branch_instruction(struct arm_state *as, unsigned int iw) {

    unsigned int signed_bit = (iw >> 23) & 0b1;
    unsigned int l_bit = (iw >> 24) & 0b1;
    unsigned int offset = iw & 0xFFFFFF;
    unsigned int destination;

    //Check signed bit
    if (signed_bit == 0) {
        destination = offset | 0x00000000;
    }
    else {
        destination = offset | 0xFF000000;
    }

    destination = destination << 2;

    if (l_bit == 1) {  
        as->regs[LR] = as->regs[PC] + 4;
    }

    as->regs[PC] += (destination + 8);

}

bool iw_is_single_data_transfer_instruction(unsigned int iw)
{
    return ((iw >> 26) & 0b11) == 0b01;
}

void execute_single_data_transfer_instruction(struct arm_state *as, unsigned int iw)
{
    unsigned int rd = (iw>>12) & 0xF;
    unsigned int rn = (iw>>16) & 0xF;
    unsigned int l_bit = (iw>>20) & 0b1;
    unsigned int w_bit = (iw>>21) & 0b1;
    unsigned int b_bit = (iw>>22) & 0b1;
    unsigned int u_bit = (iw>>23) & 0b1;
    unsigned int p_bit = (iw>>24) & 0b1;
    unsigned int i_bit = (iw>>25) & 0b1;
    unsigned int modified_base_value = as->regs[rn];
    unsigned int offset_value;

    //Check i bit
    if (i_bit == 1) {
        offset_value = as->regs[(iw & 0xF)] << ((iw>>7) & 0b11111); 
    }
    else {
        offset_value = iw & 0xFFF;
    }

    //Check p bit
    if (p_bit == 1) {
        //Check u bit
        if (u_bit == 1) {
            modified_base_value += offset_value; 
        }
        else {
            modified_base_value -= offset_value;
        }
    }

    //Check b bit
    if (b_bit == 1) {
        // Check l bit
        if (l_bit == 1) {
            as->regs[rd] = (unsigned int)*((char *)modified_base_value); //ldrb 
        }
    }
    else {
        //Check l bit
        if (l_bit == 1) {
            as->regs[rd] = *((unsigned int *) modified_base_value); //ldr
        }
        else {
            *((unsigned int *) modified_base_value) = as->regs[rd]; //str
        }
    }

    //Check p bit
    if (p_bit == 0) {
        //Check u bit
        if (u_bit == 1) {
            modified_base_value += offset_value;
        }
        else {
            modified_base_value += offset_value;
        }
    }

    //Check w bit
    if (w_bit == 1) {
        as->regs[rn] = modified_base_value;
    }

    as->regs[PC] += 4;
}

bool iw_is_push(unsigned int iw)
{
    return (((iw >> 25) & 0b111) == 0b100) && (((iw>>20) & 0b1) == 0b0);
}

bool iw_is_pop(unsigned int iw) 
{
    return (((iw >> 25) & 0b111) == 0b100) && (((iw>>20) & 0b1) == 0b1);
}

void execute_push(struct arm_state *as, unsigned int iw) 
{
    unsigned int register_list = iw & 0xFFFF;
    unsigned int rn = (iw>>16) & 0xF;
    unsigned int w_bit = (iw>>21) & 0b1;
    unsigned int s_bit = (iw>>22) & 0b1;
    unsigned int u_bit = (iw>>23) & 0b1;
    unsigned int p_bit = (iw>>24) & 0b1;
    unsigned int modified_base_value = as->regs[rn];
    unsigned int offset_value = 4;
    int i;

    for (i = (NREGS - 1); i >= 0; i--) {
        if ( ((register_list >> i) & 0b1) == 0b1) {
            //Check p bit
            if (p_bit == 1) {
                //Check u bit
                if (u_bit == 1) {
                    modified_base_value += offset_value;
                }
                else {
                    modified_base_value -= offset_value;
                }
            }
            *((unsigned int *) modified_base_value) = as->regs[i];
            //Check p bit
            if (p_bit == 0) {
                //Check u bit
                if (u_bit == 1) {
                    modified_base_value += offset_value;
                }
                else {
                    modified_base_value -= offset_value;
                }
            }
            //Check w bit
            if (w_bit == 1) {
                as->regs[rn] = modified_base_value;
            }
        }
    }

    as->regs[PC] += 4;
}

void execute_pop(struct arm_state *as, unsigned int iw) 
{
    unsigned int register_list = iw & 0xFFFF;
    unsigned int rn = (iw>>16) & 0xF;
    unsigned int w_bit = (iw>>21) & 0b1;
    unsigned int s_bit = (iw>>22) & 0b1;
    unsigned int u_bit = (iw>>23) & 0b1;
    unsigned int p_bit = (iw>>24) & 0b1;
    unsigned int modified_base_value = as->regs[rn];
    unsigned int offset_value = 4;
    int i;

    for (i = 0; i < NREGS; i++) {
        if ( ((register_list >> i) & 0b1) == 0b1) {
            //Check p bit
            if (p_bit == 1) {
                //Check u bit
                if (u_bit == 1) {
                    modified_base_value += offset_value;
                }
                else {
                    modified_base_value -= offset_value;
                }
            }
            as->regs[i] = *((unsigned int *) (modified_base_value));
            //Check p bit
            if (p_bit == 0) {
                //Check u bit
                if (u_bit == 1) {
                    modified_base_value += offset_value;
                }
                else {
                    modified_base_value -= offset_value;
                }
            }
            //Check w bit
            if (w_bit == 1) {
                as->regs[rn] = modified_base_value;
            }
        }
    }

    as->regs[PC] += 4;
}


void arm_state_execute_one(struct arm_state *as)
{
    unsigned int iw;
    unsigned int *pc;

    pc = (unsigned int *) as->regs[PC];
    iw = *pc;

    // printf("r3: %d\n", as->regs[3]);
        
    if (iw_is_bx_instruction(iw)) {
        as->branch_count++;
        if (check_cpsr_flags(as, iw)) {
            execute_bx_instruction(as, iw);
        }
    } else if (iw_is_branch_instruction(iw)) {
        as->branch_count++;
        if (check_cpsr_flags(as, iw)) {
            execute_branch_instruction(as, iw);
        }
        else {
            as->regs[PC] += 4;
        }
    } else if (iw_is_data_processing_instruction(iw)) {
        as->computational_count++;
        if (check_cpsr_flags(as, iw)) {
            execute_data_processing_instruction(as, iw);
        }
        as->regs[PC] += 4;
    } else if (iw_is_single_data_transfer_instruction(iw)) {
        as->memory_count++;
        if (check_cpsr_flags(as, iw)) {
            execute_single_data_transfer_instruction(as, iw);
        }
    } else if (iw_is_push(iw)) {
        as->memory_count++;
        if (check_cpsr_flags(as, iw)) {
            execute_push(as, iw);
        }
    } else if (iw_is_pop(iw)) {
        as->memory_count++;
        if (check_cpsr_flags(as, iw)) {
            execute_pop(as, iw);
        }
    }
}

unsigned int arm_state_execute(struct arm_state *as)
{
    while (as->regs[PC] != 0) {
        arm_state_execute_one(as);
    }

    return as->regs[0];
}

void execute_sum_array_a() 
{
    struct arm_state *as;
    unsigned int assembler_result, emu_result;
    int i;
    int positive[1000], negative[1000], zero_values[1000], positive_or_negative[1000];

    /* 
    positive[1000] -- array of positive numbers from 0 to 999 in an ascending order.
    negative [1000] -- array of negative numbers from 0 to 999 in a descending order
    zeroValues[1000] -- aray of 1000 zeros
    positive_or_negative[1000] -- array of 1000 numbers, where every even number is positive,
    every odd number is negative.
    */

    for (i = 0; i < 1000; i++) {
        positive[i] = i;
        negative[i] = -i;
        zero_values[i] = 0;
        if (i % 2 == 0) {
            positive_or_negative[i] = i;
        }
        else {
            positive_or_negative[i] = -i;
        }
    }

    as = arm_state_new((unsigned int *) sum_array_a, (unsigned int) positive, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = sum_array_a(positive, 1000);

    printf("-----------------executing sum_array_a --------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);    

    as = arm_state_new((unsigned int *) sum_array_a, (unsigned int) negative, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = sum_array_a(negative, 1000);

    printf("-----------------executing sum_array_a --------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);    

    as = arm_state_new((unsigned int *) sum_array_a, (unsigned int) zero_values, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = sum_array_a(zero_values, 1000);

    printf("-----------------executing sum_array_a --------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);    

    as = arm_state_new((unsigned int *) sum_array_a, (unsigned int) positive_or_negative, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = sum_array_a(positive_or_negative, 1000);

    printf("-----------------executing sum_array_a --------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);    
} 

void execute_find_max_a() 
{
    struct arm_state *as;
    unsigned int assembler_result, emu_result;
    int i;
    int positive[1000], negative[1000], zero_values[1000], positive_or_negative[1000];

    /* 
    positive[1000] -- array of positive numbers from 0 to 999 in an ascending order.
    negative [1000] -- array of negative numbers from 0 to 999 in a descending order
    zeroValues[1000] -- aray of 1000 zeros
    positive_or_negative[1000] -- array of 1000 numbers, where every even number is positive,
    every odd number is negative.
    */

    for (i = 0; i < 1000; i++) {
        positive[i] = i;
        negative[i] = -i;
        zero_values[i] = 0;
        if (i % 2 == 0) {
            positive_or_negative[i] = i;
        }
        else {
            positive_or_negative[i] = -i;
        }
    }

    //positive values
    as = arm_state_new((unsigned int *) find_max_a, (unsigned int) positive, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = find_max_a(positive, 1000);

    printf("-----------------executing find_max_a(positive, 1000)--------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);   

    //negative values
    as = arm_state_new((unsigned int *) find_max_a, (unsigned int) negative, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = find_max_a(negative, 1000);

    printf("-----------------executing find_max_a(negative, 1000)--------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);    

    // zero values
    as = arm_state_new((unsigned int *) find_max_a, (unsigned int) zero_values, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = find_max_a(zero_values, 1000);

    printf("-----------------executing find_max_a(zero_values, 1000)--------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);   

    // positive or negative values
    as = arm_state_new((unsigned int *) find_max_a, (unsigned int) positive_or_negative, 1000, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = find_max_a(positive_or_negative, 1000);

    printf("-----------------executing find_max_a(positive_or_negative, 1000)--------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);   
}

void execute_fib_iter_a() 
{
    struct arm_state *as;
    unsigned int assembler_result, emu_result;
    unsigned int n = 20;

    as = arm_state_new((unsigned int *) fib_iter_a, n, 0, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = fib_iter_a(n);

    printf("-----------------executing fib_iter_a(20)--------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as);    
}

void execute_fib_rec_a() 
{
    struct arm_state *as;
    unsigned int assembler_result, emu_result;
    unsigned int n = 20;

    as = arm_state_new((unsigned int *) fib_rec_a, n, 0, 0, 0);
    emu_result = arm_state_execute(as);
    assembler_result = fib_rec_a(n);

    printf("-----------------executing fib_rec_a(20)--------------------\n");
    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 
}        

void execute_find_str_a() 
{
    struct arm_state *as;
    unsigned int assembler_result, emu_result;

    as = arm_state_new((unsigned int *) find_str_a, (unsigned int) "recursion", (unsigned int) "rec", 0, 0);
    emu_result = arm_state_execute(as);
    printf("-----------------executing fib_find_str_a(recursion, rec)--------------------\n");
    assembler_result = find_str_a("recursion", "rec");

    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 

    as = arm_state_new((unsigned int *) find_str_a, (unsigned int) "recursion", (unsigned int) "on", 0, 0);
    emu_result = arm_state_execute(as);
    printf("-----------------executing fib_find_str_a(recursion, on)--------------------\n");
    assembler_result = find_str_a("recursion", "on");

    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 

    as = arm_state_new((unsigned int *) find_str_a, (unsigned int) "recursion", (unsigned int) "cur", 0, 0);
    emu_result = arm_state_execute(as);
    printf("-----------------executing fib_find_str_a(recursion, cur)--------------------\n");
    assembler_result = find_str_a("recursion", "cur");

    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 

    as = arm_state_new((unsigned int *) find_str_a, (unsigned int) "recursion", (unsigned int) "ure", 0, 0);
    emu_result = arm_state_execute(as);
    printf("-----------------executing fib_find_str_a(recursion, ure)--------------------\n");
    assembler_result = find_str_a("recursion", "ure");

    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 

    as = arm_state_new((unsigned int *) find_str_a, (unsigned int) "recursion", (unsigned int) "onr", 0, 0);
    emu_result = arm_state_execute(as);
    printf("-----------------executing fib_find_str_a(recursion, onr)--------------------\n");
    assembler_result = find_str_a("recursion", "onr");

    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 

    as = arm_state_new((unsigned int *) find_str_a, (unsigned int) "ddoser", (unsigned int) "dos", 0, 0);
    emu_result = arm_state_execute(as);
    printf("-----------------executing fib_find_str_a(ddoser,dos)--------------------\n");
    assembler_result = find_str_a("ddoser", "dos");

    arm_state_print(as, emu_result, assembler_result);
    arm_state_free(as); 



}
     
int main(int argc, char **argv)
{
    execute_sum_array_a();
    execute_find_max_a();
    execute_fib_iter_a();
    execute_fib_rec_a();
    execute_find_str_a();

    return 0;
}
