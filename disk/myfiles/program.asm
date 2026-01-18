bits 32

push eax
push ebx
push ecx

mov eax, 4
mov ebx, msg
mov ecx, 8
int 0x80

pop ecx
pop ebx
pop eax

msg db "Merhaba", 0