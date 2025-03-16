// Ahmet Sehriyaroglu 
// 2581015 

#include<unistd.h> // these 2 may not useful just look for further applications 
#include<stdio.h>

#include "game_structs.h"

#include <sys/socket.h>
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

int main() {

    printf("hello world\n");

    return 0;
}