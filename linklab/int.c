#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
int main(int argc,
char *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
      void *p =
            malloc(atoi(argv[i]));
      free(p);
    }
    return(0);
}
