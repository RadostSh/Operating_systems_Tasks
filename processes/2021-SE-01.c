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
#include <sys/time.h>
#include <pwd.h>

int main(int argc, char* argv[]) {
	if(argc == 1){
        errx(1, "needs more arguments");
	}
    const char filename[] = "foo.log";
	int foo = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(foo == -1){
        err(2, "open");
	}

    uid_t uid = getuid();
    struct passwd* usr_info = getpwuid(uid);
    char *username = usr_info->pw_name;

    struct timeval tv;
    if(gettimeofday(&tv, NULL) == -1){
        err(3, "gettimeofday");
    }
    
    const struct tm *time = localtime(&tv.tv_sec);
    if(time == NULL){
        err(4, "localtime");
    }

    char date[32];
    strftime(date, sizeof(date), "%F %T", time);
    char fullDate[64];
    snprintf(fullDate, sizeof(fullDate), "%s.%06ld", date, tv.tv_usec);

    char log[1026];
    snprintf(log, sizeof(log), "%s %s", fullDate, username);

    for(int i = 0; i < argc; i++){
        snprintf(log + strlen(log), sizeof(log) - strlen(log), " %s", argv[i]);
    }

    snprintf(log + strlen(log), sizeof(log) - strlen(log), "\n");

    if(write(foo, log, strlen(log)) == -1){
        err(5, "write");
    }

	 close(fd);

    int pd[2];
    if(pipe(pd) == -1){
        err(6, "pipe");
    }

    int pid = fork();
    if(pid == -1){
        err(7, "fork");
    }

    if(pid == 0){
        close(pd[0]);
        if(dup2(pd[1], 1) == -1){
            err(8, "dup2");
        }
        close(pd[1]);
        execlp("passwd", "passwd", "-l", username, (char*)NULL);
        err(9, "execlp");
    }
    close(pd[1]);
    pid_t pid2 = fork();
    if(pid2 == -1){
        err(10, "fork");
    }

    if(pid2 == 0){
        close(pd[1]);
        if(dup2(pd[0], 0) == -1){
            err(11, "dup2");
        }
        close(pd[1]);
        execlp("pkill", "pkill", "-u", username, (char*)NULL);
        err(12, "execlp");
    }
    close(pd[0]);
   
    wait(NULL);
    wait(NULL);
}