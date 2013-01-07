fun f(n){
  try{
    try{
      if(n==10) throw 5;
      if(n%5==0) throw 0;
      if(n%5==1) throw 1;
      if(n%5==2) throw 2;
      if(n%5==3) throw 3;
      if(n%5==4) throw 4;
    }catch(e0){
      if(e0==0) print("e0:", e0);
      else throw e0 ;
    }
  }catch(e1){
    if(e1==1) print("e1:", e1);
    else throw e1;
  }
}

fun g0(n){
  try{
    f(n);
  }catch(e2){
    if(e2==2) print("e2:", e2);  
    else throw e2;
  }
}

fun g1(n){
  try{
    g0(n);  
  }catch(e3){
    if(e3==3) print("e3:", e3);  
    else throw e3;
  }
}

fun g2(n){
  try{
    g1(n);  
  }catch(e4){
    if(e4==4) print("e4:", e4);  
    else throw e4;
  }
}
for(i=0;i<=10;i++){
  try{
    g2(i);
  }catch(e5){
    print("e5:", e5);
  }
}
