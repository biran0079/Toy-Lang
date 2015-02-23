gccflag = -DYYDEBUG=0 -g


all: exe

test: exe
	./test.sh

testparser: parser
	./testparser.sh

testeval: exe
	./testeval.sh

gctest: exe
	./gctest.sh

exe: flex bison
	gcc $(gccflag) *.c -o tl -DBUILD_INTERPRETER

tokenizer: *.c
	gcc $(gccflag) *.c -o tokenizer -DBUILD_TOKENIZER

parser: *.c
	gcc $(gccflag) *.c -o parser -DBUILD_PARSER

bison: tl.y
	bison -d tl.y

flex: tl.l bison
	flex tl.l

clear:
	rm tl.exe tl *.o lex.yy.c tl.tab.c tl.tab.h

bench: exe
	./benchmark.sh

draw: exe
	./draw.sh
