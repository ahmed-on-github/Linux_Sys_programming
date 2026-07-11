#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

char buf [16];

int main()
{
	int fd; int char_count;
/* ------------------------ 1 ------------------------*/
	fd = open("file.txt" , O_RDWR | O_CREAT | O_TRUNC, 0644);
	printf("fd = %d\n",fd);
	write(fd , "Hello file :)",13);

	char_count = read(fd, buf, 13); /* returns 0 as we read past the written bytes (EOF)*/
	printf("buf after read: \"%s\",  char count = %d\n", buf, char_count);
    printf("errno = %d\n", errno);

	lseek(fd, 0, SEEK_SET);

	char_count = read(fd, buf, 13);
	printf("buf after read: \"%s\",  char count = %d\n", buf, char_count);
	close(fd);
/* ------------------------ 2 ------------------------*/
    memset(buf,0,sizeof(buf));
	fd = open("file.txt" , O_RDWR | O_CREAT , 0644); /* Overwrites from starting */
	write(fd , "Hi ",2);
    lseek(fd, 0, SEEK_SET);
	char_count = read(fd, buf, 13);
	printf("buf after read: \"%s\",  char count = %d\n", buf, char_count);
	close(fd);
/* ------------------------ 3 ------------------------*/
    memset(buf,0,sizeof(buf));
	fd = open("file.txt" , O_RDWR | O_CREAT | O_TRUNC, 0644); /* deletes then writes from starting */
	write(fd , "Hi",2);
    lseek(fd, 0, SEEK_SET);
	char_count = read(fd, buf, 14);
	printf("buf after read: \"%s\", char count = %d\n", buf, char_count);
	close(fd);

/* ------------------------ 4 ------------------------*/
	fd = 1 ; // stdout
	write(fd , "Hello stdout :)\n",16);

	fd = 2 ; //  stderr
	write(fd , "\x1b[31;1mHello stderr :)\x1b[0m\n",sizeof("\x1b[31;1mHello stderr :)\x1b[0m\n"));

	printf("%d\n", write(0x200 , "Hello stderr :)\n",16)); /* -1, error */

/* ------------------------ 5 ------------------------*/
    /* this prints umd, nit mud */
    fwrite("m",1,1,stdout);  // or printf("m");
    write(STDOUT_FILENO, "u",1);
    printf("d\n");

    return 0;
}
