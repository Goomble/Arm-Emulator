.global fib_iter_a
.func fib_iter_a

fib_iter_a:
    push {r4, r5}  /* preserve r4 on the stack (r5 is pushed to increment stack by 8) */
    mov r1, #0 /* a = 0 */
    mov r2, #1 /* b = 1 */
    mov r3, #2 /* i = 2 */
if_statement:
    cmp r0, #0 /* compare n to 0 */
    moveq r0, r1
    beq done
for_loop:
    cmp r3, r0 /* comparing i to n */
    movgt r0, r2 /* store b as the return value */
    bgt done 
    add r4, r1, r2 /* c = a + b */
    mov r1, r2 /* a = b */
    mov r2, r4 /* b = c */
    add r3, r3, #1 /* i = i + 1 */
    b for_loop
done:
    pop {r4, r5}  /* restore r3 and r5 from the stack */
    bx lr
