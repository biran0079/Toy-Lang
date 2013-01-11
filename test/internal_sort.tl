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

l = getRandomList(100);
print(isSorted(l));
sort(l);
print(isSorted(l));

