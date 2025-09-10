#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
	if (argc != 2) {
        errx(1, "Argument must be 1");
	}

	if(mkfifo("../myfifo", 0666) == -1 && errno != EEXIST){
        err(2, "error with mkfifo");
	}

    int fd = open("../myfifo", O_WRONLY);
    if (fd == -1) {
        err(3, "open");
    }

    if(dup2(fd, 1) == -1) {
        err(4, "error with dup2");
    }
    close(fd);

    if(execlp("cat", "cat", argv[1], (char*) NULL) == -1) {
        err(5, "error with execlp");
    }
}