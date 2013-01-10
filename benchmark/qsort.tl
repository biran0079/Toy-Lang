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

fun swap(a,i,j){
  t=a[i];
  a[i]=a[j];
  a[j]=t;
}

fun internalQsort(a, left, right){
  if(left>=right)return;
  i=left;
  j=left+1;
  k=j;
  while(j<=right){
    if(a[j]<a[i]) {
      swap(a,k,j);
      k++;
    }
    j++;
  }
  k=k-1;
  swap(a, left, k);
  internalQsort(a,left,k-1);
  internalQsort(a,k+1,right);
}

fun qsort(l){
  internalQsort(l,0,len(l)-1);  
}

l = getRandomList(50000);
time(qsort(l));

