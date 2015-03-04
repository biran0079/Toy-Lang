import hashtable;
import list;

fun newEnv(p){
  local res;
  res = [p,hashtable.newStringHashTable()];
  envPut(res, "this", res);
  return res;
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


fun assert(b, msg){
  if(!b){
    print("assertion failure\n"); 
    print(msg);
    exit(-1);
  }
}

fun setUpBuiltinFuns(e){
  envPut(e, "print", newBuiltinFun(print));
  envPut(e, "len", newBuiltinFun(len));
  envPut(e, "ord", newBuiltinFun(ord));
  envPut(e, "chr", newBuiltinFun(chr));
  envPut(e, "sort", newBuiltinFun(sort));
  envPut(e, "str", newBuiltinFun(str));
  envPut(e, "rand", newBuiltinFun(rand));
  envPut(e, "parse", newBuiltinFun(parse));
  envPut(e, "read", newBuiltinFun(read));
  envPut(e, "exit", newBuiltinFun(exit));
  envPut(e, "apply", builtinApply);
  envPut(e, "sysArgs", newBuiltinFun(lambda() {return list.suffix(sysArgs(), 1);}));
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

fun newGlobalEnv() {
  local res;
  res = newEnv(none);
  setUpBuiltinFuns(res);
  return res;
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
    e2 = newEnv(e);
    assert(len(ids) == len(args),str(len(ids)) + " parameter needed, " + str(len(args)) + " parameters get\n");
    for(i=0;i<len(ids);i++)
      envPutLocal(e2, ids[i][1], args[i]);
    throw ["return", eval(e2, fbody)];
  }
  return internalFunction;
}
