#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define FILE_LENGTH 2*(BUFSIZ+12)

struct mm_st {
    int written_0, pre_0, cur_0;
    int written_1, pre_1, cur_1;
    char data_0[BUFSIZ];
    char data_1[BUFSIZ];
};

static void sig_end() {  
    wait(NULL);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int child, fd;
    struct mm_st* m_area;
    char buffer[BUFSIZ] = "";
    void* file_memory;
    char string[BUFSIZ];
    
    if (argc != 2 || strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0) {
        fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
        exit(EXIT_FAILURE);
    }
    
    fd = open("chat_log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    
    if(fd < 0) {
        perror("open file failed\n");
        exit(EXIT_FAILURE);
    }
    
    lseek(fd, FILE_LENGTH+1, SEEK_SET);
    write(fd, "", 1);
    lseek(fd, 0, SEEK_SET);
    
    file_memory = mmap(0, FILE_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    
    if(file_memory == MAP_FAILED) {
        fprintf(stderr, "mmap failed\n");
        exit(EXIT_FAILURE);
    }
    
    m_area = (struct mm_st*)file_memory;
    memset(m_area, 0, FILE_LENGTH);
    m_area -> written_0 = 0;
    m_area -> written_1 = 0;
    m_area -> pre_0 = 0;
    m_area -> pre_1 = 0;
    m_area -> cur_0 = 0;
    m_area -> cur_1 = 0;

    close(fd);
    
    argv++;
    signal(SIGUSR1, sig_end);
    
    if(strcmp(*argv, "1") == 0) {
        child = fork();
        switch (child) {
            case -1:
                perror("Forking failed\n");
                exit(EXIT_FAILURE);
            case 0: // child
                while(strncmp(buffer,"end chat",8)) {
                    if(m_area -> written_1) {
                        memset(buffer, 0, BUFSIZ);
                        write(2,"User 2:", 7);  
                        int j=0;
                        for(int i = m_area -> pre_1; i < m_area -> cur_1; i++,j++) {
                            buffer[j] = m_area -> data_1[i];
                        }
                        write(1, buffer, j);                 
                        m_area -> written_1 = 0;
                    }
                }
                break;
            default: // parent
                while(strncmp(buffer, "end chat", 8)) {
                    memset(buffer, 0, BUFSIZ);
                    int nbytes = read(0, buffer, BUFSIZ);
                    if(nbytes > 1) {
                        m_area -> pre_0 = m_area -> cur_0;
                        m_area -> cur_0 += nbytes;
                        strcat(m_area -> data_0, buffer);
                        m_area -> written_0 = 1;
                    }
                }
        }
    }
    
    if(strcmp(*argv, "2") == 0) {
        child = fork();
        switch (child) {
            case -1:
                perror("Forking failed\n");
                exit(EXIT_FAILURE);
            case 0: // child
                while(strncmp(buffer,"end chat",8)) {
                    if(m_area -> written_0) {
                        memset(buffer, 0, BUFSIZ);
                        write(2,"User 1:", 7);
                        int j=0;
                        for(int i = m_area -> pre_0; i < m_area -> cur_0; i++,j++) {
                            buffer[j] = m_area -> data_0[i];
                        }
                        write(1, buffer, j);                 
                        m_area -> written_0 = 0;
                    }
                }
                break;
            default: // parent
                while(strncmp(buffer, "end chat", 8)) {
                    memset(buffer, 0, BUFSIZ);
                    int nbytes = read(0, buffer, BUFSIZ);
                    if(nbytes > 1) {
                        m_area -> pre_1 = m_area -> cur_1;
                        m_area -> cur_1 += nbytes;
                        strcat(m_area -> data_1, buffer);
                        m_area -> written_1 = 1;
                    }
                }
        }
    }
    
    if(child > 0) {
        kill(child,SIGKILL);
        wait(NULL);
    }
    else if(child == 0) {
        kill(getppid(),SIGUSR1);
    }
        
    munmap(file_memory, FILE_LENGTH);   
    exit(EXIT_SUCCESS);
}