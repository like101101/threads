define val
    p /u (((unsigned long long)$arg0) << 32) | $arg1
    p /x (((unsigned long long)$arg0) << 32) | $arg1
end

define rdtsc
   val $edx $eax
end

set disassembly-flavor intel

display /1i $pc
display /x { $edx, $eax }
display /1gx ($rbp - 0x84)
display /1gx ($rbp - 0x88)
display /1gx ($rbp - 0x28)
display /1gx ($rbp - 0x7c)
display /1gx ($rbp - 0x80)
display /1gx ($rbp - 0x20)

b *0x00005555555555c2
b *0x00005555555555ff

run 80000 1