#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void handleSigInt(int sig) {}

void sendInt(int number, pid_t getter_pid) {
  for (int i = 0; i < 32; i++) {
    int bit = (number >> i) & 1;

    if (kill(getter_pid, bit ? SIGUSR1 : SIGUSR2) == -1) {
      printf("Error sending SIGUSR%d to %d\n", bit ? 1 : 2, getter_pid);
      exit(1);
    }
    pause();
  }
}

int main() {
  signal(SIGINT, handleSigInt);

  printf("This program\'s pid is %d\n", getpid());

  pid_t getter_pid;
  printf("Enter the pid of the getter: ");
  scanf("%d", &getter_pid);

  int number;
  printf("Enter a number: ");
  scanf("%d", &number);
  sendInt(number, getter_pid);

  printf("Sent %d to %d\n", number, getter_pid);

  return 0;
}
