fun newHashTable(){
  a = [];
  for(i=0;i<89;i+=1) 
    a+=[[]];
  return a;
}
fun hash(s){
  res = 0;  
  l=len(s);
  for(i=0;i<l;i+=1){
    res=res*89;
    res+=ord(s[i]);
  }
  return res;
}
fun hashTablePut(t, k, v){
  size = len(t);
  i = hash(k) % size;
  if(i<0) i += size;
  l = t[i];
  llen = len(l);
  for(i=0;i<llen;i+=1){
    if(l[i][0] == k) {
      l[i][1]=v;  
      return;
    }
  }
  l+=[[k,v]];
}

fun hashTableGet(t, k){
  size = len(t);
  i = hash(k) % size;
  if(i<0) i += size;
  l = t[i];
  llen = len(l);
  for(i=0;i<llen;i+=1)
    if(l[i][0] == k) 
      return l[i][1];
  return;
}

fun hashTableKeys(t){
  res = []; 
  for(l : t)
    for(item : l)  
      res+=[item[0]];
  return res;
}


s = "hello this is biran hello again this is biran again";
fun substr (s,i,l){
  res =  "";
  for(j=0;j<l;j+=1)
    res+=s[i+j];
  return res;
}
fun split(s){
  res=[];
  l = len(s);
  i=0;
  while(1){
    while(i<l && s[i]==" ") i+=1;
    if(i==l)break;
    j=i;
    while(j<l && s[j]!= " "){
      j+=1;
    }
    res+=substr(s,i,j-i);
    i=j;
  }
  return res;
}
t = newHashTable();
for(w : split(s)){
  n = hashTableGet(t, w);
  if(n==none)
    hashTablePut(t, w, 1);
  else
    hashTablePut(t,w,n+1);
}
for(w : hashTableKeys(t)){
  print(w, hashTableGet(t, w));
}
