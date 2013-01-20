fun prefix(l, n){
  local res, i;
  res = [];
  i=0;
  while(i<n){
    res += [l[i]];
    i ++;
  }
  return res;
}
fun suffix(l,n){
  local res,length, i;
  res=[];
  length = len(l);
  i=n;
  while(i<length){
    res += [l[i]];
    i ++;
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

fun map(f, l){
  res = [];
  for(i : l)
    res+=[f(i)];
  return res;
}
