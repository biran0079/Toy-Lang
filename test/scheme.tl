fun cons(a,b){
  fun f(i) {
    if(i==0)  return a;
    return b;
  }
  return f;
}
fun null() {}
fun car(p){
  return p(0);  
}

fun cdr(p){
  return p(1);  
}

fun slistToList(l){
  if(l==null)  return [];
  return [car(l)] + slistToList(cdr(l));
}

fun range(n){
  res=null;
  while(n>0){
    res = cons(n, res);
    n=n-1;
  }
  return res;
}

fun map(f, l){
  if(l==null)return null;
  return cons(f(car(l)) , map(f,cdr(l)));
}
fun square(n){
  return n*n;  
}
fun filter(f, l){
  if(l==null) return null;
  if(f(car(l)))  return cons(car(l), filter(f, cdr(l)));
  return filter(f,cdr(l));
}
fun isOdd(n){
  return n%2==1;  
}
fun sieve(l){
  if(l==null)return null;
  i=car(l);
  return cons(i, sieve(filter(lambda(n){return n%i!=0;},cdr(l))));
}
fun flat(l){
  if(l==null) return null;
  return append(car(l), flat(cdr(l)));
}
fun append(l1, l2){
  if(l1==null)  return l2;
  return cons(car(l1), append(cdr(l1), l2));
}
print(slistToList(map(square, range(10))));
print(slistToList(filter(isOdd, range(10))));
print(slistToList(sieve(cdr(range(20)))));
print(slistToList(append(range(4), range(5))));
print(slistToList(flat(cons(range(4), cons(range(5), null)))));
fun perm(l){
  if(l==null)  return cons(null, null);
  return flat(
      map(lambda(x){
              return map(lambda(l){ return cons(x, l); },
                  perm(filter(lambda(y) {return y!=x;}, l)));
        }, l));
}

fun printSlistOfSlist(l){
  if(l==null) return null;
  print(slistToList(car(l)));
  return printSlistOfSlist(cdr(l));
}

printSlistOfSlist(perm(range(3)));
