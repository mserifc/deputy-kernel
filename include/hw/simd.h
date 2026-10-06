#pragma once

extern char simd_SSEActivated;

void simd_copy(void* dst, const void* src, unsigned int len);
void simd_fill(void* dst, int value, unsigned int len);
void simd_extfill(void* dst, unsigned int value, unsigned int len);