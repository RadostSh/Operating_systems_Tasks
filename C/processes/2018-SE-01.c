#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

int pipew(int pipeFd[2]){
    int p = pipe(pipeFd);

    if(p == -1){
        err(2, "pipe");
    }

    return p;
}

pid_t forkw(void){
    pid_t pid = fork();

    if(pid == -1){
        err(3, "fork");
    }
    return pid;
}

int dup2w(int oldFd, int newFd){
    int fd = dup2(oldFd, newFd);
    if(fd == -1){
        err(7, "dup2");
    }

    return fd;
}

int waitw(void){
    int status;

    if(wait(&status) == -1){
        err(5, "wait");
    }

    if(WIFEXITED(status)){
        if(WEXITSTATUS(status)){
            errx(6, "Child exited with status: %d", WEXITSTATUS(status));
        }
    } else {
        err(7, "Child was killed");
    }

    return status;
}

int main(int argc, char* argv[]) {
    if(argc > 2){
        errx(1, "arg must be 1");
    }

    // find argv[1] -printf "%t\n" | sort | head -n 1

    int findToSort[2];
    pipew(findToSort);

    pid_t pid = forkw();

    if(pid == 0){
        close(findToSort[0]);
        dup2w(findToSort[1], 1);
        close(findToSort[1]);
        execlp("find", "find", argv[1], "-printf", "%t\n", (char*)NULL);
        err(8, "execlp");
    } 
    close(findToSort[1]);
    int sortToHead[2];
    pipew(sortToHead);

    pid_t pid2 = forkw();
    if(pid2 == 0){
        close(sortToHead[0]);

        dup2w(findToSort[0], 0);
        close(findToSort[0]);
        dup2w(sortToHead[1], 1);
        close(sortToHead[1]);

        execlp("sort", "sort", (char*)NULL);
        err(9, "err with execlp");
    }
    close(findToSort[0]);
    close(sortToHead[1]);

    pid_t pid3 = forkw();
    if(pid3 == 0){
        dup2(sortToHead[0], 0);
        close(sortToHead[0]);
        execlp("head", "head", "-n1", (char*)NULL);
        err(10, "execlp");
    }
    close(sortToHead[0]);

    for(int i = 0; i < 3; i++){
        waitw();
    }

return 0;

}