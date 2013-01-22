import eval;

code = parse("for(i=0;i<10;i++)print(i);");
eval.eval(eval.newEnv(none), code);
