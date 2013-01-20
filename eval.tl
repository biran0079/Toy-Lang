import hashtable;
import list;
import string;

fun newEnv(p){
  return [p,hashtable.newStringHashTable()];
}

fun envGet(e, key) {
  while(e!=none){
    if(hashtable.has(e[1], key)) {
      return hashtable.get(e[1],key);
    }
    e=e[0];
  }  
  return none;
}

fun envPut(e, key, value) {
  while(1){
    if(hashtable.has(e[1], key)){
      hashtable.put(e[1], key, value);
      return;
    } else {
      if(e[0] == none)  break;
      e = e[0];
    }
  }
  hashtable.put(e[1], key, value);
}

fun envPutLocal(e, key, value) {
  hashtable.put(e[1], key, value);
}


fun assert(b){
  if(!b){
    print("assertion failure\n"); 
    exit(-1);
  }
}

fun getBuiltinFuns(){
  local res;
  res = hashtable.newStringHashTable();
  fun put(name, f){
    hashtable.put(res, name, f);  
  }
  hashtable.put(res, "len", len);
  hashtable.put(res, "ord", ord);
  hashtable.put(res, "chr", chr);
  hashtable.put(res, "sort", sort);
  hashtable.put(res, "str", str);
  hashtable.put(res, "rand", rand);
  hashtable.put(res, "parse", parse);
  hashtable.put(res, "read", read);
  hashtable.put(res, "exit", exit);
  return res;
}

path = ["lib/", "./"];
fun readFromPath(f){
  local i;
  for(i=0;i<len(path);i++){
    s = read(path[i]+f);
    if(s!=none) return s;
  }
  print("failed to read " + f + "from path");
  exit(-1);
}

fun eval(e, l){
  local tag,funname,i,n,exps, builtInFun, res;
  builtInFun = getBuiltinFuns();
  tag = l[0];
  n = len(l);
  if(tag == "stmts") {
    for(i=1;i<n;i++)
      eval(e, l[i]);  
    return none;
  } else if(tag == "int")  return l[1];
  else if(tag == "+") return eval(e, l[1])+eval(e, l[2]); 
  else if(tag == "-") return eval(e, l[1])/eval(e, l[2]); 
  else if(tag == "*") return eval(e, l[1])*eval(e, l[2]); 
  else if(tag == "/") return eval(e, l[1])/eval(e, l[2]); 
  else if(tag == "%") return eval(e, l[1])%eval(e, l[2]); 
  else if(tag == ">") return eval(e, l[1])>eval(e, l[2]); 
  else if(tag == "<") return eval(e, l[1])<eval(e, l[2]); 
  else if(tag == ">=") return eval(e, l[1])>=eval(e, l[2]); 
  else if(tag == "<=") return eval(e, l[1])<=eval(e, l[2]); 
  else if(tag == "==") return eval(e, l[1])==eval(e, l[2]); 
  else if(tag == "!=") return eval(e, l[1])!=eval(e, l[2]); 
  else if(tag == "not") return !eval(e, l[1]);
  else if(tag == "none") return none;
  else if(tag == "id") return envGet(e, l[1]);
  else if(tag == "import") {
    local res;
    res = newEnv(e);
    eval(res, parse(readFromPath(l[1] + ".tl")));
    envPut(l[0][1], res);
  } else if(tag == "local") {
    for(i=1;i<len(l);i++)
      envPutLocal(e, l[i][1], none);
  } else if(tag == "list_access") {
    local lst, idx;
    lst = eval(e, l[1]);
    idx = eval(e, l[2]);
    return lst[idx];
  } else if(tag == "++") {
    local v,res;
    v = eval(e,l[1]);
    res = v++;
    if(l[1][0] == "id") {
      envPut(e, l[1][1], v);
    } else if(l[1][0] == "list_access") {
      local lst, idx;
      lst = eval(e, l[1][1]);
      idx = eval(e, l[1][2]);
      lst[idx]++;
    }
    return res;
  } else if(tag == "=") {
    local left, idx,lst;
    left = l[1];
    v = eval(e, l[2]);
    if(left[0]=="list_access") {
      lst = eval(e, left[1]);
      idx = eval(e, left[2]);
      lst[idx] = v;
    }else if(left[0]=="id"){
      envPut(e, left[1], v);
    }else{
      print("unknown type for =\n");
      exit(-1);
    }
  } else if(tag == "list") {
    res = []; 
    for(i=1;i<len(l);i++)
      res += [eval(e, l[i])];
    return res;
  } else if(tag == "exps") {
    for(i=0;i<len(l);i++)
      res = eval(e, l[i]);
    return res;
  } else if(tag == "call"){
    funname = l[1];
    if(funname[0]=="id"){
      if(funname[1]=="print"){
        exps=l[2];
        assert(exps[0]=="exps");
        args =list.map(lambda(x){return eval(e,x);}, list.suffix(exps, 1));
        print(string.join(" ", list.map(str, args)));
        return ;
      } else if(hashtable.has(builtInFun, funname[1])){
        local f;
        f = hashtable.get(builtInFun, funname[1]);
        exps=l[2];
        assert(exps[0]=="exps");
        if(len(exps) == 2) {
          return f(eval(e,exps[1]));
        } else if(len(exps) == 1){
          return  f();
        }
      }
    }
    closure = eval(e, funname);
    args =list.map(lambda(x){return eval(e,x);}, list.suffix(exps, 1));
    e2 = newEnv(closure[0]);
    f = closure[1];
    ids = f[1];
    fbody = f[2];
    assert(len(ids) == len(args));
    for(i=0;i<len(ids);i++)
      envPutLocal(e2, ids[i], args[i]);
    return eval(e2, fbody);
  }
}

code = parse("print(1+1, 3*4+5, str(123), rand(), 1<2, 3==3, 4>=4, !(1==1), [1,2,3]);\
    a=1;print(a);b=[1,2,3];b[0]=0;print(b);a++;print(a);b[0]++;\
    print(b);");

eval(newEnv(none), code);

e  =newEnv(newEnv(none));
