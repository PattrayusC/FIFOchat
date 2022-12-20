#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO_1 "/tmp/fifo1to2"
#define FIFO_2 "/tmp/fifo2to1"
#define MAX_RBUF 80

int FIFO_FD1, FIFO_FD2;

int main(int argc, char *argv[]) {
  int child, n_bytes;
  char rbuf[MAX_RBUF] = "";
  //memset(rbuf, '\0', sizeof(rbuf));
  if(argc != 2 || strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0) {
    fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
    exit(EXIT_FAILURE);
  }
  if(access(FIFO_1, F_OK) == -1) {
    FIFO_FD1 = mkfifo(FIFO_1, 0777);
    if(FIFO_FD1) {
      fprintf(stderr, "Could not create fifo %s\n", FIFO_1);
      exit(EXIT_FAILURE);
    }
  }
  if(access(FIFO_2, F_OK) == -1) {
    FIFO_FD2 = mkfifo(FIFO_2, 0777);
    if(FIFO_FD2) {
      fprintf(stderr, "Could not create fifo %s\n", FIFO_2);
      exit(EXIT_FAILURE);
    }
  }
  argv++;
  if(strcmp(*argv, "1") == 0) {
    FIFO_FD1 = open(FIFO_1, O_WRONLY);
    FIFO_FD2 = open(FIFO_2, O_RDONLY);
    child = fork();
    switch(child) {
      case -1 :
        perror("Forking failed");
        exit(EXIT_FAILURE);
      case 0 ://child
        while(strncmp(rbuf, "end chat", 8)) {
          while ((n_bytes = read(FIFO_FD2, rbuf, sizeof(rbuf))) > 0) {//อ่านจากท่อFD2
            write(1, rbuf, n_bytes);//เขียนลงจอ
          }
        }
        break;
      default ://parent
        while(strncmp(rbuf, "end chat", 8)) {
          while ((n_bytes = read(0, rbuf, sizeof(rbuf))) > 0) {//อ่านinput
            write(FIFO_FD1, rbuf, n_bytes);//เขียนลงท่อFD1
          }
        }
    }
  }
  if(strcmp(*argv, "2") == 0) {
    FIFO_FD1 = open(FIFO_1, O_RDONLY);
    FIFO_FD2 = open(FIFO_2, O_WRONLY);
    child = fork();
    switch(child) {
      case -1 :
        perror("Forking failed");
        exit(EXIT_FAILURE);
      case 0 :
        while(strncmp(rbuf, "end chat", 8)) {
          while ((n_bytes = read(FIFO_FD1, rbuf, sizeof(rbuf))) > 0) {//อ่านจากท่อFD1
            write(1, rbuf, n_bytes);//เขียนลงจอ
          }
        }
        break;
      default :
        while(strncmp(rbuf, "end chat", 8)) {
          while ((n_bytes = read(0, rbuf, sizeof(rbuf))) > 0) {//อ่านinput
            write(FIFO_FD2, rbuf, n_bytes);//เขียนลงท่อFD2
          }
        }
    }
  }

  if(FIFO_FD1 != -1) {
    close(FIFO_FD1);
  }
  if(FIFO_FD2 != -1) {
    close(FIFO_FD2);
  }
  exit(EXIT_SUCCESS);
}