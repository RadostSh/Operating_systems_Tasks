#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
	if(argc != 3){
        errx(1, "arguments must be 2");
	}

	int pd[2];
	if(pipe(pd) == -1){
        err(4, "pipe");
	}

	int pid = fork();
	if(pid == -1){
        err(5, "fork");
	}

	if(pid == 0){
        close(pd[0]);
        if(dup2(pd[1], 1) == -1){
            err(6, "dup2");
        }
        close(pd[1]);
        execlp("cat", "cat", argv[1], (char*)NULL);
        err(7, "execlp");
	}
	close(pd[1]);
	
    int output = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(output == -1){
        err(3, "open output file");
	}

	uint8_t byte;
	uint8_t mask = 0x20;
	uint8_t prev = 0;
	int res;
	while((res = read(pd[0], &byte, sizeof(byte))) == sizeof(byte)){
	    if(prev == 0x7D){
            byte ^= mask;
			 prev = 0;
	    } else {
	        if(byte == 0x55){
                continue;
	        } else if(byte == 0x7D){
                prev = byte;
                continue;
	        }
	    }

	    if(write(output, &byte, sizeof(byte)) == -1){
            err(8, "write");
	    }
	}
	if(res == -1){
        err(8, "read");
	}
	close(pd[0]);
	close(output);
}