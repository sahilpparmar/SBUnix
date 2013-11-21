#
# irq_common.s -- IRQ common handler
# 

.macro PUSHA
    pushq %rdi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %r8
    pushq %r9
.endm

.macro POPA
    popq %r9
    popq %r8
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rdi
.endm

.text

.extern irq_handler
.extern timer_handler

.global irq0
.global irq1
.global irq_common

irq_common:
    PUSHA
    movq %rsp, %rdi
    callq irq_handler
    POPA
    add $0x10, %rsp
    sti
    iretq

irq0:
    cli
    PUSHA
    movq %rsp, %rdi
    callq timer_handler
    POPA
    sti
    iretq
 
irq1:
    cli
    pushq $0x0
    pushq $0x21
    jmp irq_common

