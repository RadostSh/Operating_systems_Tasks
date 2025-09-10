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
#include <time.h>

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(3, "argument should be a number");
    }
    return num;
}

int main(int argc, char* argv[]) {
    if(argc < 2){
        err(1, "arguments");
    }
    
    int num = convertToInt(argv[1]);
    if(num < 1 || num > 9){
        errx(4, "Number must be between 1 and 9");
    }

    char* args[256];
    for(int i = 2; i <= argc; i++){
        if(i == argc){
            args[i] = NULL;
            break;
        }
        args[i - 2] = argv[i];

    }

    int file = open("run.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(file == -1){
        err(2, "open");
    }
    
    bool flag = true;
    int count = 1;
    while(flag){
        int fd = fork();
        if(fd == -1){
            err(6, "fork");
        }
        
        time_t startTime = 0;
        if(fd == 0){
            startTime = time(NULL); 
            execvp(argv[2], args);
            err(7, "execlp");
        }
        int status;
        if(wait(&status) == -1){
            err(8, "wait");
        }
        if(WIFEXITED(status) == 0){
            err(129, "Program was terminated");
        }
        int exitCode = WEXITSTATUS(status);

        time_t endTime = time(NULL);
        
        char buff[1026];
        int len = snprintf(buff, sizeof(buff), "%ld %ld %d\n", startTime, endTime, exitCode);
        if(write(file, buff, len) == -1){
            err(9, "write");
        }
        
        if(count == 2){
            if(status != 0 && (endTime - startTime) < num){
                flag = false;
            } else {
                count = 1;
            }
        } else {
            count++;
        }   
    }
    close(file);
}