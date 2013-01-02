b=1;
fun f(){
  a=0;
  b=2;
  fun g(){
    a = a+1;  
    b = b+1;
    return a;
  }
  return g;
}
x = f();
print(x());
print(x());
print(x());
print(x());
print(b);
