F = lambda () {
  fun new(a) {
    return this; 
  }
  fun getA(self){
    return self.a;
  }
  fun setA(self,v){
    print(self.a);
    self.a = v; 
    print(self.a);
  }
  return this;
};
F = F();
a = F.new(1);
print(a.getA(a));
a.setA(a, 2);
print(a.getA(a));
