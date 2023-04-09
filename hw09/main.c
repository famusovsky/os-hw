#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

const int MSGS_COUNT = 10;
const int BUFSIZE = 256;
#define SEM_KEY 1918

int getSemaphore() {
  int sem_id = semget(SEM_KEY, 2, IPC_CREAT | 0666);
  if (sem_id < 0) {
    perror("Error creating semaphore: ");
    exit(EXIT_FAILURE);
  }
  int rc = semctl(sem_id, 0, SETVAL, 0);
  if (rc < 0) {
      perror("Error initializing semaphore 0: ");
      exit(EXIT_FAILURE);
  }
  rc = semctl(sem_id, 1, SETVAL, 1);
  if (rc < 0) {
      perror("Error initializing semaphore 1: ");
      exit(EXIT_FAILURE);
  }

  printf("Semaphore created with id: %d\n", sem_id);
  return sem_id;
}

void eraseSemaphore(int sem_id) {
  if (semctl(sem_id, 0, IPC_RMID, NULL) < 0) {
    perror("Error removing semaphore: ");
    exit(EXIT_FAILURE);
  }
}

void runSemOp(int sem_id, int sem_num, int sem_op, int sem_flg) {
  struct sembuf sb;
  sb.sem_num = sem_num;
  sb.sem_op = sem_op;
  sb.sem_flg = sem_flg;
  if (semop(sem_id, &sb, 1) < 0) {
    perror("Error running semop: ");
    exit(EXIT_FAILURE);
  }
}

void parent_process(int fd, int sem_id) {
  char buffer[BUFSIZE];
  printf("Parent: ");
  for (int i = 0; i < MSGS_COUNT; ++i) {
    runSemOp(sem_id, 0, 0, SEM_UNDO);

    printf("Parent: ");
    snprintf(buffer, BUFSIZE, "Message %d from parent", i + 1);
    write(fd, buffer, BUFSIZE);
    runSemOp(sem_id, 0, 1, 0);
    runSemOp(sem_id, 1, -1, 0);

    runSemOp(sem_id, 0, 0, SEM_UNDO);

    read(fd, buffer, BUFSIZE);
    printf("Parent: Received: %s\n", buffer);
    runSemOp(sem_id, 0, 1, 0);
    runSemOp(sem_id, 1, -1, 0);
  }
}

void child_process(int fd, int sem_id) {
  char buffer[BUFSIZE];
  printf("Child: ");
  for (int i = 0; i < MSGS_COUNT; ++i) {
    runSemOp(sem_id, 1, 0, SEM_UNDO);

    read(fd, buffer, BUFSIZE);
    printf("Child: Received: %s\n", buffer);
    runSemOp(sem_id, 1, 1, 0);
    runSemOp(sem_id, 0, -1, 0);
    
    semop(sem_id, &(struct sembuf){.sem_num = 1, .sem_op = 0, .sem_flg = SEM_UNDO}, 1);

    printf("Child: ");
    snprintf(buffer, BUFSIZE, "Message %d from child", i + 1);
    write(fd, buffer, BUFSIZE);
    runSemOp(sem_id, 1, 1, 0);
    runSemOp(sem_id, 0, -1, 0);
  }
}


int main() {
  int fd[2];
  int sem_id;

  if (pipe(fd) == -1) {
    perror("Error creating pipe\n");
    exit(EXIT_FAILURE);
  }

  sem_id = getSemaphore();
  semop(sem_id, &(struct sembuf){.sem_num = 0, .sem_op = 0, .sem_flg = 0}, 1);
  semop(sem_id, &(struct sembuf){.sem_num = 1, .sem_op = 1, .sem_flg = 0}, 1);

  pid_t pid;
  if ((pid = fork()) < 0) {
    perror("Error creating child process\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    close(fd[1]);
    child_process(fd[0], sem_id);
    close(fd[0]);
    exit(EXIT_SUCCESS);
  }

  close(fd[0]);
  parent_process(fd[1], sem_id);
  close(fd[1]);

  eraseSemaphore(sem_id);

  return 0;
}