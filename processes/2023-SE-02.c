#include <unistd.h>
#include <sys/wait.h>
#include <err.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

int pipe_wrapper(int mypipe[2]){
    int p = pipe(mypipe);
    if(p == -1){
        err(26, "error with pipe");
    }
    return p;
}

pid_t fork_wrapper(void){
    pid_t pid = fork();

    if(pid == -1){
        err(26, "error with fork");
    }

    return pid;
}

int wait_wrapper(void){
    int status;
    if(wait(&status)){
        err(26, "error with wait");
    }

    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(26, "Child exit with status: %d", WEXITSTATUS(status));
        }
    } else {
        errx(26, "Child was killed");
    }

    return status;
}

int dup2_wrapper(int oldFd, int newFd){
    int fd = dup2(oldFd, newFd);

    if(fd == -1){
        err(26, "error with dup2");
    }

    return fd;
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
        errx(26, "Needed one argument");
	}

	int mypipe[2];
    pipe_wrapper(mypipe);

    pid_t pids[4096];

    for(int i = 1; i < argc; i++){
        pid_t pid = fork_wrapper();

        if(pid == 0){
            close(mypipe[0]);
            dup2_wrapper(mypipe[1], 1);
            close(mypipe[1]);
        
            if(execlp(argv[i], argv[i], (char*)NULL) == -1){
                err(26, "error with execlp");
            }
        }

        pids[i - 1] = pid;
    }

    close(mypipe[1]);
    const char* pattern = "found it";
    char ch;
    size_t index = 0;
    int res;
    while((res = read(mypipe[0], &ch, sizeof(ch))) == sizeof(ch)){
        if(ch == pattern[index]){
            index++;
        } else {
            index = 0;
        }

        if(index == strlen(pattern)){
            for(int i = 0; i < argc - 1; i++){
                if(kill(pids[i], SIGTERM) == -1){
                    err(26, "kill");
                } 
                wait_wrapper();
                exit(0);
            }
        }
    }

	if(res == -1){ 
        err(26, "read");
    } 

	close(mypipe[0]);
	
	for(int i = 1; i < argc; i++){
        wait_wrapper();
    }
    exit(1);
}