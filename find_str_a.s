.global find_str_a
.func find_str_a

find_str_a:
    push {r4, r5, r6, r7, r8, r9} /* preserve r4,r5,r6,r7,r8 and r9 on the stack */
    mov r2, #0 /* counter i */
    mov r3, #0 /* string length (s_length) */
    mov r4, #0 /* substring length (sub_length) */
    mov r9, #-1 /*return value */
    ldrb r5, [r0, r3] /* load the first bit of the main string (s) */
    ldrb r6, [r1, r4] /* load the first bit of the substring (sub) */
string_length: /* get the length of the main string (s) */
    cmp r5, #0 /* while *s != '\0'*/
    beq substring_length 
    add r3, r3, #1 /* increment main string's length (s_length++) */
    ldrb r5, [r0, r3] /* load the next element of the main string (s++) */
    b string_length 
substring_length: /* get the length of the sub string (sub) */
    cmp r6, #0 /* while *sub != '\0' */
    beq find_substring
    add r4, r4, #1 /* increment sub string's length (sub_length++) */
    ldrb r6, [r1, r4] /* load the next element of the sub string */
    b substring_length
find_substring:
    cmp r2, r3 /* compare i to s_length */
    beq done
    mov r7, #0 /* j = 0 */
    mov r8, #0 /* count = 0 */
    while_loop:
        ldrb r5, [r0, r2] /* load s[i] */
        ldrb r6, [r1, r7] /* load sub[j] */
        cmp r5, r6 /* check if s[i] == sub[j] */
        bne restore_i
        cmp r5, #0 /* check if s[i] != '\0' */
        beq restore_i
        cmp r6, #0 /* check if sub[i] != '\0' */
        beq restore_i
        add r8, r8, #1 /* increment count (count++) */
        add r2, r2, #1 /* increment i (i++) */
        add r7, r7, #1 /* increment j (j++) */
        b while_loop
    restore_i:
        sub r2, r2, r8 /* restore i (i-=count) */
    if_statement:
        cmp r8, r4 /* check if count == sub_length */
        moveq r9, r2 /* if count == sub_length, return i */
        beq done
    increment_counter:
    add r2, r2, #1 /* increment i (i++) */
    b find_substring
done:
    mov r0, r9 /* store the return value in r0 */
    pop {r4, r5, r6, r7, r8, r9}   /* restore r4,r5,r6,r7,r8,r9 from the stack */
    bx lr
