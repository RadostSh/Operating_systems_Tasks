#include <err.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>

pid_t fork_wrapper(void){
    pid_t pid = fork();

    if(pid == -1){
        err(3, "fork");
    }

    return pid;
}

void pipe_wrapper(int pipeFd[2]){
    if(pipe(pipeFd) == -1){
        err(2, "error with pipe");
    }
}

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(3, "arg must be number");
    }
    return num;
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
        errx(1, "argc must be 2");
	}

	int N = convertToInt(argv[1]);

	if( N < 0 || N > 9){
        errx(3, "Argument must be number between 0-9");
	}

	int D = convertToInt(argv[2]);

    if(D < 0 || D > 9){
        errx(4, "Argument must be number between 0-9");
	}

	int parentToChild[2];
	int childToParent[2];
    pipe_wrapper(parentToChild);
    pipe_wrapper(childToParent);

    pid_t pid = fork_wrapper();

    if(pid == 0){
        close(parentToChild[1]);
        close(childToParent[0]);

        int dummy1;
        int res;
        while((res = read(parentToChild[0], &dummy1, sizeof(dummy1))) == sizeof(dummy1)){

            if(write(1, "DONG", 4) == -1){
                err(9, "error with write");
            }

            int dummy2 = 0;
            if(write(childToParent[1], &dummy2, sizeof(dummy2)) == -1){
                err(10, "error with write");

            }

        }

        if(res == -1){
            err(11, "error with read");
        }

        close(childToParent[1]);
        close(parentToChild[0]);
		 exit(0);

    }

    close(parentToChild[0]);
    close(childToParent[1]);

    for(int i = 0; i < N; i++){
        if(write(1, "DING", 4) == -1){
            err(5, "error with write");
        }

        int dummy1 = 0;
        if(write(parentToChild[1], &dummy1, sizeof(dummy1)) == -1){
            err(6, "error with write");
        }

        int dummy2;
        if(read(childToParent[0], &dummy2, sizeof(dummy2)) == -1){
            err(7, "error with read");
        }

        sleep(D);
    }

    close(childToParent[0]);
    close(parentToChild[1]);

    int status;
    if(wait(&status) == -1){
        err(12, "error");
    }

    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(15, "child exited with status: %d", WEXITSTATUS(status));
        }
    } else {
        errx(14, "child was killed");
    }
}