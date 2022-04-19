@==========================================================================
@    ds_2by2_asm.s
@
@
@
@ Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
@ Qualcomm Technologies Proprietary and Confidential.
@
@==========================================================================

@==========================================================================
@                          Edit History
@
@
@ when       who         what where why
@ --------   -------     --------------------------------------------------
@ 07/10/14   mingy       Created file.
@
@==========================================================================

@==========================================================================
@                         FUNCTION LIST
@--------------------------------------------------------------------------
@ - ds_2by2_asm
@
@==========================================================================


   @ .section ds_2by2_asm, "x"  @ AREA
    .text                            @ |.text|, CODE, READONLY
    .align 2
    .code 32                         @ CODE32






@==========================================================================
@   Main Routine
@==========================================================================

@==========================================================================
@ FUNCTION     : ds_2by2_asm
@--------------------------------------------------------------------------
@ DESCRIPTION  : Performs 2 by 2 down scale on input uint8_t image. Output is
@                int16_t.
@--------------------------------------------------------------------------
@ C PROTOTYPE  : void ds_2by2_asm(uint8_t *pSrc,
@                             const  uint32_t srcWidth,
@                             const  uint32_t srcHeight,
@                             const  uint32_t srcStride,
@                             int16_t *pDst,
@                             const  uint32_t dstStride)@
@--------------------------------------------------------------------------
@ REG INPUT    : R0: uint8_t  *pSrc
@                      Pointer to the input
@                R1: uint32_t srcWidth
@                      Input Image width
@                R2: uint32_t  srcHeight
@                      Input image height
@                R3: uint32_t srcStride
@                      Input Image stride
@--------------------------------------------------------------------------
@ STACK ARG    : uint8_t  *pDst
@                     Pointer to output image
@              : uint32_t dstStride
@                     Output Image stride
@--------------------------------------------------------------------------
@ REG OUTPUT   : None
@--------------------------------------------------------------------------
@ MEM INPUT    : pSrc - an input image
@--------------------------------------------------------------------------
@ MEM OUTPUT   : pDst - The downscaled output image in a larger buffer
@--------------------------------------------------------------------------
@ REG AFFECTED : none
@--------------------------------------------------------------------------
@ STACK USAGE  : none
@--------------------------------------------------------------------------
@ NOTES        :
@==========================================================================

