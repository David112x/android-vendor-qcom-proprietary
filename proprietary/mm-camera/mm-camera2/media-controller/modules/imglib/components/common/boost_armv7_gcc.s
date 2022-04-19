@==========================================================================
@    boost_armv7_gcc.S
@
@
@
@ Copyright (c) 2016 QUALCOMM Technologies Inc.
@ All Rights Reserved.
@ Confidential and Proprietary - Qualcomm Technologies, Inc.
@==========================================================================

@==========================================================================
@                          Edit History
@
@
@ when       who       what where why
@ --------   ------    ----------------------------------------------------
@ 12/22/12   mingy     Created file.
@==========================================================================

@==========================================================================
@                         FUNCTION LIST
@--------------------------------------------------------------------------
@
@ - boost_line_asm
@
@==========================================================================

    .section pdaf_gm_with_proj_armv7_gcc, "x"   @ AREA
    .text                                       @ |.text|, CODE, READONLY
    .align 2
    .code  32                                   @ CODE32

@==============================================================================
@   ARM Registers
@==============================================================================
p_src    .req r0
length   .req r1
K        .req r2
p_dst    .req r3

@==========================================================================
@   Main Routine
@==========================================================================

    .global boost_line_asm

@ Clamping constants 0x0 and 0xFF
.equ COEFF_0,      0
.equ COEFF_255,    255

@==========================================================================
@ FUNCTION     : boost_line_asm
@--------------------------------------------------------------------------
@ DESCRIPTION  : Apply the coefficient onto the input pixels and clamp to 255.
@--------------------------------------------------------------------------
@ C PROTOTYPE  : void boost_line_asm(uint8_t  *p_src,
@                                    int32_t   length,
@                                    uint8_t   K,
@                                    uint8_t  *p_dst)
@--------------------------------------------------------------------------
@ REG INPUT    : R0: uint8_t  *p_src
@                      pointer to the input line
@                R1: int32_t   length
@                      length of the input
@                R2: uint8_t  K
@                      the coefficient to be applied on the input pixels
@                R3: uint8_t  *p_dst
@                      pointer to the output line
@--------------------------------------------------------------------------
@ STACK ARG    : None
@--------------------------------------------------------------------------
@ REG OUTPUT   : None
@--------------------------------------------------------------------------
@ MEM INPUT    : p_src  - a line of luma pixels
@                length - length of the input pixels
@                K      - the coefficient
@--------------------------------------------------------------------------
@ MEM OUTPUT   : p_dst - the processed pixels
@--------------------------------------------------------------------------
@ REG AFFECTED : ARM:  R0-R3
@                NEON: Q0-Q15
@--------------------------------------------------------------------------
@ STACK USAGE  : none
@--------------------------------------------------------------------------
@ CYCLES       : none
@
@--------------------------------------------------------------------------
@ NOTES        :
@==========================================================================

.type boost_line_asm, %function
boost_line_asm:

    @==========================================================================
    @ Store stack registers
    @==========================================================================
    STMFD SP!, {R4, LR}

    VPUSH {D8-D15}

    VDUP.8    D0, R2                   @ load K into D0

    @==========================================================================
    @ The main loop
    @==========================================================================
loop_boost_line:

    VLD1.8   {D2}, [p_src]!           @ Load 8 pixels (uint8_t) to D2

	VMULL.U8    Q2, D2, D0            @ multiply input pixel by K

	VQMOVN.U16  D6, Q2                @ convert uint16_t to uint8_t and saturate (clamp to 0xFF)

	VST1.8      {D6}, [p_dst]!        @ store processed pixels

    SUBS length, length, #8           @ check if the length is less than 8

    BGT loop_boost_line               @ loop if more than 8 pixels left

end_boost_line:

    VPOP {D8-D15}

    LDMFD SP!, {R4, PC}

    .end
