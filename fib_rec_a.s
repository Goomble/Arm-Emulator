
.global fib_rec_a
.func fib_rec_a

fib_rec_a:
    push {r4, r5, r6, lr} /* preserve r4, r5 and lr on the stack (r6 is pushed to increment the stack by 16) */
    mov r4, r0 /* save n */
    cmp r0, #0 /* base case of n = 0 */
    beq fib_ret 
    cmp r0, #1 /* base case of n = 1 */
    beq fib_ret
else:
    sub r0, r4, #1 /* get (n-1) */
    bl fib_rec_a /* run fib_rec_a(n-1) */
    mov r5, r0 /* save the result of fib_rec_a(n-1) in r5 */
    sub r0, r4, #2 /* get (n-2) */
    bl fib_rec_a /* run fib_rec_a(n-2) */
    add r0, r5, r0 /* fib_rec_a(n-1) + fib_rec_a(n-2) */
fib_ret:
    pop {r4, r5, r6, lr} /* restore r4, r5, r6 and lr */
    bx lr 
