fun new(){
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

fun put(t, k, v){
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

fun get(t, k){
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

fun keys(t){
  res = []; 
  for(l : t)
    for(item : l)  
      res+=[item[0]];
  return res;
}
