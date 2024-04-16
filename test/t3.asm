mov [0], 'n'
mov [1], 'p'
mov [2], 1
mov edx, ds
mov cl, 1
mov ch, 1
mov al, 1
cmp [2], 0
jn final
add edx, 2
final: sys 2
stop
