#include "global_test.hpp"

int main() {
  printf("hello, i am main!\n");
  char* a = new char(100);
  for (int i = 0; i < 100; i++) {
    printf("%c \n", a[i]);
  }
  delete a;
  for (int i = 1; i < 100; i++) {
    printf("%c \n", a[i]);
  }
  return 0;
}