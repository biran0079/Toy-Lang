a=0;
while(a<10){
  print(a);
  print(a*a);
  a=a+1;
}
print(a);
a=0;
c=0;
while(1){
  if(a==10)break;
  b=0;
  while(1){
    if(b==10)break;
    c=c+1;
    b=b+1;
  }
  a=a+1;
}
print(c);
