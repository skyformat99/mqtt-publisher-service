#include <stdio.h>
#include <stdlib.h>

#include "provenancelib.h"

int main( void ){
  int a;
  provenance_set_tracked(true);
  scanf("%d", &a);
  printf("Hello World\n");
  printf("%d", a);
}
