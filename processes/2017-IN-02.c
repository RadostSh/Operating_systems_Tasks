#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdio.h>

pid_t fork_wrapper(void){
    pid_t pid = fork();
    if(pid == -1){
        err(2, "fork");
    }
    return pid;
}

void wait_wrapper(void){
    int status;
    if(wait(&status) == -1){
        err(4, "wait");
    }

    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(5, "child exited with status %d", WEXITSTATUS(status));
        }
    } else {
        errx(6, "child was killed");
    }
}

ssize_t read_wrapper(int fd, void* buff, size_t bytes){
    int res = read(fd, buff, bytes);
    if(res == -1){
        err(9, "read");
    }
    return res;
}

bool readArgs(char arg1[], char arg2[]){
    int ind = 0, spaceCount = 0;
    
    for(int i = 0; i < 10; i++){

        if (ind >= 5){
            errx(10, "Len of arg must be 4");
        }
        
        char ch;
        int res;
        res = read_wrapper(0, &ch, sizeof(ch));

        if(res == 0){
            return false;
        }

        if(ch == 0x20){
            spaceCount++;
            arg1[ind] = '\0';
            ind = 0;
            continue;
        } else if(ch == 0x0A){
            if(spaceCount == 0){
                arg1[ind] = '\0';
            } else if(spaceCount == 1){
                arg2[ind] = '\0';
            }
            break;
        }

        if(spaceCount == 0){
            arg1[ind] = ch;
        } else if(spaceCount == 1){
            arg2[ind] = ch;
        }
        ind++;
    }
    return true;
}

int main(int argc, char* argv[]){
    if(argc > 2){
        errx(7, "Argument must be 1");
    }

    if(argc == 2 && strlen(argv[1]) > 4){
        errx(8, "Argument must be 4 symbols");
    }

    char command[5] = "";
    if(argc == 2){
        strcpy(command, argv[1]);
    } else {
        strcpy(command, "echo");
    }

    char arg1[5] = "";
    char arg2[5] = "";
    while(readArgs(arg1, arg2)){
        pid_t pid = fork_wrapper();
        if(pid == 0){
            if(strlen(arg2) == 0){
                execlp(command, command, arg1, (char*)NULL);
                err(12, "execlp");
            } else {
                execlp(command, command, arg1, arg2, (char*)NULL);
                err(12, "execlp");
            }
        }
        strcpy(arg1, "");
        strcpy(arg2, "");

        wait_wrapper();
    }   
}