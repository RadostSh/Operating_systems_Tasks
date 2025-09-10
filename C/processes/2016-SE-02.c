#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(void) {
	char buff[] = "Enter command";
	char command[1024];
	char bin_path[1032];

	while(true){
        if(write(1, buff, strlen(buff)) < 0) {
            err(1, "write");
        }

        int bytesRead = read(0, command, sizeof(command));
        if(bytesRead == -1){
            err(2, "read");
        }

        command[bytesRead - 1] = '\0';

        if(strcmp(command, "exit") == 0){
            break;
        }

        snprintf(bin_path, sizeof(bin_path), "/bin/%s", command);

        pid_t pid = fork();
        if(pid == -1){
            err(3, "fork");
        }

        if(pid == 0){
            execlp(bin_path, command, (char*)NULL);
            err(4, "execlp");
        }

        int stat;
        wait(&stat);
        if(!WIFEXITED(stat)){
            err(5, "command wasn't executed");
        }
	}
}