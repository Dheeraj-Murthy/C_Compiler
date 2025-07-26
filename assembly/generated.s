.section __TEXT,__text
.global _main
.align 2

_main:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov x9, #0
    str x9, [sp, #-16]!
    mov x9, #0
    str x9, [sp, #-16]!
    mov x9, #0
    str x9, [sp, #-16]!
    mov x9, #1
    str x9, [sp, #-16]!
Lloop0:
    mov x0, #100
    str x0, [sp, #-16]!
    mov x1, #100
    str x1, [sp, #-16]!
    ldr x1, [sp], #16
    ldr x0, [sp], #16
    cmp x0, x1
    b.eq Llabel0
    ldr x0, [sp, #32]
    str x0, [sp, #-16]!
    ldr x0, [sp], #16
    str x0, [sp, #32]
    ldr x0, [sp, #32]
    str x0, [sp, #-16]!
    ldr x0, [sp, #32]
    str x0, [sp, #-16]!
    ldr x0, [sp], #16
    str x0, [sp, #-16]!
    ldr x0, [sp], #16
    ldr x0, [sp, #48]
    str x0, [sp, #-16]!
    ldr x1, [sp], #16
    add x0, x0, x1
    str x0, [sp, #-16]!
    ldr x0, [sp], #16
    str x0, [sp, #0]
    ldr x0, [sp, #0]
    str x0, [sp, #-16]!
    b Lloop0
Llabel0:
    mov x0, #0
    str x0, [sp, #-16]!
    mov x16, #1
    ldr x0, [sp], #16
    svc #0x80
    mov x0, #0
    ldp x29, x30, [sp], #16
    ret
