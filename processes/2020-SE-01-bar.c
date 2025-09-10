#include <fcntl.h>
#include <err.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if(argc != 2){
        err(1, "arg must be 1");
	}

	int fd = open("../myfifo", O_RDONLY);
	if(fd < 0){
        err(2, "error with open myfifo");
    }

    if(dup2(fd, 0) == -1){
        err(3, "dup2");
    }
    close(fd);

    if(execl(argv[1], argv[1], (char*)NULL) == -1){
        err(4, "execl");
    }
}