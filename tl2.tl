import eval2;
import lang;
import list;

args = sysArgs();
if(len(args) < 2) {
  print("one argument required for eval.tl\n");  
  exit(-1);
}
code = parse(read(args[1]));
f = eval2.process(code);
global = lang.newGlobalEnv();
f(global);

