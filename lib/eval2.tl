import core;
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

fun runLoopBody(body, e) {
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
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x && y;});
  } else if(tag == "||") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x || y;});
  } else if(tag == "!=") {
    return processBinaryOperator(l[1], l[2], lambda(x, y) {return x != y;});
  } else if(tag == "not") {
    return processUnaryOperator(l[1], lambda(x) {return !x;});
  } else if(tag == "none") {
    return lambda(e) {return none;};
  } else if(tag == "id") {
    return lambda(e) {
      if(core.envHas(e, l[1]))
        return core.envGet(e, l[1]);
    };
  } else if(tag == "return") {
    local f;
    f = process(l[1]);
    return lambda(e) {
      throw ["return", t(e)];
    };
  } else if(tag == "import") {
    return lambda(e) {
      local res, moduleName;
      res = core.newEnv(e);
      moduleName = l[1][1];
      process(parse(core.readFromPath(moduleName + ".tl")))(res);
      core.envPut(e, moduleName, res);
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
        runLoopBody(body, e);
      }
    };
  } else if(tag == "foreach") {
    local lst, body;
    lst = process(l[2]);
    body = process(l[3]);
    return lambda(e) {
      for (i : lst(e)) {
        core.envPut(e, l[1][1], i);
        runLoopBody(body, e);
      }
    };
  } else if(tag == "while") {
    local cond, body;
    cond = process(l[1]);
    body = process(l[2]);
    return lambda(e) {
      while (cond(e)) {
        runLoopBody(body, e);
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
    return lambda(e) {
      local res, ids, fbody;
      ids = list.suffix(l[2], 1);
      fbody = l[3];
      res = core.newInternalFun(e, ids, fbody);
      core.envPutLocal(e, l[1][1], res);
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
        envPutLocal(e, id[1], none);
      }
    };
  } else if(tag == "try") {
    local tryb, catchb, finallyb;
    tryb = process(l[1]);
    catchb = process(l[2]);
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
          core.envPutLocal(e, l[2][1], ex[1]);
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
  } else if(tag == "time") {
    local f;
    f = process(l[1]);
    return lambda(e) {
      return time(f(e));
    };
  } else if(tag == "list_access") {
    local lst, idx;
    lst = process(l[1]);
    idx = process(l[2]);
    return lambda(e) {
      return lst(e)[idx(e)];
    };
  } else if(tag == "+=") {
    local l, r, lst, idx;
    l = process(l[1]);
    r = process(l[2]);
    if (l[1][0] == "list_access") {
      lst = process(l[1][1]);
      idx = process(l[1][2]);
    }
    return lambda(e) {
      local res;
      res = l(e) + r(e);
      if(l[1][0] == "id") {
        core.envPut(e, l[1][1], res);
      } else if(l[1][0] == "list_access") {
        lst(e)[idx(e)] = res;
      }
      return res;
    };
  } else if(tag == "++") {
    local v, res, lst, idx;
    f = process(l[1]);
    if (l[1][0] == "list_access") {
      lst = process(l[1][1]);
      idx = process(l[1][2]);
    }
    return lambda(e) {
      local res;
      res = f(e);
      res++;
      if(l[1][0] == "id") {
        core.envPut(e, l[1][1], res);
      } else if(l[1][0] == "list_access") {
        lst(e)[idx(e)] = res;
      }
      return res;
    };
  } else if(tag == "=") {
    local left, idx, lst, f;
    left = l[1];
    f = process(l[2]);
    return lambda(e) {
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
          envPut(e, left[1], res);
          return res;
        };
      }else if(left[0]=="module_access"){
        return lambda(e) {
          local res;
          res = e;
          for(i=1;i<len(left)-1;i++){
            res = envGet(res, left[i][1]);
          }
          envPut(res, left[i][1], f(e));
        };
      }else {
        print("unknown type for =\n");
        exit(-1);
      }
    };
  }
}
