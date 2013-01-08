gccflag = -DYYDEBUG=0 -g

src = $(wildcard *.c)
obj = $(src:.cpp=.o)

all: exe

test: exe
	./test.sh

exe: flex bison $(obj)
	gcc $(gccflag) $(obj) -o tl

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

%.o: %.c
	gcc -c $(gccflag) $<
