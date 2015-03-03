#include "parser.h"
#include "util.h"

int main(int argc, char **args) {
  init(argc, args);
  if (argc != 2) {
    error("One argument (input file) required.");
  }
  char *src = args[1];
  char *code = readFileWithPath(src);
  List *tokens = tokenize(code);
  Node *tree = parse(tokens);
  if (!tree) {
    error("failed to parse file %s\n", src);
  }
  freeTokenList(tokens);
  printAst(tree);
  cleanup();
  return 0;
}
