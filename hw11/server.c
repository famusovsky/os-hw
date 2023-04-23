#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_MSG_SIZE 1024
#define DEFAULT_PORT 8888

int main(int argc, char *argv[]) {
  int server_fd;
  int client_fd[2];
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr[2];
  char buffer[MAX_MSG_SIZE] = {0};
  int valread;

  // Create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Set server address and port
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(DEFAULT_PORT);

  // Bind socket to address and port
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  printf("Server\'s ip address: %s\n", inet_ntoa(server_addr.sin_addr));
  printf("Server\'s port: %d\n", ntohs(server_addr.sin_port));

  // Listen for incoming connections
  if (listen(server_fd, 3) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server started. Waiting for clients...\n");

  for (int i = 0; i < 2; i++) {
    // Accept incoming connection
    if ((client_fd[i] = accept(server_fd, (struct sockaddr *)&client_addr[i],
                               (socklen_t *)&client_addr[i])) < 0) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }
    printf("Client %d connected: %s:%d\n", i + 1,
           inet_ntoa(client_addr[i].sin_addr), ntohs(client_addr[i].sin_port));
  }

  // Wait for incoming messages
  while (1) {
    // Wait for first client message
    valread = read(client_fd[0], buffer, MAX_MSG_SIZE);
    printf("Received message from the first Client: %s\n", buffer);

    // Check if message is "The End"
    if (strcmp(buffer, "The End\n") == 0) {
      printf("Closing connection...\n");
      break;
    }

    // Send message to the second Client
    send(client_fd[1], buffer, strlen(buffer), 0);
    memset(buffer, 0, MAX_MSG_SIZE);
  }

  // Close connection
  for (int i = 0; i < 2; ++i) {
    close(client_fd[i]);
  }
  close(server_fd);

  printf("Server stopped.\n");
  return 0;
}
