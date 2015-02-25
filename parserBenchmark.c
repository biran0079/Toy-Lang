#include "parser.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char **args) {
  init(argc, args);
  if (argc != 2) {
    error("One argument (input file) required.");
  }
  char *src = args[1];
  char *code = readFileWithPath(src);

  clock_t st = clock();
  int i;
  for (i = 0; i < 100; i++) parse(tokenize(code));
  printf("new parser %f src\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);

  FILE *f = fopen(src, "r");

  st = clock();
  for (i = 0; i < 100; i++) {
    fseek(f, 0L, SEEK_SET);
    yyrestart(f);
    yyparse();
  }
  printf("yy parser %f src\n", (clock() - st) * 1.0 / CLOCKS_PER_SEC);

  fclose(f);
  return 0;
}
