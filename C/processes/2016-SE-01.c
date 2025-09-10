#include <fcntl.h>
#include <unistd.h>
#include <err.h>

void pipe_wrapper(int p[2]){
    if(pipe(p) == -1){
        err(2, "pipe");
    }

}

pid_t fork_wrapper(void){
    pid_t pid = fork();

    if(pid == -1){
        err(3, "fork");
    }

    return pid;
}

int dup2_wrapper(int old, int new){
    int res = dup2(old, new);

    if(res == -1){
        err(4, "dup2");
    }

    return res;
}

int main(int argc, char* argv[]) {
	if(argc != 2){
        err(1, "arg must be one");
	}

    //cat argv[1] | sort

    int catToSort[2];
    pipe_wrapper(catToSort);

    pid_t pid = fork_wrapper();

    if(pid == 0){
        close(catToSort[0]);

        dup2_wrapper(catToSort[1], 1);
        close(catToSort[1]);

        execlp("cat", "cat", argv[1], (char*)NULL);
        err(5, "execlp");
    }

    close(catToSort[1]);

    dup2_wrapper(catToSort[0], 0);
    close(catToSort[0]);

    execlp("sort", "sort", (char*)NULL);
    err(6, "execlp");

}