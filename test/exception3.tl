for(i=0;i<10;i++){
  try{
    if(i%2==0)throw 0;
    if(i==9)throw 1;
  }catch(e){
    if(e==0)continue;
    if(e==1)break;
  }
  print(i);
}
