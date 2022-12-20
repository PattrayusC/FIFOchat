#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

static void sig_end() {
  wait(NULL);
  exit(EXIT_SUCCESS);
}

struct my_msg {
    long int msg_type;
    char cdata[BUFSIZ];
};

int main(int argc, char *argv[]) {
  int msgID , child , nbytes;
  struct my_msg a_msg;
  char buffer[BUFSIZ] = "";

  if (argc != 2 || strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0) {
    fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
    exit(EXIT_FAILURE);
  }

  msgID = msgget((key_t) 6313179 , 0666 | IPC_CREAT);
  if(msgID == -1) {
    perror("msgget failed\n");
    exit(EXIT_FAILURE);
  }

  signal(SIGUSR1, sig_end);
  argv++;

  if (strcmp(*argv, "1") == 0) {
    child = fork();
    switch (child) {
    case -1:
      perror("Forking failed\n");
      exit(EXIT_FAILURE);
    case 0: // child
      while (strncmp(a_msg.cdata, "end chat", 8)) {
        if ((nbytes = msgrcv(msgID, (void *)&a_msg, BUFSIZ, 2, 0)) > 0) {
          write(1, a_msg.cdata, nbytes);
        }
      }
      if(msgctl(msgID, IPC_RMID, 0) == -1) {
        fprintf(stderr, "msgctl failed\n");
        exit(EXIT_FAILURE);
      }
      break;
    default: // parent
      while (strncmp(buffer, "end chat", 8)) {
        if ((nbytes = read(0, buffer, BUFSIZ)) > 0) {
          a_msg.msg_type = 1;
          strcpy(a_msg.cdata, buffer);
          if (msgsnd(msgID, (void *)&a_msg, nbytes, 0) == -1) {
            fprintf(stderr, "msgsnd failed\n");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  }

  if (strcmp(*argv, "2") == 0) {
    child = fork();
    switch (child) {
    case -1:
      perror("Forking failed\n");
      exit(EXIT_FAILURE);
    case 0: // child
      while (strncmp(a_msg.cdata, "end chat", 8)) {
        if ((nbytes = msgrcv(msgID, (void *)&a_msg, BUFSIZ, 1, 0)) > 0) {
          write(1, a_msg.cdata, nbytes);
        }
      }
      if(msgctl(msgID, IPC_RMID, 0) == -1) {
        fprintf(stderr, "msgctl failed\n");
        exit(EXIT_FAILURE);
      }
      break;
    default: // parent
      while (strncmp(buffer, "end chat", 8)) {
        if ((nbytes = read(0, buffer, BUFSIZ)) > 0) {
          a_msg.msg_type = 2;
          strcpy(a_msg.cdata, buffer);
          if (msgsnd(msgID, (void *)&a_msg, nbytes, 0) == -1) {
            fprintf(stderr, "msgsnd failed\n");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  }

  if(child > 0) {
    kill(child, SIGUSR1);
    wait(NULL);
  }
  else if(child == 0) {
    kill(getppid(), SIGUSR1);
  }

  exit(EXIT_SUCCESS);
}