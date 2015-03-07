#include "core.h"
#include "ast.h"
#include "list.h"
#include "value.h"
#include "env.h"
#include "gc.h"
#include "util.h"
#include "tokenizer.h"
#include "parser.h"
#include "eval.h"
#include "opStack.h"

extern List *parseTrees;
extern Env *globalEnv;
extern int sysArgc;
extern char **sysArgv;

static void run(char* src) {
  char *code = readFileWithPath(src);
  if (!code) error("cannot read file %s\n", src);
  List *tokens = tokenize(code);
  tlFree(code);
  parse(tokens);
  freeTokenList(tokens);
  Node *tree = listLast(parseTrees);
  if (0 == eval(globalEnv, tree)) {
    opStackPop();  // pop eval result
  }
  cleanup();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    error("input file required\n");
  }
  init(argc, argv);
  char *src = argv[1];
  sysArgc = argc - 1;
  sysArgv = argv + 1;

  run(src);

  return 0;
}
