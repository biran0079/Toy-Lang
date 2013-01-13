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


fun swap(a,i,j){
  t=a[i];
  a[i]=a[j];
  a[j]=t;
}

fun isSorted(l){
  length = len(l);
  i=1;
  while(i<length){
    if(l[i-1] > l[i])
      return 0;
    i=i+1;  
  }
  return 1;
}
