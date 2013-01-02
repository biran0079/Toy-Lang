fun f(){
  a=0;
  fun g(){
    a = a+1;  
    return a;
  }
  return g;
}
x = f();
print(x());
print(x());
print(x());
print(x());
