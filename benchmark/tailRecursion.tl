fun test() {
  fun f(n){
    if(n) f(n-1);
  }

  fun sum(n, res){
    if(!n)return res;
    return sum(n-1, res+n);
  }

  f(1000000);
  sum(1000000, 0);
}
time(test());
