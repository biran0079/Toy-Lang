apply(print, []);
apply(print, [1,2,3]);
a = [3,4,5];
apply(print, a);
fun f(x, y) {
  print(x,y);
}
apply(f, [1, 2]);
apply(apply, [f, [3, 4]]);

fun f(x, y) {
  fun g(z) {
    print(x, y, z);
  }
  return g;
}
apply(f(1, 2), [3]);
