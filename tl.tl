import eval;
import list;
import lang;

args = sysArgs();
if(len(args) < 2) {
  print("one argument required for eval.tl\n");  
  exit(-1);
}
code = parse(read(args[1]));
global = lang.newGlobalEnv();
eval.eval(global, code);
