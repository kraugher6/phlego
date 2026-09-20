.global _start
.text
_start:
    # --- 1. LUI & AUIPC ---
    lui     x1, 0x12345
    auipc   x2, 0x1000

    # --- 2. Immediate Arithmetic ---
    addi    x3, x0, 10
    addi    x4, x3, 20
    slti    x5, x4, 40
    sltiu   x6, x4, 5
    xori    x7, x4, 0x0F
    ori     x8, x7, 0xF0
    andi    x9, x8, 0xFF
    slli    x10, x3, 2
    srli    x11, x10, 1
    srai    x12, x11, 1

    # --- 3. Register Arithmetic (R-type) ---
    add     x13, x3, x4
    sub     x14, x13, x3
    slt     x15, x3, x4
    sltu    x16, x4, x3
    xor     x17, x3, x4
    or      x18, x3, x4
    and     x19, x3, x4

    # --- 4. RV32M Extensions ---
    mul     x20, x3, x4
    mulh    x21, x3, x4
    div     x22, x4, x3
    rem     x23, x4, x3

    # --- 5. Memory Operations ---
    la      x31, data_area
    sw      x4, 0(x31)
    lw      x24, 0(x31)
    sh      x4, 4(x31)
    lh      x25, 4(x31)
    sb      x4, 6(x31)
    lb      x26, 6(x31)

    # --- 6. Control Flow & Branches ---
    beq     x3, x3, target1
    addi    x27, x27, 99
target1:
    bne     x3, x4, target2
    addi    x27, x27, 99
target2:
    blt     x3, x4, target3
    addi    x27, x27, 99
target3:
    bge     x4, x3, target4
    addi    x27, x27, 99
target4:

    # JAL & JALR
    jal     x1, target_jump
    addi    x27, x27, 99
target_jump:
    jalr    x0, x1, 4

    addi    x27, x27, 99

exit_label:
    ecall

.data
data_area:
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
