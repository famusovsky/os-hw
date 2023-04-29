#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXBUFFER 255

void dieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

int main(int argc, char *argv[]) {
  int sock;                          // Socket
  struct sockaddr_in broadcast_addr; // Broadcast address
  char *broadcast_ip;                // IP broadcast address
  unsigned short broadcast_port;     // Server port
  int broadcast_permission; // Socket opt to set permission to broadcast

  if (argc < 3) {
    fprintf(stderr, "Usage:  %s <IP Address> <Port>\n", argv[0]);
    exit(1);
  }

  broadcast_ip = argv[1];         // First arg:  broadcast IP address
  broadcast_port = atoi(argv[2]); // Second arg:  broadcast port

  // Create socket for sending/receiving datagrams
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    dieWithError("socket() failed");
  }

  // Set socket to allow broadcast
  broadcast_permission = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcast_permission,
                 sizeof(broadcast_permission)) < 0) {
    dieWithError("setsockopt() failed");
  }

  // Construct local address structure
  memset(&broadcast_addr, 0, sizeof(broadcast_addr)); // Zero out structure
  broadcast_addr.sin_family = AF_INET;                // Internet address family
  broadcast_addr.sin_addr.s_addr =
      inet_addr(broadcast_ip);                     // Broadcast IP address
  broadcast_addr.sin_port = htons(broadcast_port); // Broadcast port

  printf("Input \'The End\' to finish broadcasting\n");
  printf("Input a message to broadcast: ");

  char buffer[MAXBUFFER];
  while (fgets(buffer, MAXBUFFER, stdin)) {
    printf("\n");

    // Broadcast buffer in datagram to clients every 3 seconds
    if (sendto(sock, buffer, MAXBUFFER, 0, (struct sockaddr *)&broadcast_addr,
               sizeof(broadcast_addr)) != MAXBUFFER) {
      dieWithError("sendto() sent a different number of bytes than expected");
    }

    // Check if message is "The End"
    if (strcmp(buffer, "The End\n") == 0) {
      printf("Closing connection...\n");
      break;
    }

    memset(buffer, 0, MAXBUFFER);
    printf("Input a message to broadcast: ");
  }

  close(sock);
  exit(0);
}
