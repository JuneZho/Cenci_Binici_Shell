#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h>

int main() 
{ 
    int x =1 ;
    for (;;){
        printf("Sleeping: %d\n", x);
        sleep(1); 
        x++;
    }

    return 0; 
}