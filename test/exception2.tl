try{
  try{
    try {
      throw 123;  
    } catch (e) {
      print("inner catch:",e);
      throw e;
    } finally {
      print("inner finally");
    }
  } catch(e2) {
    print("outer catch:", e2);
  } finally {
    print("outer finally");
  }
}catch(e){
  print("should not reach here");
}finally{
  print("last finally");  
}
