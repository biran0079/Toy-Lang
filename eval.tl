import hashtable;

code = parse("print(1+1);");

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
      print(eval(exps[1]));
    }
  }
}

eval(code);

