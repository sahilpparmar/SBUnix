#
# init_desc_table.s -- loads GDTR and IDTR
#

.text

# Load a new GDT
#  parameter 1: address of gdtr (%rdi)
#  parameter 2: new code descriptor offset (%rsi)
#  parameter 3: new data descriptor offset (%rdx)
.global load_gdt
load_gdt:
    lgdt (%rdi)
    pushq %rsi                  # push code selector
    movabsq $.done, %r10
    pushq %r10                  # push return address
    lretq                       # far-return to new cs descriptor ( the retq below )
.done:
    movq %rdx, %es
    movq %rdx, %fs
    movq %rdx, %gs
    movq %rdx, %ds
    movq %rdx, %ss
    retq


# Load a new IDT
#  parameter 1: address of idtr (%rdi)
.global load_idtr
load_idtr:
    lidt (%rdi)
    retq

