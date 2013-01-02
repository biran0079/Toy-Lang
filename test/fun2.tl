a=3;
fun f(){
  a=1;
  print(a);
  g();
  print(a);
}
fun g(){
  print(a);
  a=2;
  print(a);
}
f();
print(b);
