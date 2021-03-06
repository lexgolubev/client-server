#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_LENGTH 1000

int main(int argc, char* argv[])
{
    int socketfd;
    struct addrinfo host_info_hint;
    struct addrinfo *host_info_list;
    char port[] = "8082";
    int file;
    char fileName[BUF_LENGTH];
    int status;
    char buf[BUF_LENGTH];
    ssize_t bytesRecivied = 1;

    if (argc != 3) {
        return -1;
    }

    memset(buf, 0, sizeof(char) * BUF_LENGTH);
    memset(&host_info_hint, 0, sizeof(host_info_hint));

    host_info_hint.ai_family = AF_INET;
    host_info_hint.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(argv[1], port, &host_info_hint, &host_info_list);
    if (status != 0) {
        printf("Error while getting address info\n");
        return 1;
    }

    socketfd = socket(host_info_list->ai_family,
                      host_info_list->ai_socktype,
                      host_info_list->ai_protocol);
    if (socketfd == -1) {
        printf("Error while create socket\n");
        return 1;
    }
    printf("Socket created successfully\n");

    status = connect(socketfd,
                     host_info_list->ai_addr,
                     host_info_list->ai_addrlen);
    if (status == -1) {
        printf("Error while connecting\n");
        return 1;
    }
    printf("Connected successfully\n");

    send(socketfd, argv[2], strlen(argv[2]), 0);

    sprintf(fileName, "%s.out", argv[2]);
    file = open(fileName, O_WRONLY | O_CREAT);
    if (file < 0) {
        printf("File not opened\n");
        return 1;
    }

    printf("Transferring file from server...\n");
    do {
        bytesRecivied = read(socketfd, buf, BUF_LENGTH);
        write(file, buf, bytesRecivied);
    } while (bytesRecivied > 0);
    printf("File transferred succesfully\n");

    close(file);
    shutdown(socketfd, SHUT_RDWR);

    return 0;
}

