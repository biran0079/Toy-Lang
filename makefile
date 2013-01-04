all: exe
test: exe
	./test.sh
exe: flex bison
	gcc -g *.c -o tl
bison: tl.y tl.h
	bison -d tl.y
flex: tl.l bison
	flex tl.l
clear:
	rm tl.exe tl *.o
