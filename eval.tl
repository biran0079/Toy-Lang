import hashtable;
import list;
import string;

code = parse("print(1+1, 3*4+5);");

fun assert(b){
  if(!b){
    print("assertion failure\n"); 
    exit(-1);
  }
}
fun eval(l){
  local tag,funname,i,n,exps;
  tag = l[0];
  n = len(l);
  if(tag == "stmts") {
    for(i=1;i<n;i++)
      eval(l[i]);  
    return none;
  } else if(tag == "int")  return l[1];
  else if(tag == "+") return eval(l[1])+eval(l[2]); 
  else if(tag == "-") return eval(l[1])/eval(l[2]); 
  else if(tag == "*") return eval(l[1])*eval(l[2]); 
  else if(tag == "/") return eval(l[1])/eval(l[2]); 
  else if(tag == "call"){
    funname = l[1];
    assert(funname[0]=="id");
    if(funname[1]=="print"){
      exps=l[2];
      assert(exps[0]=="exps");
      args =list.map(eval, list.suffix(exps, 1));
      print(string.join(" ", list.map(str, args)));
    }
  }
}

eval(code);

