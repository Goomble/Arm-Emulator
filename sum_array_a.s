.global sum_array_a
.func sum_array_a

sum_array_a:
    push {r4, r5, r6}   /* preserve r4 on the stack (r5 is pushed to increment stack by 8) */
    mov r2, #0 /* counter: i */
    mov r3, #0 /* sum */
for_loop:
    cmp r2, r1 /* comparing counter to length: Is i=len? */
    beq done 
    ldr r4, [r0, r2, lsl #2]  /* load array element a[i]  */
    add r3, r3, r4  /* sum = sum + a[i] */
    add r2, r2, #1  /*  i = i + 1 */
    b for_loop
done:
    mov r0, r3
    pop {r4, r5, r6}   /* restore r4 and r5 from the stack */
    bx lr
