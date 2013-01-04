gccflag = -DYYDEBUG=0
all: exe
test: exe
	./test.sh
exe: flex bison
	gcc *.c -o tl $(gccflag)
bison: tl.y tl.h
	bison -d tl.y
flex: tl.l bison
	flex tl.l
clear:
	rm tl.exe tl *.o
