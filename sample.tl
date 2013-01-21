import eval;

code = parse(read("test/while.tl"));
eval.eval(eval.newEnv(none), code);
