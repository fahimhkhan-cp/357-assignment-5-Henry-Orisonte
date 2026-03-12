#include <stdio.h>
#include <stdlib.h>



int main (int argc, char *argv[]) {
    int portnumber;

    if (argc != 2){
        fprintf(stderr, "Usage: %s <portnumber>\n", argv[0]);
        exit(1);
    }
    
    portnumber = atoi(argv[1]);
    if (portnumber < 1024 || portnumber > 65535) {
        fprintf(stderr, "Error: Port number must be between 1024 and 65535.\n");
        exit(1);
    }

    printf("Starting server on port %d...\n", portnumber);

    return 0;
}