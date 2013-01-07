fun f(n){
  if(n) f(n-1);
}
print(f(50000));

fun sum2(n, res){
  if(!n)return res;
  return sum2(n-1, res+n);
}
print(sum2(50000,0));
