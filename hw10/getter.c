#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int bit = 0;

void handleSigUsr1(int sig) { bit = 1; }

void handleSigUsr2(int sig) { bit = 0; }

void sendResponse(pid_t pid) {
  if (kill(pid, SIGINT) == -1) {
    printf("Error sending SIGINT to %d\n", pid);
    exit(1);
  }
}

int getNumber(pid_t sender_pid) {
  int number = 0;
  printf("Binary representation of the number: ");

  for (int i = 0; i < 32; i++) {
    pause();
    printf("%d", bit);
    number |= bit << i;

    sendResponse(sender_pid);
  }
  printf("\n");
  return number;
}

int main() {
  signal(SIGUSR1, handleSigUsr1);
  signal(SIGUSR2, handleSigUsr2);

  printf("This program\'s pid is %d\n", getpid());

  pid_t sender_pid;
  printf("Enter the pid of the sender: ");
  scanf("%d", &sender_pid);

  int number = getNumber(sender_pid);
  printf("Received %d from %d\n", number, sender_pid);

  return 0;
}