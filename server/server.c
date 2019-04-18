#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){
    //Defining a variable to hold the command line argument
    char* listening_port;

    //Catch improper argument number
    if(argc != 2){
        printf("Unrecognized argument. Please use the following format:\n./server <listening_port>\n");
        return 1;
    }

    //Allocate space for the command line argument
    listening_port = (char*)malloc(strlen(argv[1]) + 1);

    //Copy the command line argument into a variable
    strcpy(listening_port, argv[1]);

    //Printing command line argument for proof
    printf("Listening Port: %s\n", listening_port);
}
