#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define BUF_LENGTH 1000
#define PORT_NUMBER 8082
#define MAX_CONNECTIONS 10

#define PTHREAD

void* sendfile(void* param) {
  int clientSocket = (int) param;
  int file;
  ssize_t bytesSent = 1;
  off_t offset = 0;
  char buf[BUF_LENGTH];
  ssize_t bytesRead;

  recv(clientSocket, buf, BUF_LENGTH, 0);

  file = open(buf, O_RDONLY);
  if (file < 0) {
    printf("Error while opening file\n");
  }

  do {
    bytesRead = read(file, buf, BUF_LENGTH);
    bytesSent = write(clientSocket, buf, bytesRead);
  } while (bytesSent > 0);
  close(file);
  shutdown(clientSocket, SHUT_RDWR);
}

int main()
{
    int serverSocket;
    struct sockaddr_in addr;
    socklen_t addrSize;
    int optval = 1;
    int status;
    char buf[BUF_LENGTH];

    memset(buf, 0, BUF_LENGTH * sizeof(char));

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printf("Error while creating socket\n");
        return 1;
    }
    printf("Socket created successfully\n");

    status = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (status != 0) {
        printf("Error while setting socket options\n");
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUMBER);
    inet_aton("127.0.0.1", &addr.sin_addr.s_addr);

    status = bind(serverSocket, (struct sockaddr*)(&addr), sizeof(addr));
    if (status != 0) {
        printf("Error while binding\n");
        return 1;
    }
    printf("Bind successfully\n");

    status = listen(serverSocket, MAX_CONNECTIONS);
    if (status != 0) {
        printf("Error while listening\n");
        return 1;
    }
    printf("Server started...\n");

    while (1) {
        int clientSocket;
        struct sockaddr_in clientSocketAddr;

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientSocketAddr, &addrSize);
        if (clientSocket < 0) {
            printf("Error while accpeting\n");
            return 1;
        }

        int rc;
        pthread_t clientThread;
        #ifdef PTHREAD
        rc = pthread_create(&clientThread, NULL, sendfile, (void*) clientSocket);
        if (rc){
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          return -1;
        }
        #else
        pid_t childPid = fork();
        if (childPid == 0){
          sendfile((void*)clientSocket);
          exit(0);
        }
        #endif
      sendfile(buf);
    }

    shutdown(serverSocket, SHUT_RDWR);
    return 0;
}
