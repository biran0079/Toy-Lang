fun merge(lst, l, mid, r){
  res=[];
  i=l;
  j=mid+1;
  while(i<=mid && j<=r){
    if(lst[i]<lst[j]){
      res += lst[i];  
      i++;
    }else{
      res += lst[j];
      j++;
    }
  }
  while(i<=mid) {
    res += lst[i];  
    i++;
  }
  while(j<=r) {
    res += lst[j];
    j++;
  }
  return res;
}

fun msortInternal(lst, l, r) {
  if(r <= l)return;
  mid = (l+r)/2;
  msortInternal(lst, l, mid);
  msortInternal(lst, mid+1, r);
  t = merge(lst, l, mid, r);
  for(i=l;i<=r;i++)
    lst[i] = t[i-l];
}

fun msort(lst){
  msortInternal(lst, 0, len(lst) - 1);
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

l = getRandomList(50000);
msort(l);

