#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MSG_SIZE 1024

int main(int argc, char *argv[]) {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[MAX_MSG_SIZE] = {0};

  if (argc != 3) {
    printf("Usage: %s <server_ip_address> <server_port>\n", argv[0]);
    return -1;
  }

  // Create socket file descriptor
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  // Set server address and port
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[2]));

  // Convert IPv4 and IPv6 addresses from text to binary form

  if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  // Connect to server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    return -1;
  }

  while (1) {
    // Wait for server response
    if (read(sock, buffer, MAX_MSG_SIZE) == 0) {
      printf("Server disconnected\n");
      break;
    }

    printf("Received message from Server: %s\n", buffer);

    memset(buffer, 0, MAX_MSG_SIZE);
  }

  // Close connection
  close(sock);

  return 0;
}
