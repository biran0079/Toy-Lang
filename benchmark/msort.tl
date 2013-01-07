fun merge(l1, l2){
  res=[];
  len1 = len(l1);
  len2 = len(l2);
  i=0;
  j=0;
  while(i<len1 && j<len2){
    if(l1[i]<l2[j]){
      res += l1[i];  
      i += 1;
    }else{
      res += l2[j];
      j = j+1;
    }
  }
  while(i<len1) {
    res += l1[i];  
    i += 1;
  }
  while(j<len2) {
    res += l2[j];
    j += 1;
  }
  return res;
}
fun prefix(l, n){
  res = [];
  i=0;
  while(i<n){
    res += l[i];
    i += 1;
  }
  return res;
}
fun suffix(l,n){
  res=[];
  length = len(l);
  i=n;
  while(i<length){
    res += l[i];
    i += 1;
  }
  return res;
}
fun msort(lst){
  length = len(lst);
  if(length<=1)return;
  mid = length / 2;
  l1 = prefix(lst, mid);
  l2 = suffix(lst, mid);
  msort(l1);
  msort(l2);
  t = merge(l1, l2);
  i=0;
  while(i<length){
    lst[i] = t[i];
    i=i+1;  
  }
}
fun getRandGenerator(seed){
  fun rand(){
    seed = seed *  123;
    seed  = seed + 234;
    return seed % 1000;
  }  
  return rand;
}
fun getRandomList(n){
  rand = getRandGenerator(1);
  res=[];
  while(n>0){
    res += rand();  
    n = n - 1;
  }
  return res;
}

l = getRandomList(10000);
time(msort(l));

