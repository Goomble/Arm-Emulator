.global find_max_a
.func find_max_a

find_max_a:
    push {r4, r5}  /* preserve r4 on the stack (r5 is pushed to increment stack by 8) */
    mov r2, #0 /* get the first element */
    ldr r3, [r0, r2, lsl #2] /* max = a[0] */
    mov r2, #1 /* counter: i */
for_loop:
    cmp r2, r1 /* comparing counter to length: Is i=len? */
    beq done 
    ldr r4, [r0, r2, lsl #2] /* load array element a[i] */
    cmp r4, r3 /* compare a[i] to max */
    movgt r3, r4 /* change max, if a[i] is greater */
    add r2, r2, #1 /* i = i + 1 */
    b for_loop
done:
    mov r0, r3
    pop {r4, r5}  /* restore r4 and r5 from the stack */
    bx lr
