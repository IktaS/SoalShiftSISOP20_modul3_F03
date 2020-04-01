#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define POKEZONE_PORT 8080
#define SHOP_PORT 8090

int connect_socket(uint16_t _PORT){
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(_PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    return sock;
}

int main(int argc, char const *argv[]){
    int sock = connect_socket(POKEZONE_PORT);

    char *hello = "shop | lull_pow";
    send(sock,hello, strlen(hello),0);

    char buffer[2048] = {0};

    int valread = read(sock,buffer,2048);
    printf("%s\n",buffer);
    while(1){}
}