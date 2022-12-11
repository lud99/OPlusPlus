cd Build
nasm -f win32 temp\program.asm -o temp\program.o

gcc -o program temp\program.o -m32