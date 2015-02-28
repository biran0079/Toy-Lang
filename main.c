#include "core.h"
#include "ast.h"
#include "list.h"
#include "value.h"
#include "env.h"
#include "gc.h"
#include "dumpGCHistory.h"
#include "util.h"
#include "tokenizer.h"
#include "parser.h"
#include "toDot.h"
#include "eval.h"
#include "opStack.h"

extern List *parseTrees;
extern List *values;
extern List *path;
extern Value *globalEnv;
extern int memoryLimit;
extern int shouldDumpGCHistory, gcTestMode;
extern int sysArgc;
extern char **sysArgv;
#ifdef USE_YY_PARSER
extern FILE *yyin;
#endif

void help() {
  fprintf(stderr, "Usage: tl [<options>] <filename>\n");
  fprintf(stderr,
          "\t-d\tinstead of evaluating the program, it converts tabstract "
          "syntax tree to dot language, \n"
          "\t\twhich can be compiled to image using dot tool\n"
          "\t-m <int>\tconfig memory limit for trigering GC\n"
          "\t-h\tdump GC history to chart in html when GC\n"
          "\t-g\tGC test mode, whenever gc is called, forceGC is called. "
          "Memory limit is ignored.\n"
          "\t-p\tprint abstract syntax tree without executing the code.\n");
  exit(-1);
}

int main(int argc, char **argv) {
  int toDot = 0;
  int i;
  char *src = 0;
  int printParseTree = 0;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'd':
          toDot = 1;
          break;
        case 'm':
          i++;
          memoryLimit = atoi(argv[i]);
          break;
        case 'h':
          shouldDumpGCHistory = 1;
          break;
        case 'g':
          gcTestMode = 1;
          break;
        case 'p':
          printParseTree = 1;
          break;
        default:
          help();
      }
    } else {
      src = argv[i];
      sysArgc = argc - i;
      sysArgv = argv + i;
      break;
    }
  }
  if (!src) {
    error("One input file argument is required.");
  }
#ifdef USE_YY_PARSER
  yyin = fopen(src, "r");
  if (!yyin) {
    error("cannot open input file\n");
  }
#endif
  init(argc, argv);
  char *s = getFolder(src);
  if (strcmp(s, "./")) listPush(path, s);
#ifdef USE_YY_PARSER
  yyparse();
#else
  char* code = readFileWithPath(src);
  if (!code) error("cannot open input file\n");
  List *tokens = tokenize(code);
  parse(tokens);
#endif
  if (toDot) {
    char *s = catStr(src, ".dot");
    FILE *f = fopen(s, "w");
    if (!f) error("failed to open %s\n", s);
    nodeToDot(f, listGet(parseTrees, 0));
    fclose(f);
    tlFree(s);
  } else if (printParseTree) {
    printAst(listGet(parseTrees, 0));
  } else {
    Node *tree = listGet(parseTrees, 0);
    if (0 == eval(globalEnv, tree)) {
      opStackPop(); // pop eval result
    }
  }
  cleanup();
  if (shouldDumpGCHistory) {
    char *s = catStr(src, ".html");
    FILE *f = fopen(s, "w");
    if (!f) error("failed to open %s\n", s);
    dumpGCHistory(f);
    fclose(f);
    free(s);
    clearGCHistory();
  }
  return 0;
}
