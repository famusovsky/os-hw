/*
Реализовать для родительского и дочернего процесса двухстороннюю связь с
использованием только одного неименованного канала (pipe) и циклической
организацией обмена посредством семафора. При этом канал должен быть постоянно
открытым. Обмен должен завершаться после поочередной передачи в каждую сторону
по десять сообщений.
По завершении обмена обеспечить корректное закрытие каналов и удаление семафора.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

const int MSGS_COUNT = 10;
const int BUFSIZE = 256;
#define SEM_KEY 1234

int getSemaphore() {
  int sem_id = semget(SEM_KEY, 2, IPC_CREAT | 0666);
  if (sem_id < 0) {
    perror("Error creating semaphore: ");
    exit(EXIT_FAILURE);
  }
  semctl(sem_id, 0, SETVAL, 0);
  semctl(sem_id, 1, SETVAL, 0);
  printf("Semaphore created with id: %d\n", sem_id);
  return sem_id;
}

void eraseSemaphore(int sem_id) {
  if (semctl(sem_id, 0, IPC_RMID, NULL) < 0) {
    perror("Error removing semaphore: ");
    exit(EXIT_FAILURE);
  }
}

void parent_process(int fd, int sem_id) {
  char buffer[BUFSIZE];
  for (int i = 0; i < MSGS_COUNT; ++i) {
    semctl(sem_id, 0, SETVAL, 0);
    printf("Parent: ");
    strncpy(buffer, "Message %d from parent", BUFSIZE);
    snprintf(buffer, BUFSIZE, "Message %d from parent", i + 1);
    write(fd, buffer, BUFSIZE);

    semop(sem_id, &(struct sembuf){.sem_num = 1, .sem_op = -1, .sem_flg = 1}, 1);
    semop(sem_id, &(struct sembuf){.sem_num = 0, .sem_op = 1, .sem_flg = 1}, 1);

    read(fd, buffer, BUFSIZE);
    printf("Received: %s\n", buffer);

    semop(sem_id, &(struct sembuf){.sem_num = 1, .sem_op = -1, .sem_flg = 1}, 1);
    semop(sem_id, &(struct sembuf){.sem_num = 0, .sem_op = 1, .sem_flg = 1}, 1);
  }
}

void child_process(int fd, int sem_id) {
  char buffer[BUFSIZE];
  for (int i = 0; i < MSGS_COUNT; ++i) {
    semctl(sem_id, 1, SETVAL, 0);

    semop(sem_id, &(struct sembuf){.sem_num = 0, .sem_op = -1, .sem_flg = 0}, 1);
    read(fd, buffer, BUFSIZE);
    printf("Child: Received: %s\n", buffer);
    semop(sem_id, &(struct sembuf){.sem_num = 1, .sem_op = 1, .sem_flg = 0}, 1);

    semctl(sem_id, 0, SETVAL, 0);

    printf("Child: ");
    strncpy(buffer, "Message %d from child", BUFSIZE);
    snprintf(buffer, BUFSIZE, "Message %d from child", i + 1);
    write(fd, buffer, strlen(buffer) + 1);
    semop(sem_id, &(struct sembuf){.sem_num = 1, .sem_op = -1, .sem_flg = 0}, 1);
    semop(sem_id, &(struct sembuf){.sem_num = 0, .sem_op = 1, .sem_flg = 0}, 1);
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
