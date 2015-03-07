#include "tokenizer.h"
#include "util.h"

int main(int argc, char **args) {
  initIdMap();
  char *code = readFileWithPath(args[1]);
  List *tokens = tokenize(code);
  int i;
  for (i = 0; i < listSize(tokens); i++) {
    printf("%s\n", tokenTypeToStr(((Token *)listGet(tokens, i))->type));
  }
  return 0;
}
