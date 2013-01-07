gccflag = -DYYDEBUG=0 -g
all: exe
test: exe
	./test.sh -l
exe: flex bison
	gcc $(gccflag) *.c -o tl
bison: tl.y tl.h
	bison -d tl.y
flex: tl.l bison
	flex tl.l
clear:
	rm tl.exe tl *.o
bench: exe
	./benchmark.sh
draw: exe
	./draw.sh
