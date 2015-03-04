CFLAGS = -g -Wall
CC=clang

all: tl

lex.yy.c: tl.l tl.tab.c
	flex tl.l

%.tab.c %.tab.h: %.y
	bison -d $<

CORE_OBJS = ast.o builtinFun.o closure.o compile.o core.o dumpGCHistory.o env.o eval.o \
	gc.o hashTable.o list.o toDot.o util.o value.o idMap.o opStack.o mem.o

PARSER_OBJs = tokenizer.o parser.o 

YY_PARSER_OBJS = lex.yy.o tl.tab.o

TL_MAIN_OBJ = main.o

PARSER_MAIN_OBJ = parserMain.o

PARSER_BENCHMARK_OBJ = parserBenchmark.o

tl: $(CORE_OBJS) $(PARSER_OBJs) $(TL_MAIN_OBJ)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(PARSER_OBJs) $(TL_MAIN_OBJ) -o tl

# clear before building yytl
yytl: CFLAGS = -DYYDEBUG=0 -g -DUSE_YY_PARSER
yytl: CC = gcc
yytl: $(CORE_OBJS) $(TL_MAIN_OBJ) $(YY_PARSER_OBJS)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(TL_MAIN_OBJ) $(YY_PARSER_OBJS) -o tl

parser: $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_MAIN_OBJ)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_MAIN_OBJ) -o parser

parserBenchmark: CFLAGS=-O3
parserbenchmark: $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_BENCHMARK_OBJ) $(YY_PARSER_OBJS)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_BENCHMARK_OBJ) $(YY_PARSER_OBJS) -o parserbenchmark 

tokenizer: list.o idMap.o hashtable.o util.o tokenizerMain.o tokenizer.o
	$(CC) $(CFLAGS) list.o idMap.o hashtable.o util.o tokenizerMain.o tokenizer.o -o tokenizer


test: tl test/*.tl test/*.tl.out
	./test.sh

testparser: parser
	./testparser.sh

testeval: tl
	./test.sh tl.tl

testevaleval: tl
	./test.sh tl.tl tl.tl

testgc: tl
	./gctest.sh

clear:
	rm -f tl.exe tl *.o lex.yy.c tl.tab.c tl.tab.h parser parser.exe\
	tokenizer tokenizer.exe parserbenchmark parserbenchmark.exe

benchmark: CFLAGS=-O3
benchmark: tl
	./benchmark.sh

draw: tl
	./draw.sh

format:
	clang-format -i --style=Google *.c *.h

testeval2: tl
	./test.sh tl2.tl

testeval2eval2: tl
	./test.sh tl2.tl tl2.tl

testevaleval2: tl
	./test.sh tl.tl tl2.tl

testeval2eval: tl
	./test.sh tl2.tl tl.tl

debug: CFLAGS = -DDEBUG_GC -g
debug: all
