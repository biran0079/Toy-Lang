fun getRandomList(n){
  res=[];
  while(n>0){
    res += rand();  
    n = n - 1;
  }
  return res;
}


import list;

fun internalQsort(a, left, right){
  if(left>=right)return;
  i=left;
  j=left+1;
  k=j;
  while(j<=right){
    if(a[j]<a[i]) {
      list.swap(a,k,j);
      k=k+1;
    }
    j=j+1;  
  }
  k=k-1;
  list.swap(a, left, k);
  internalQsort(a,left,k-1);
  internalQsort(a,k+1,right);
}
fun qsort(l){
  internalQsort(l,0,len(l)-1);  
}


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
fun msort(lst){
  length = len(lst);
  if(length<=1)return;
  mid = length / 2;
  l1 = list.prefix(lst, mid);
  l2 = list.suffix(lst, mid);
  msort(l1);
  msort(l2);
  t = merge(l1, l2);
  i=0;
  while(i<length){
    lst[i] = t[i];
    i=i+1;  
  }
}

