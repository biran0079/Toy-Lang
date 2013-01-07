fun f(n){
  fun g1(){
    local n;
    n=123;
    return n;
  }
  fun g2(){
    n++;
    return n;
  }
  return [g1,g2];
}
l = f(0);
g1 = l[0];
g2 = l[1];
print(g1(),g2(),g2(),g1(),g2());

