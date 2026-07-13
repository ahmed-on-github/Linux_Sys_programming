#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/module.h>
#include <linux/kernel.h>

int main()
{
    /*lseek():
    #include <sys/types.h>
    #include <unistd.h>

    off_t lseek(int fd, off_t offset, int whence);

    SEEK_SET
          The file offset is set to offset bytes.

    SEEK_CUR
          The file offset is set to its current location plus offset bytes.

    SEEK_END
          The file offset is set to the size of the file plus offset bytes.

    RETURN VALUE
       Upon successful completion, lseek() returns the resulting offset location as measured in bytes from the beginning
       of the file.  On error, the value (off_t) -1 is returned and errno is set to indicate the error.
    */
    /*Ex1 : lseek example to move inside file and overwrite content*/
    struct record{
        int val;
        char name[20];
    };
    struct record bufferRecord = {0};
    int fd = open( "lseekFile.txt" , O_RDWR | O_CREAT , 0644);
    int count;


    if( fd < 0 ){
        perror("Error opening \"lseekFile\" ");
        exit(-1);
    }
    if( write(fd, "Hi this is a message to be written in lseekFile :)\n"
    , 50U/*sizeof("Hi this is a message to be written in lseekFile :)\r\n") */)
    < 50U/*sizeof("Hi this is a message to be written in lseekFile :)\r\n") */){
        perror("Error writing in \"lseekFile\" ");
        exit(-1);
    }

    lseek(fd, 0 , SEEK_SET);
    if( read(fd, &bufferRecord , sizeof(struct record)) < 0 ){
        perror("Error Reading \"lseekFile\" ");
        exit(-1);
    }
    (bufferRecord.val)++;

    if( lseek(fd , sizeof(struct record), SEEK_SET) < 0){
        perror("Error seeking in \"lseekFile\" ");
        exit(-1);
    }
    if( write(fd, &bufferRecord , sizeof(struct record)) < 0 ){
        perror("Error writing to\"lseekFile\" ");
        exit(-1);
    }
    /*Ex2 : read size of file and current offset*/
    count = lseek(fd, 0, SEEK_END);
    printf("sizeof \"lseekFile\" = %u\r\n", count);
    count = lseek(fd, 0, SEEK_CUR);
    printf("sizeof \"lseekFile\" = %u\r\n", count);

    close(fd);
    return 0;
}
