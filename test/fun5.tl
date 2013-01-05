fun isPrime(n){
  i=2;
  while(i*i<=n){
    if(n%i==0)  return 0;
    i=i+1;
  }
  return 1;
}
fun main(){
  i=2;
  while(i<20){
    if(isPrime(i))
      print(i);
    i=i+1;
  }
}
main();
fun f(){}
fun g(){return f();}
print(g());
