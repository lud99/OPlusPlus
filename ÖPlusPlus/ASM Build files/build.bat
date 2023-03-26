nasm -f win32 generated\program.asm -o generated\program.o

gcc -o program generated\program.o -m32