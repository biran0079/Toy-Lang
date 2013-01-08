gccflag = -DYYDEBUG=0 -g

src = $(wildcard *.c)

all: exe

test: exe
	./test.sh

exe: flex bison 
	gcc $(gccflag) *.c -o tl

bison: tl.y
	bison -d tl.y

flex: tl.l bison
	flex tl.l

clear:
	rm tl.exe tl *.o

bench: exe
	./benchmark.sh

draw: exe
	./draw.sh

