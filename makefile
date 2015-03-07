CFLAGS = -g -Wall
CC=clang

all: tl

%.tab.c %.tab.h: %.y
	bison -d $<

CORE_OBJS = ast.o builtinFun.o closure.o core.o env.o eval.o \
	gc.o hashTable.o list.o util.o value.o idMap.o opStack.o mem.o

PARSER_OBJs = tokenizer.o parser.o


TL_MAIN_OBJ = main.o

PARSER_MAIN_OBJ = parserMain.o

PARSER_BENCHMARK_OBJ = parserBenchmark.o

tl: $(CORE_OBJS) $(PARSER_OBJs) $(TL_MAIN_OBJ)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(PARSER_OBJs) $(TL_MAIN_OBJ) -o tl

parser: $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_MAIN_OBJ)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_MAIN_OBJ) -o parser

parserBenchmark: CFLAGS=-O3
parserBenchmark: $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_BENCHMARK_OBJ) $(YY_PARSER_OBJS)
	$(CC) $(CFLAGS) $(CORE_OBJS) $(PARSER_OBJs) $(PARSER_BENCHMARK_OBJ) $(YY_PARSER_OBJS) -o parserBenchmark

tokenizer: list.o idMap.o hashtable.o util.o tokenizerMain.o tokenizer.o
	$(CC) $(CFLAGS) list.o idMap.o hashtable.o util.o tokenizerMain.o tokenizer.o -o tokenizer


test: tl
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
	rm -f tl.exe tl *.o parser parser.exe\
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
