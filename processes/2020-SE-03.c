#include <sys/wait.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct triple{
    char name[8];
    uint32_t offset;
    uint32_t length;
} triple;

int pipes[8][2];
int pids[8];

void child_process(triple t, int index) {
    if(pipe(pipes[index]) == -1){
        err(1, "pipe");
    }

    int pid = fork();
    if(pid == -1){
        err(2, "fork");
    }

    if (pid == 0) {
        close(pipes[index][0]);
        //XOR operaion
        int fd = open(t.name, O_RDONLY);
        if(fd == -1){
            err(3, "open");
        }
        if(lseek(fd, t.offset * sizeof(uint16_t), SEEK_SET) == -1){
            err(4, "lseek");
        }
        uint16_t result = 0;
        for(uint32_t i = 0; i < t.length; i++) {
            uint16_t temp;
            if(read(fd, &temp, sizeof(temp)) == -1){
                err(5, "read");
            }
            result ^= temp;
        }
        if(write(pipes[index][1], &result, sizeof(result)) == -1){
            err(6, "write");
        }
        exit(0);
    }
    close(pipes[index][1]);
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        errx(1, "one argument");
    }

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
        err(1, "open");
	}

	triple t;
	int n;
	int count = 0;
	while((n = read(fd, &t, sizeof(t))) > 0) {
        child_process(t, count);
        count++;
	}
	if (n < 0) {
        err(2, "read");
	}

	for(int i = 0; i < count; i++) {
        int wstatus;
        if(wait(&wstatus) == -1){
            err(9, "wait");
        }
        if (!WIFEXITED(wstatus)) {
            errx(3, "child not exit");
        }

        if(WEXITSTATUS(wstatus) != 0) {
            errx(4, "child did not exit with 0");
        }
	}

	uint16_t result = 0;
	for(int i = 0; i < count; i++) {
        uint16_t temp;
        if(read(pipes[i][0], &temp, sizeof(temp)) == -1){
            err(10, "read");
        }
        result ^= temp;
        close(pipes[i][0]);
	}
	char output[1024];
	int len = snprintf(output, sizeof(output), "Result: %.4X\n", result);
	if(write(1, output, len) == -1){
        err(11, "write");
    }
}