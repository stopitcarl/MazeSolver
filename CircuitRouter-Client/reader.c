#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char *argv[]){
	int fd = open(argv[1], O_RDONLY);
	char arr[6];
	printf("Opened %s with fd: %d\n", argv[1], fd);

	while(1){
		read(fd, arr, sizeof(arr));
		printf("_%s_\n", arr);
	}



}
