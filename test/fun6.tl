fun main() {
  fun f() {
    print("in main");
  }
  fun g() {
    fun f() {
      print("in g");
    }
    f();
  }
  f();
  g();
  f();
  fun f() {
    print ("in main 2");
  }
}

main();

fun f() {
  print("out main");
}

f();
main();
f();

