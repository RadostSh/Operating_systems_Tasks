#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <err.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

const char* WORDS[] = {
    "tic ",
    "tac ",
    "toe\n"
};

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(1, "arguments must be number");
    }
    return num;
}

pid_t forkw(void){
    pid_t fd = fork();
    if(fd == -1){
        err(5, "fork");
    }
    return fd;
}

void pipew(int pd[2]){
    if(pipe(pd) == -1){
        err(6, "pipe");
    }
}

void dup2w(int old, int new){
    if(dup2(old, new) == -1){
        err(7, "dup2");
    }
}

void waitw(void){
    int status;
    if(wait(&status) == -1){
        err(8, "wait");
    }

    if(WIFEXITED(status)){
        if(WEXITSTATUS(status)){
            errx(9, "Child exit with status %d", WEXITSTATUS(status));
        }
    } else {
        errx(10, "Child was killed");
    }
}

ssize_t writew(int fd, const void* buff, ssize_t bytes){
    ssize_t res = write(fd, buff, bytes);
    if(res == -1){
        err(11, "write");
    }
    return res;
}

void handleMessages(int words, int fromFD, int toFD){
    int count;
    ssize_t read_size;

    bool written_final_signal = false;

    while((read_size = read(fromFD, &count, sizeof(count))) == sizeof(count)){
        if(count >= words){
            if(!written_final_signal){
                writew(toFD, &count, sizeof(count));
            }
            return;
        }

        writew(1, WORDS[count % 3], strlen(WORDS[count % 3]));
        int next = count + 1;
        writew(toFD, &next, sizeof(next));

        if(next >= words){
            written_final_signal = true;
        }
    }
    if(read_size == -1){
        err(11, "read");
    }
}


int main(int argc, char* argv[]) {
	if(argc != 3){
        errx(2, "arg must be 2");
	}

	int child = convertToInt(argv[1]);
	if(child < 1 || child > 8){
        errx(3, "First argument must be between 1 and 8");
	}

	int words = convertToInt(argv[2]);
	if(words < 1 || words > 35){
        errx(4, "Second argument must be between 1 and 35");
	}
    
    int pipes = child + 1;
    int pipeFDs[8][2];
    for(int i = 0; i < pipes; i++){
        pipew(pipeFDs[i]);
    }

    for(int i = 1; i <= child; i++){
        pid_t fd = forkw();
        if(fd == 0){
            int fromFD = pipeFDs[i-1][0];
            int toFD = pipeFDs[i][1];

            for(int p = 0; p < pipes; p++){
                if(pipeFDs[p][0] != fromFD){
                    close(pipeFDs[p][0]);
                }
                if(pipeFDs[p][1] != toFD){
                    close(pipeFDs[p][1]);
                }
            }
            handleMessages(words, fromFD, toFD);
            close(fromFD);
            close(toFD);
            exit(0);
        }
    }

    int fromFD = pipeFDs[child][0];
    int toFD = pipeFDs[0][1];

    for(int i = 0; i < pipes; i++){
        if(pipeFDs[i][0] != fromFD){
            close(pipeFDs[i][0]);
        }
        if(pipeFDs[i][1] != toFD){
            close(pipeFDs[i][1]);
        }
    }

    writew(1, WORDS[0], strlen(WORDS[0]));

    int next = 1;
    writew(toFD, &next, sizeof(next));

    handleMessages(words, fromFD, toFD);

    close(fromFD);
    close(toFD);

    for(int i = 1; i <= child; i++){
        waitw();
    }
}