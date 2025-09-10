#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdlib.h>

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(2, "agument must be number");
    }

    return num;
}

pid_t fork_wrapper(void){
    pid_t pid = fork();
    if(pid == -1){
        err(3, "error with fork");
    }
    return pid;
}

void pipe_wrapper(int pipefd[2]){
    if(pipe(pipefd) == -1){
        err(4, "error with pipe");
    }
}

void dup2_wrapper(int oldfd, int newfd){
    if(dup2(oldfd, newfd) == -1){
        err(5, "error with dup2");
    }
}

int main(int argc, char* argv[]) {
	if(argc != 4){
        errx(1, "Arguments must be 3");
	}

    int N = convertToInt(argv[2]);
    if(N < 0){
        errx(3, "Arg must be positive number");
    }

    int fdUrandom = open("/dev/urandom", O_RDONLY);
    if(fdUrandom == -1){
        err(4, "error with open");
    }

    int fdNull = open("/dev/null", O_WRONLY);
    if(fdNull == -1){
        err(5, "open");
    }

    int fdOut = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0600);
    if(fdOut == -1){
        err(6, "open");
    }

    int status = 0;
    char buff[UINT16_MAX];

    for(int i = 0; i < N; i++){
        int fdPipe[2];
        pipe_wrapper(fdPipe);

        pid_t pid = fork_wrapper();

        if(pid == 0){
            close(fdUrandom);
            close(fdOut);
            close(fdPipe[1]);

            dup2_wrapper(fdPipe[0], 0);
            close(fdPipe[0]);
            
            dup2_wrapper(fdNull, 1);
            dup2_wrapper(fdNull, 2);
            close(fdNull);

            execl(argv[1], argv[1], (char*)NULL);
        }
        close(fdPipe[0]);
        uint16_t toWrite;
        if(read(fdUrandom, &toWrite, sizeof(toWrite)) == -1){
            err(7, "error with read");
        }

        if(read(fdUrandom, &buff, toWrite) == -1){
            err(8, "error");
        }

        if(write(fdPipe[1], &buff, toWrite) == -1){
            err(9, "err");
        }

        close(fdPipe[1]);

        int result;
        if(wait(&result) == -1){
            err(10, "wait");
        }
        if(WIFSIGNALED(result)){
            if(write(fdOut, &buff, toWrite) == -1){
                err(11, "write");
            }
            status = 42;
            break;
        }

    }

    close(fdOut);
    close(fdUrandom);
    close(fdNull);

    return status;
}