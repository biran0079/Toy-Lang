fun new(hash, eq){
  a = [];
  for(i=0;i<89;i+=1) 
    a+=[[]];
  return [hash, eq, a];
}

fun newStringHashTable() {
  eq = lambda (a,b) {return a==b;};
  return new(stringHash, eq); 
}

fun stringHash(s){
  res = 0;  
  l=len(s);
  for(i=0;i<l;i+=1){
    res=res*91;
    res+=ord(s[i]);
  }
  return res;
}

fun put(table, k, v){
  local hash,eq,t;
  hash=table[0];
  eq=table[1];
  t = table[2];
  size = len(t);
  i = hash(k) % size;
  if(i<0) i += size;
  l = t[i];
  llen = len(l);
  for(i=0;i<llen;i+=1){
    if(eq(l[i][0], k)) {
      l[i][1]=v;  
      return;
    }
  }
  l+=[[k,v]];
}

fun get(table, k){
  local hash,eq,t;
  hash=table[0];
  eq=table[1];
  t = table[2];
  size = len(t);
  i = hash(k) % size;
  if(i<0) i += size;
  l = t[i];
  llen = len(l);
  for(i=0;i<llen;i+=1)
    if(eq(l[i][0], k))
      return l[i][1];
  return;
}

fun keys(table){
  local t;
  t = table[2];
  res = []; 
  for(l : t)
    for(item : l)  
      res+=[item[0]];
  return res;
}
