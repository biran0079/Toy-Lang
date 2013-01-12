a = "hello world";
b = "";
l = len(a);
for(i=0;i<l;i++){
  b+=chr(ord(a[i]));  
}
print(b);
