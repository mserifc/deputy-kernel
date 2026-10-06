.section .data
    .global simd_SSEActivated
    simd_SSEActivated:
        .byte 0

.section .text
    # simd_copy(void* dst, const void* src, unsigned int len)
    .global simd_copy
    simd_copy:
        push %eax
        movb simd_SSEActivated, %al
        test %al, %al
        jz .copy_nosse
        push %esi
        push %edi
        push %ecx
        mov 20(%esp), %edi
        mov 24(%esp), %esi
        mov 28(%esp), %ecx
        .copy_loop:
            cmp $16, %ecx
            jb .copy_tail
            movdqu 0(%esi), %xmm0
            movdqu %xmm0, 0(%edi)
            add $16, %esi
            add $16, %edi
            sub $16, %ecx
            jmp .copy_loop
        .copy_tail:
            test %ecx, %ecx
            jz .copy_done
        .copy_bytecopy:
            movb 0(%esi), %al
            movb %al, 0(%edi)
            inc %esi
            inc %edi
            dec %ecx
            jnz .copy_bytecopy
        .copy_done:
            pop %ecx
            pop %edi
            pop %esi
            pop %eax
            ret
        .copy_nosse:
            pop %eax
            ret

    # simd_fill(void* dst, int value, unsigned int len)
    .global simd_fill
    simd_fill:
        push %eax
        movb simd_SSEActivated, %al
        test %al, %al
        jz .fill_nosse
        push %edi
        push %ecx
        push %edx
        mov 20(%esp), %edi       # dst
        mov 24(%esp), %edx       # value (byte)
        mov 28(%esp), %ecx       # len

        # Baytı 16 byte'a yay
        movzbl %dl, %eax
        imul $0x01010101, %eax, %eax
        movd %eax, %xmm0
        pshufd $0x00, %xmm0, %xmm0

        # --- Hizalama: 16-byte sınırına kadar tek tek yaz ---
        .fill_align:
            test %ecx, %ecx
            jz .fill_done
            test $15, %edi          # edi % 16 == 0 ?
            jz .fill_check_size   # hizalandı, devam
            movb %dl, 0(%edi)
            inc %edi
            dec %ecx
            jmp .fill_align

        # --- Hizalandıktan sonra: büyük mi küçük mü? ---
        .fill_check_size:
            cmp $262144, %ecx
            jae .fill_nt_loop

        # --- Normal SSE yolu (hizalı, movdqa) ---
        .fill_loop:
            cmp $16, %ecx
            jb .fill_tail
            movdqa %xmm0, 0(%edi)   # hizalı olduğu garantili
            add $16, %edi
            sub $16, %ecx
            jmp .fill_loop
        jmp .fill_tail

        # --- Non-temporal yol: 64 byte unroll ---
        .fill_nt_loop:
            cmp $64, %ecx
            jb .fill_nt_tail
            movntdq %xmm0,  0(%edi)
            movntdq %xmm0, 16(%edi)
            movntdq %xmm0, 32(%edi)
            movntdq %xmm0, 48(%edi)
            add $64, %edi
            sub $64, %ecx
            jmp .fill_nt_loop
        .fill_nt_tail:
            sfence

        .fill_tail:
            test %ecx, %ecx
            jz .fill_done
        .fill_byteset:
            movb %dl, 0(%edi)
            inc %edi
            dec %ecx
            jnz .fill_byteset
        .fill_done:
            pop %edx
            pop %ecx
            pop %edi
            pop %eax
            ret
        .fill_nosse:
            pop %eax
            ret

    # simd_extfill(void* dst, unsigned int value, unsigned int len)
    .global simd_extfill
    simd_extfill:
        push %eax
        movb simd_SSEActivated, %al
        test %al, %al
        jz .extfill_nosse
        push %edi
        push %ecx
        push %edx
        mov 20(%esp), %edi       # dst
        mov 24(%esp), %edx       # value (32-bit)
        mov 28(%esp), %ecx       # len (byte)

        # 32-bit değeri xmm0'a 4 kez yay: AABBCCDD AABBCCDD AABBCCDD AABBCCDD
        movd %edx, %xmm0
        pshufd $0x00, %xmm0, %xmm0

        # --- Hizalama: 16-byte sınırına kadar 4'er byte yaz ---
        .extfill_align:
            test %ecx, %ecx
            jz .extfill_done
            test $15, %edi
            jz .extfill_check_size
            cmp $4, %ecx
            jb .extfill_done         # 4 byte kalmadıysa dur
            mov %edx, 0(%edi)
            add $4, %edi
            sub $4, %ecx
            jmp .extfill_align

        # --- Hizalandıktan sonra: büyük mi küçük mü? ---
        .extfill_check_size:
            cmp $262144, %ecx
            jae .extfill_nt_loop

        # --- Normal SSE yolu (hizalı, movdqa) ---
        .extfill_loop:
            cmp $16, %ecx
            jb .extfill_tail
            movdqa %xmm0, 0(%edi)
            add $16, %edi
            sub $16, %ecx
            jmp .extfill_loop

        # --- Non-temporal yol: 64 byte unroll ---
        .extfill_nt_loop:
            cmp $64, %ecx
            jb .extfill_nt_tail
            movntdq %xmm0,  0(%edi)
            movntdq %xmm0, 16(%edi)
            movntdq %xmm0, 32(%edi)
            movntdq %xmm0, 48(%edi)
            add $64, %edi
            sub $64, %ecx
            jmp .extfill_nt_loop
        .extfill_nt_tail:
            sfence

        .extfill_tail:
            cmp $4, %ecx
            jb .extfill_done
            mov %edx, 0(%edi)
            add $4, %edi
            sub $4, %ecx
            jmp .extfill_tail
        .extfill_done:
            pop %edx
            pop %ecx
            pop %edi
            pop %eax
            ret
        .extfill_nosse:
            pop %eax
            ret
