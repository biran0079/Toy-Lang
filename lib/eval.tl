import hashtable;
import lang;
import list;
import string;

fun eval(e, l){
  local tag,funname,i,n,exps, res;
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
    if(lang.envHas(e, l[1]))
      return lang.envGet(e, l[1]);
    return none;
  } else if(tag == "return") throw ["return", eval(e, l[1])];
  else if(tag == "module_access") {
    local res;
    res = e;
    for(i=1;i<len(l);i++){
      res = lang.envGet(res, l[i][1]);
    }
    return res;
  } else if(tag == "import") {
    local res, moduleName;
    res = lang.newEnv(e);
    moduleName = l[1][1];
    eval(res, parse(lang.readFromPath(moduleName + ".tl")));
    lang.envPut(e, moduleName, res);
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
      lang.envPut(e, l[1][1], i);
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
    local res, ids, fbody;
    ids = list.suffix(l[2], 1);
    fbody = l[3];
    res = newInternalFun(e, ids, fbody);
    lang.envPutLocal(e, l[1][1], res);
    return res;
  } else if(tag == "lambda") {
    local ids, fbody;
    ids = l[1];
    fbody = l[2];
    return newInternalFun(e, ids, fbody);
  } else if(tag == "local") {
    local ids;
    ids = l[1];
    for(i=1;i<len(ids);i++)
      lang.envPutLocal(e, ids[i][1], none);
  } else if(tag == "try") {
    try {
      eval(e, l[1]);
    } catch(ex) {
      if(ex[0] == "exception") {
        lang.envPutLocal(e, l[2][1], ex[1]);
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
      lang.envPut(e, l[1][1], v);
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
      lang.envPut(e, l[1][1], v);
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
      lang.envPut(e, left[1], v);
    }else if(left[0]=="module_access"){
      local res;
      res = e;
      for(i=1;i<len(left)-1;i++){
        res = lang.envGet(res, left[i][1]);
      }
      lang.envPut(res, left[i][1], v);
    }else {
      print("unknown type for =\n");
      exit(-1);
    }
    return v;
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
    exps=l[2];
    f = eval(e, l[1]);
    lang.assert(exps[0]=="exps", "exps type expected for arguments of function call");
    args = list.map(lambda(x){return eval(e,x);}, list.suffix(exps, 1));
    try{
      f(args);
    }catch(ex){
      if(ex[0]=="return") {
        return ex[1];
      } else {
        throw ex;
      }
    }
  }else{
    print("unown tag " + tag + " in " + str(l));
    exit(-1);
  }
}

fun newBuiltinFun(f) {
  fun internalBuiltinFun(args) {
    throw ["return", apply(f, args)];  
  }
  return internalBuiltinFun;
}

fun builtinApply(args) {
  throw ["return", apply(args[0], list.suffix(args, 1))];
}

fun newInternalFun(e, ids, fbody) {
  fun internalFunction(args) {
    local e2, i;
    e2 = lang.newEnv(e);
    lang.assert(len(ids) == len(args),str(len(ids)) + " parameter needed, " + str(len(args)) + " parameters get\n");
    for(i=0;i<len(ids);i++)
      lang.envPutLocal(e2, ids[i][1], args[i]);
    throw ["return", eval(e2, fbody)];
  }
  return internalFunction;
}
