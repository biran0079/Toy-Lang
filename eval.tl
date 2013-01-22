import hashtable;
import list;
import string;

fun newEnv(p){
  return [p,hashtable.newStringHashTable()];
}

fun envHas(e, key) {
  while(e!=none){
    if(hashtable.has(e[1], key)) {
      return 1;
    }
    e=e[0];
  }  
  return 0;
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
  e0 = e;
  while(1){
    if(hashtable.has(e[1], key)){
      hashtable.put(e[1], key, value);
      return;
    } else {
      if(e[0] == none)  break;
      e = e[0];
    }
  }
  hashtable.put(e0[1], key, value);
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
  hashtable.put(res, "print", print);
  hashtable.put(res, "len", len);
  hashtable.put(res, "ord", ord);
  hashtable.put(res, "chr", chr);
  hashtable.put(res, "sort", sort);
  hashtable.put(res, "str", str);
  hashtable.put(res, "rand", rand);
  hashtable.put(res, "parse", parse);
  hashtable.put(res, "read", read);
  hashtable.put(res, "exit", exit);
  hashtable.put(res, "sysArgs", sysArgs);
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
  else if(tag == "+") return eval(e, l[1]) + eval(e, l[2]); 
  else if(tag == "-") return eval(e, l[1]) - eval(e, l[2]); 
  else if(tag == "*") return eval(e, l[1]) * eval(e, l[2]); 
  else if(tag == "/") return eval(e, l[1]) / eval(e, l[2]); 
  else if(tag == "%") return eval(e, l[1]) % eval(e, l[2]); 
  else if(tag == ">") return eval(e, l[1]) > eval(e, l[2]); 
  else if(tag == "<") return eval(e, l[1]) < eval(e, l[2]); 
  else if(tag == ">=") return eval(e, l[1]) >= eval(e, l[2]); 
  else if(tag == "<=") return eval(e, l[1]) <= eval(e, l[2]); 
  else if(tag == "==") return eval(e, l[1]) == eval(e, l[2]); 
  else if(tag == "&&") return eval(e, l[1]) && eval(e, l[2]); 
  else if(tag == "||") return eval(e, l[1]) || eval(e, l[2]); 
  else if(tag == "!=") return eval(e, l[1]) != eval(e, l[2]); 
  else if(tag == "not") return !eval(e, l[1]);
  else if(tag == "none") return none;
  else if(tag == "id") {
    if(envHas(e, l[1]))
      return envGet(e, l[1]);
    if(hashtable.has(builtInFun, l[1]))
      return ["builtinFun", hashtable.get(builtInFun, l[1])];
    return none;
  } else if(tag == "return") throw ["return", eval(e, l[1])];
  else if(tag == "module_access") {
    local res;
    res = e;
    for(i=1;i<len(l);i++){
      res = envGet(res, l[i][1]);
    }
    return res;
  } else if(tag == "import") {
    local res, moduleName;
    res = newEnv(e);
    moduleName = l[1][1];
    eval(res, parse(readFromPath(moduleName + ".tl")));
    envPut(e, moduleName, res);
  } else if(tag == "continue") {
    throw ["continue"];
  } else if(tag == "break") {
    throw ["break"];
  } else if(tag == "for") {
    for(eval(e, l[1]); eval(e, l[2]); eval(e, l[3])){
      try{
        eval(e, l[4]);
      }catch(ex){
        if(ex[0] == "continue"){
          continue;
        } else if(ex[0] == "break") {
          break;
        } else {
          throw ex;
        }
      }  
    }
  } else if(tag == "foreach") {
    for(i : eval(e, l[2])) {
      envPut(e, l[1][1], i);
      try {
        eval(e, l[3]);
      }catch(ex){
        if(ex[0] == "continue"){
          continue;
        } else if(ex[0] == "break") {
          break;
        } else {
          throw ex;
        }
      }  
    }
  } else if(tag == "while") {
    while(eval(e,l[1])){
      try{
        eval(e,l[2]);  
      }catch(ex){
        if(ex[0] == "continue"){
          continue;
        } else if(ex[0] == "break") {
          break;
        } else {
          throw ex;
        }
      }  
    }
  } else if(tag == "if") {
    if(eval(e,l[1]))
      eval(e, l[2]);
    else {
      if(len(l) == 4)
        eval(e, l[3]);
    }
  } else if(tag == "fun") {
    local closure;
    closure = [e, list.suffix(l, 1)];
    envPut(e, l[1][1], closure);
    return closure;
  } else if(tag == "lambda") {
    local closure;
    closure = [e, [["id", "lambda"], l[1], l[2]]];
    return closure;
  } else if(tag == "local") {
    local ids;
    ids = l[1];
    for(i=1;i<len(ids);i++)
      envPutLocal(e, ids[i][1], none);
  } else if(tag == "try") {
    try {
      eval(e, l[1]);
    } catch(ex) {
      if(ex[0] == "exception") {
        envPutLocal(e, l[2][1], ex[1]);
        eval(e, l[3]);
      } else {
        throw ex;
      }
    } finally {
      if(len(l) == 5)  
        eval(e, l[4]);
    }
  } else if(tag == "throw") {
    local ex;
    ex = ["exception", eval(e, l[1])];
    throw ex;
  } else if(tag == "time") {
    time(eval(e, l[1]));
  } else if(tag == "list_access") {
    local lst, idx;
    lst = eval(e, l[1]);
    idx = eval(e, l[2]);
    return lst[idx];
  } else if(tag == "+=") {
    local v,res;
    v = eval(e,l[1]);
    res = (v+=eval(e, l[2]));
    if(l[1][0] == "id") {
      envPut(e, l[1][1], v);
    } else if(l[1][0] == "list_access") {
      local lst, idx;
      lst = eval(e, l[1][1]);
      idx = eval(e, l[1][2]);
      lst[idx] = v;
    }
    return res;
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
    return v;
  } else if(tag == "local") {
    for(i=1;i<len(l);i++)
      envPut(e, l[i][1], none);
  } else if(tag == "list") {
    res = []; 
    for(i=1;i<len(l);i++){
      res += [eval(e, l[i])];
    }
    return res;
  } else if(tag == "string") {
    return l[1];
  } else if(tag == "exps") {
    for(i=1;i<len(l);i++)
      res = eval(e, l[i]);
    return res;
  } else if(tag == "call" || tag == "tail_call"){
    funname = l[1];
    exps=l[2];
    closure = eval(e, funname);
    if(closure[0] == "builtinFun") {
      local f;
      if(closure[1]==print){
        assert(exps[0]=="exps");
        args =list.map(lambda(x){return eval(e,x);}, list.suffix(exps, 1));
        print(string.join(" ", list.map(str, args)));
        return ;
      } else {
        f = closure[1];
        assert(exps[0]=="exps");
        if(len(exps) == 2) {
          return f(eval(e,exps[1]));
        } else if(len(exps) == 1){
          return  f();
        }
      }
    }else{
      args =list.map(lambda(x){return eval(e,x);}, list.suffix(exps, 1));
      e2 = newEnv(closure[0]);
      f = closure[1];
      ids = list.suffix(f[1], 1);
      fbody = f[2];
      assert(len(ids) == len(args));
      for(i=0;i<len(ids);i++)
        envPutLocal(e2, ids[i][1], args[i]);
      try{
        eval(e2, fbody);
      }catch(ex){
        if(ex[0]=="return") {
          return ex[1];
        } else {
          throw ex;
        }
      }
    }
  }else{
    print("unown tag " + tag + " in " + str(l));
    exit(-1);
  }
}
