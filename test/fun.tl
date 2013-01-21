fun x(a,b){
  print(a);  
  print(b);  
}
x(1,2);
fun y(a){
  if(a>0){
    print(a);  
    y(a-1);
  }  
}
y(10);
fun add(a,b){
  return a+b;  
}
print(add(3,4));