pSrc        .req r0
pDst        .req r11
srcWidth    .req r1
srcHeight   .req r2
srcStride   .req r3
dstStride   .req r5
pSrc2       .req r12

    .global ds_2by2_asm
    .type ds_2by2_asm,  %function
    ds_2by2_asm:

    @==========================================================================
    @ Store stack registers
    @==========================================================================
    STMFD SP!, {R4-R11, LR}            @ push registers into stack

    VPUSH {D8-D15}

    LDR pDst, [SP, #100]               @ load source height from stack
    LDR dstStride, [SP, #104]          @ load source height from stack

    @==========================================================================
    @Keep the back up of pDst Pointer to update it after the completion of a row
    @Keep thr track of no of Rows completed processing the rescaling
    @==========================================================================
    MOV R8,pDst
    MOV R9,#0

    ADD  pSrc2, pSrc, srcStride        @ calculate the second line pointer

ds_2by2_asm_loop_vertical:

    MOV  R4, srcWidth                  @ restore the original width for each vertical loop
    MOV  R6, pSrc                      @ Store the Src Pointer

ds_2by2_asm_loop_horizontal:

    SUBS R7, R4, #16                   @ check the # of remaining length

    BGE regular_ds_2by2_asm            @ Regular processing if 16 or more pixels left

    @==========================================================================
    @ If less than 16 pixels left, load pixels two by two
    @ There are at least two pixel in the trailing part, load the first two
    @ pixels unconditionally
    @==========================================================================
    VLD2.8  {D2[0],D3[0]}, [pSrc]!
    VLD2.8  {D4[0],D5[0]}, [pSrc2]!
    ADDS R7, R7, #14           @ add the subtracted 16-2 back to the counter
    BLE  regular_ds_2by2_asm

    VLD2.8  {D2[1],D3[1]}, [pSrc]!
    VLD2.8  {D4[1],D5[1]}, [pSrc2]!
    SUBS R7, R7, #2            @ check if more pixels left in the trailing part
    BLE regular_ds_2by2_asm

    VLD2.8  {D2[2],D3[2]}, [pSrc]!
    VLD2.8  {D4[2],D5[2]}, [pSrc2]!
    SUBS R7, R7, #2            @ check if more pixels left in the trailing part
    BLE regular_ds_2by2_asm

    VLD2.8  {D2[3],D3[3]}, [pSrc]!
    VLD2.8  {D4[3],D5[3]}, [pSrc2]!
    SUBS R7, R7, #2            @ check if more pixels left in the trailing part
    BLE regular_ds_2by2_asm

    VLD2.8  {D2[4],D3[4]}, [pSrc]!
    VLD2.8  {D4[4],D5[4]}, [pSrc2]!
    SUBS R7, R7, #2            @ check if more pixels left in the trailing part
    BLE regular_ds_2by2_asm

    VLD2.8  {D2[5],D3[5]}, [pSrc]!
    VLD2.8  {D4[5],D5[5]}, [pSrc2]!
    SUBS R7, R7, #2            @ check if more pixels left in the trailing part
    BLE regular_ds_2by2_asm

    VLD2.8  {D2[6],D3[6]}, [pSrc]!
    VLD2.8  {D4[6],D5[6]}, [pSrc2]!

regular_ds_2by2_asm:

    SUBS R4, R4, #16           @ check the # of remaining length

    BLT skip_ds_2by2_asm       @ skip loading the next 16 values if less than 16 pixels left

    @==========================================================================
    @ Load 16 pixels from the first and the second lines
    @==========================================================================
    VLD2.8   {D2,D3}, [pSrc]!          @ D2: 14 12 10 8 6 4 2 0
                                       @ D3: 15 13 11 9 7 5 3 1

    VLD2.8   {D4,D5}, [pSrc2]!         @ D4:  o  m  k i g e c a
                                       @ D5:  p  n  l j h f d b
skip_ds_2by2_asm:

    @==========================================================================
    @ Perform the additions
    @==========================================================================
    VADDL.U8   Q4, D2, D3         @ Q4: 14+15 12+13 10+11 8+9 6+7 4+5 2+3 0+1
    VADDL.U8   Q5, D4, D5         @ Q5:  o+p   m+n   k+l  i+j g+h e+f c+d a+b

    VADD.I16   Q6, Q4, Q5         @ Q6:                         0+1+a+b

    @==========================================================================
    @ Right shift by 2, equivalent to divide by 4
    @==========================================================================
    VRSHRN.U16  D12, Q6, #2       @ Q6:                       (0+1+a+b)/4

    BLT trailing_ds_2by2_asm

    @==========================================================================
    @ Store 8 output values
    @==========================================================================
    VST1.8 {D12}, [pDst]!

    BGT ds_2by2_asm_loop_horizontal    @ loop if more pixels left

    BEQ  ds_2by2_asm_next_line         @ done if multiple of 16 pixles

trailing_ds_2by2_asm:

    @==========================================================================
    @ After horizontally down scaled by 2, there are from 1 ~ 7 pixels left in
    @ the trailing part. Note the original width is two times the down scaled
    @ width. R4 is the original decremented width.
    @ First add 14 to the length so the length would be from 0 to 12.
    @ eg: 6 pixel left in the trailing part, so 6-16+14 = 4.
    @ Then save 1 pixel unconditionally since at least 1 pixel left in the
    @ trailing part.
    @==========================================================================
    ADDS R4, R4,  #14

    VST1.8 {D12[0]}, [pDst]!
    BLE  ds_2by2_asm_next_line         @ done if 0 pixel left

    VST1.8 {D12[1]}, [pDst]!
    SUBS R4, R4,  #2                   @ update length counter
    BLE  ds_2by2_asm_next_line         @ done if 0 pixel left

    VST1.8 {D12[2]}, [pDst]!
    SUBS R4, R4,  #2                   @ update length counter
    BLE  ds_2by2_asm_next_line         @ done if 0 pixel left

    VST1.8 {D12[3]}, [pDst]!
    SUBS R4, R4,  #2                   @ update length counter
    BLE  ds_2by2_asm_next_line         @ done if 0 pixel left

    VST1.8 {D12[4]}, [pDst]!
    SUBS R4, R4,  #2                   @update length counter
    BLE  ds_2by2_asm_next_line         @ done if 0 pixel left

    VST1.8 {D12[5]}, [pDst]!
    SUBS R4, R4,  #2                   @ update length counter
    BLE  ds_2by2_asm_next_line         @ done if 0 pixel left

    VST1.8 {D12[6]}, [pDst]!
    SUBS R4, R4,  #2                   @ update length counter

ds_2by2_asm_next_line:

    ADD  pSrc, R6, srcStride           @ update the source line pointer
    ADD  pSrc, pSrc,srcStride          @ Skip a row as pSrc2 points that

    ADD  pSrc2, pSrc, srcStride        @ calculate the second line pointer

    @==========================================================================
    @ update the destination pointer
    @ At this point one line is stred in pDst. Since this is by 2 downscaling,
    @ pDst needs to increment by srcWidth/2 to point to the next line.
    @ However, pDst data type is int16_t (2-byte), so actually pDst needs
    @ to incremnt by srcWidth/2 * sizeof(int16_t) = srcWidth
    @==========================================================================

    @==========================================================================
    @ Update the pDst pointer by moving it a row of DstStride length
    @ Increment the R9 to store the no of rows completed
    @==========================================================================
    ADD R9, R9,#1
    MUL R10,R9,dstStride
    ADD pDst, R10,R8

    @ADD  pDst, pDst, dstStride          @ update the destination pointer

    SUBS srcHeight, srcHeight, #2        @ check the # of remaining height

    BGT ds_2by2_asm_loop_vertical        @ Process the next 2 lines

done_ds_2by2_asm:

    VPOP {D8-D15}

    LDMFD SP!, {R4-R11, PC}              @ pop registers from stack

    .end
