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
