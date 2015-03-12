import lang;
import list;

fun getTag(l) {
  return l[0];
}

fun withoutTag(l) {
  return list.suffix(l, 1);
}

fun processBinaryOperator(l, r, f) {
  local left, right;
  left = process(l);
  right = process(r);
  return lambda(e) {
    return f(left(e), right(e));
  };
}

fun processUnaryOperator(x, f) {
  local g;
  g = process(x);
  return lambda(e) {
    return f(g(e));
  };
}

fun process(l) {
  local tag, stmts;
  tag = getTag(l);
  if(tag == "stmts") {
    stmts = list.map(process, withoutTag(l));
    return lambda(e) {
      for (stmt : stmts) {
        stmt(e);
      }
      return none;
    };
  } else if(tag == "int") {
    return lambda(e) {
      return l[1];
    };
  } else if(tag == "+") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x + y;});
  } else if(tag == "-") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x - y;});
  } else if(tag == "*") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x * y;});
  } else if(tag == "/") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x / y;});
  } else if(tag == "%") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x % y;});
  } else if(tag == ">") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x > y;});
  } else if(tag == "<") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x < y;});
  } else if(tag == ">=") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x >= y;});
  } else if(tag == "<=") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x <= y;});
  } else if(tag == "==") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x == y;});
  } else if(tag == "&&") {
    local left, right;
    left = process(l[1]);
    right = process(l[2]);
    return lambda(e) {
      return left(e) && right(e);
    };
  } else if(tag == "||") {
    local left, right;
    left = process(l[1]);
    right = process(l[2]);
    return lambda(e) {
      return left(e) || right(e);
    };
  } else if(tag == "!=") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x != y;});
  } else if(tag == "not") {
    return processUnaryOperator(l[1], lambda(x) {return !x;});
  } else if(tag == "none") {
    return lambda(e) {return none;};
  } else if(tag == "id") {
    return lambda(e) {
      if(lang.envHas(e, l[1]))
        return lang.envGet(e, l[1]);
    };
  } else if(tag == "return") {
    local f;
    f = process(l[1]);
    return lambda(e) {
      throw ["return", f(e)];
    };
  } else if(tag == "module_access") {
    return lambda(e) {
      local res;
      res = e;
      for(i=1;i<len(l);i++){
        res = lang.envGet(res, l[i][1]);
      }
      return res;
    };
  } else if(tag == "import") {
    return lambda(e) {
      local res, moduleName;
      res = lang.newEnv(e);
      moduleName = l[1][1];
      process(parse(lang.readFromPath(moduleName + ".tl")))(res);
      lang.envPut(e, moduleName, res);
    };
  } else if(tag == "continue") {
    return lambda(e) {
      throw ["continue"];
    };
  } else if(tag == "break") {
    return lambda(e) {
      throw ["break"];
    };
  } else if(tag == "for") {
    local ini, cond, inc, body;
    ini = process(l[1]);
    cond = process(l[2]);
    inc = process(l[3]);
    body = process(l[4]);
    return lambda(e) {
      for (ini(e); cond(e); inc(e)) {
        try {
          body(e);
        } catch(ex) {
          if(ex[0] == "continue"){
            continue;
          } else if(ex[0] == "break") {
            break;
          } else {
            throw ex;
          }
        }
      }
    };
  } else if(tag == "foreach") {
    local lst, body;
    lst = process(l[2]);
    body = process(l[3]);
    return lambda(e) {
      for (i : lst(e)) {
        lang.envPut(e, l[1][1], i);
        try {
          body(e);
        } catch(ex) {
          if(ex[0] == "continue"){
            continue;
          } else if(ex[0] == "break") {
            break;
          } else {
            throw ex;
          }
        }
      }
    };
  } else if(tag == "while") {
    local cond, body;
    cond = process(l[1]);
    body = process(l[2]);
    return lambda(e) {
      while (cond(e)) {
        try {
          body(e);
        } catch(ex) {
          if(ex[0] == "continue"){
            continue;
          } else if(ex[0] == "break") {
            break;
          } else {
            throw ex;
          }
        }
      }
    };
  } else if(tag == "if") {
    local cond, branch1, branch2;
    cond = process(l[1]);
    branch1 = process(l[2]);
    if (len(l) == 4) {
      branch2 = process(l[3]);
    } else {
      branch2 = lambda(e) {};
    }
    return lambda(e) {
      if (cond(e)) {
        branch1(e);
      } else {
        branch2(e);
      }
    };
  } else if(tag == "fun") {
    local fbody;
    fbody = process(l[3]);
    return lambda(e) {
      local res, ids;
      ids = list.suffix(l[2], 1);
      res = newInternalFun(e, ids, fbody);
      lang.envPutLocal(e, l[1][1], res);
      return res;
    };
  } else if(tag == "lambda") {
    return lambda(e) {
      local ids, fbody;
      ids = l[1];
      fbody = l[2];
      return newInternalFun(e, ids, fbody);
    };
  } else if(tag == "local") {
    return lambda(e) {
      local ids;
      ids = list.suffix(l[1], 1);
      for(id : ids) {
        lang.envPutLocal(e, id[1], none);
      }
    };
  } else if(tag == "try") {
    local tryb, catchb, finallyb;
    tryb = process(l[1]);
    catchb = process(l[3]);
    if (len(l) == 5) {
      finallyb = process(l[4]);
    } else {
      finallyb = lambda(e) {};
    }
    return lambda(e) {
      try {
        tryb(e);
      } catch(ex) {
        if(ex[0] == "exception") {
          lang.envPutLocal(e, l[2][1], ex[1]);
          catchb(e);
        } else {
          throw ex;
        }
      } finally {
        finallyb(e);
      }
    };
  } else if(tag == "throw") {
    local f;
    f = process(l[1]);
    return lambda(e) {
      throw ["exception", f(e)];
    };
  } else if(tag == "list_access") {
    local lst, idx;
    lst = process(l[1]);
    idx = process(l[2]);
    return lambda(e) {
      return lst(e)[idx(e)];
    };
  } else if(tag == "+=") {
    local left, right, lst, idx;
    left = process(l[1]);
    right = process(l[2]);
    if (l[1][0] == "list_access") {
      lst = process(l[1][1]);
      idx = process(l[1][2]);
    }
    return lambda(e) {
      local res, lv;
      res = left(e);
      res += right(e);
      if(l[1][0] == "id") {
        lang.envPut(e, l[1][1], res);
      } else if(l[1][0] == "list_access") {
        lst(e)[idx(e)] = res;
      }
      return res;
    };
  } else if(tag == "++") {
    local f, lst, idx;
    f = process(l[1]);
    if (l[1][0] == "list_access") {
      lst = process(l[1][1]);
      idx = process(l[1][2]);
    }
    return lambda(e) {
      local res;
      res = f(e);
      if(l[1][0] == "id") {
        lang.envPut(e, l[1][1], res + 1);
      } else if(l[1][0] == "list_access") {
        lst(e)[idx(e)] = res + 1;
      }
      return res;
    };
  } else if(tag == "=") {
    local left, idx, lst, f;
    left = l[1];
    f = process(l[2]);
    if(left[0]=="list_access") {
      lst = process(left[1]);
      idx = process(left[2]);
      return lambda(e) {
        local res;
        res = f(e);
        lst(e)[idx(e)] = res;
        return res;
      };
    }else if(left[0]=="id"){
      return lambda(e) {
        local res;
        res = f(e);
        lang.envPut(e, left[1], res);
        return res;
      };
    }else if(left[0]=="module_access"){
      return lambda(e) {
        local res;
        res = e;
        for(i=1;i<len(left)-1;i++){
          res = lang.envGet(res, left[i][1]);
        }
        lang.envPut(res, left[i][1], f(e));
      };
    }else {
      print("unknown type for =\n");
      exit(-1);
    }
  } else if(tag == "list") {
    local exps;
    exps = list.map(process, withoutTag(l));
    return lambda(e) {
      return list.map(lambda(f) {return f(e);}, exps);  
    };
  } else if(tag == "string") {
    return lambda(e) {
      return l[1];  
    };
  } else if(tag == "exps") {
    local exps;
    exps = list.map(process, withoutTag(l));
    return lambda(e) {
      local res;
      res = list.map(lambda(f) {return f(e);}, exps);  
      return res[len(res) - 1];
    };
  } else if(tag == "call" || tag == "tail_call"){
    local f, args, exps;
    exps=l[2];
    f = process(l[1]);
    lang.assert(exps[0]=="exps", "exps type expected for arguments of function call");
    args = list.map(lambda(x){return process(x);}, list.suffix(exps, 1));
    return lambda(e) {
      try{
        argsv = list.map(lambda(f) {return f(e);}, args);
        fv = f(e);
        fv(argsv);
      }catch(ex){
        if(ex[0]=="return") {
          return ex[1];
        } else {
          throw ex;
        }
      }
    };
  }else{
    print("unown tag " + tag + " in " + str(l));
    exit(-1);
  }
}


fun newInternalFun(e, ids, fbody) {
  fun internalFunction(args) {
    local e2, i;
    e2 = lang.newEnv(e);
    lang.assert(len(ids) == len(args),str(len(ids)) + " parameter needed, " + str(len(args)) + " parameters get\n");
    for(i=0;i<len(ids);i++)
      lang.envPutLocal(e2, ids[i][1], args[i]);
    throw ["return", fbody(e2)];
  }
  return internalFunction;
}
