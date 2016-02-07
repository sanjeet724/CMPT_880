#include <stdio.h>

void a() {}

void b() {}

void c() {a();}

void d() {a();}

int
main(int argc, char **argv) {
  a();
  b();
  c();
  d();
  return 0;
}


