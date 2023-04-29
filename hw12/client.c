#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdio.h>      // for printf() and fprintf()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <sys/socket.h> // for socket(), connect(), sendto(), and recvfrom()
#include <unistd.h>     // for close()

#define MAXRECVSTRING 255 // Longest string to receive

int main(int argc, char *argv[]) {
  int sock;                            // Socket
  struct sockaddr_in broadcast_addr;   // Broadcast Address
  unsigned short broadcast_port;       // Port
  char recv_string[MAXRECVSTRING + 1]; // Buffer for received string
  int recv_string_len;                 // Length of received string

  if (argc != 2) { // Test for correct number of arguments
    fprintf(stderr, "Usage: %s <Broadcast Port>\n", argv[0]);
    exit(1);
  }

  broadcast_port = atoi(argv[1]); // First arg: broadcast port

  // Create a best-effort datagram socket using UDP
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket() failed");
    exit(1);
  }

  // Construct bind structure
  memset(&broadcast_addr, 0, sizeof(broadcast_addr)); // Zero out structure
  broadcast_addr.sin_family = AF_INET;                // Internet address family
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  broadcast_addr.sin_port = htons(broadcast_port);    // Broadcast port

  // Bind to the broadcast port
  if (bind(sock, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) <
      0) {
    perror("bind() failed");
    exit(1);
  }

  while (1) {
    // Receive a single datagram from the server
    if ((recv_string_len =
             recvfrom(sock, recv_string, MAXRECVSTRING, 0, NULL, 0)) < 0) {
      perror("recvfrom() failed");
      exit(1);
    }

    // exit if "The End" is received
    if (strcmp(recv_string, "The End\n") == 0) {
      printf("Closing connection...\n");
      break;
    }

    recv_string[recv_string_len] = '\0';
    printf("Received: %s\n", recv_string); // Print the received string
  }

  close(sock);
  exit(0);
}
