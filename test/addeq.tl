a=1;
b=a;
a+=1;
print(a);
print(b);
a=[];
b=a;
a+=[2];
print(a);
print(b);
a=a+[3];
print(a);
print(b);
a[0]+=1;
print(a);
c=[a];
c+=[b];
print(c);
c[1]+=[4];
print(c);
print(b);
c+=[1,2];
print(c);
