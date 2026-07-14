#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

/*For mmap()*/
#include <sys/mman.h>
/*For readv() and writev() -- Gather-Scatter IO*/
#include <sys/uio.h>

#include <sys/stat.h>
/*For off_t and ssize_t*/
#include <sys/types.h>

typedef struct iovec iovec_t;
typedef struct stat fileStat_t;

#define GATHER_SCATTER_COUNT 3UL

int main(int argc , char **argv){
    fileStat_t srcFileStat  = {0};
    int fd_src, fd_dest;
    /*---------------------------------
    -----------------------------------1- Gather-Scatter IO -----------------------------------
                                                            -----------------------------------*/
    if( (fd_src = open("src.txt", O_RDWR | O_CREAT, 0640)) < 0 ){
        fprintf(stderr, "errno = %d\n", errno);
        perror("open");
        exit(-2);
    }
    else{}

    iovec_t bufferArr [GATHER_SCATTER_COUNT] = {0};

    bufferArr[0].iov_len = 8;
    bufferArr[0].iov_base = (void *)malloc(bufferArr[0].iov_len);

    bufferArr[1].iov_len = 32;
    bufferArr[1].iov_base = (void *)malloc(bufferArr[1].iov_len);

    bufferArr[2].iov_len = 4;
    bufferArr[2].iov_base = (void *)malloc(bufferArr[2].iov_len);

    uint8_t i  = 0;
    if( readv(fd_src,(const iovec_t *)bufferArr,GATHER_SCATTER_COUNT) < 0){
        fprintf(stderr, "errno = %d\n", errno);
        perror("readv");
        close(fd_src);
        for(i = 0 ; i < GATHER_SCATTER_COUNT ; i++){
            free(bufferArr[i].iov_base);
        }
        exit(-1);
    }
    else{
        for(i = 0 ; i < GATHER_SCATTER_COUNT ; i++){
            fwrite(bufferArr[i].iov_base,bufferArr[i].iov_len,1,stdout);
            printf("\r\n");
        }
    }
    if( (fd_dest = open("dest.txt",O_RDWR | O_CREAT, 0640)) < 0){ /* must be open for read as well for mmap */
        fprintf(stderr,"errno = %d\n",errno);
        perror("fopen");
        close(fd_src);
        for(i = 0 ; i < GATHER_SCATTER_COUNT ; i++){
            free(bufferArr[i].iov_base);
        }
        exit(-1);
    }
    else{}
    /*Swap the 1st and last iovec structs*/
    iovec_t temp = bufferArr[0];
    bufferArr[0] = bufferArr[GATHER_SCATTER_COUNT-1];
    bufferArr[GATHER_SCATTER_COUNT-1] = temp;

    for(i = 0 ; i < GATHER_SCATTER_COUNT ; i++){
        fwrite(bufferArr[i].iov_base,bufferArr[i].iov_len,1,stdout);
        printf("\r\n");
    }

    /*Now Scatter the buffers over the new files with the new ordering*/
    if( (writev(fd_dest,(const iovec_t *)bufferArr, GATHER_SCATTER_COUNT)) < 0){
        fprintf(stderr, "errno = %d\n", errno);
        perror("writev");
        close(fd_src);
        close(fd_src);
        for(i = 0 ; i < GATHER_SCATTER_COUNT ; i++){
            free(bufferArr[i].iov_base);
        }
        exit(-1);
    }

    for(i = 0 ; i < GATHER_SCATTER_COUNT ; i++){
        free(bufferArr[i].iov_base);
    }

    /*---------------------------------
    -----------------------------------2- mmap()-----------------------------------
                                                            -----------------------------------*/

    srcFileStat.st_size = lseek(fd_src, 0, SEEK_END);
    printf("src.txt len = %ld chars\n", srcFileStat.st_size);

    /* Now fork  and map the src file to be private for each of the parent and child */
    pid_t pid;

    if(  (pid = fork()) <0 ){
      perror("fork: ");
      fprintf(stderr,"errno = %d\r\n", errno);
      close(fd_src);
      close(fd_dest);
      exit(-1);

    }
    char *srcFilePtr = (char *)mmap(NULL, (srcFileStat.st_size)/2
    /*Number of bytes to be mapped from disk to the buffer
    (this buffer resides between stack and heap with shared libraries).
     Assume large file and we are limited to a buffer of half its size*/
    , PROT_WRITE | PROT_READ, MAP_PRIVATE, fd_src, 0); /* MAP_PRIVATE or MAP_SHARED */
    if(srcFilePtr == MAP_FAILED){
        perror("mmap: ");
        fprintf(stderr,"errno = %d\r\n", errno);
        close(fd_src);
        close(fd_src);
        exit(-1);
    }

    if( pid == 0 ){ /* child */

        printf("Child printing memory mapped src file:\r\n");
        for(i = 0 ; i < srcFileStat.st_size; i++){ /* Overflowing mapped size (srcFileStat.st_size)/2): Usually unsafe
                                                      but works here only because we still access the same mapped page
                                                      but generally will generate segmentation fault (SIGSEGV)
                                                    */
            if( srcFilePtr[i] == ' ' ) srcFilePtr[i] = '-'; /* child's own copy replaces spaces with dashes*/
            printf("%c", srcFilePtr[i]);
        }
        printf("\r\n");
        exit(0); /* not to spawn new process in the next fork*/
    }
    else{ /* parent */
        sleep(3);
        printf("Parent printing memory mapped src file:\r\n");
        for(i = 0 ; i < srcFileStat.st_size; i++){ /* Overflowing mapped size (srcFileStat.st_size)/2): Usually unsafe
                                                      but works here only because we still access the same mapped page
                                                      but generally will generate segmentation fault (SIGSEGV)
                                                    */
            printf("%c", srcFilePtr[i]);
        }
        printf("\r\n");
        wait(NULL);
    }

    munmap(srcFilePtr, srcFileStat.st_size/2); /* Do not use this line if you do not want to save changes.
                                                  Child exits before this line so no worries */
    close(fd_src);


    /* Now fork again and map the dest file to be shared across parent and child */
    loff_t destFileSize = (loff_t)lseek(fd_dest, 0, SEEK_END);
    lseek(fd_dest, 0, SEEK_SET); /* return offset to beginning of file. However, this does not work with memory-mapped files   */
    {
        if(  (pid = fork()) <0 ){
          perror("fork: ");
          fprintf(stderr,"errno = %d\r\n", errno);
          close(fd_dest);
        }
        char *destFilePtr = (char *)mmap(NULL, destFileSize/2
        /*Number of bytes to be mapped from disk to the buffer
        (this buffer resides between stack and heap with shared libraries).
         Assume large file and we are limited to a buffer of half its size*/
        , PROT_WRITE | PROT_READ , MAP_SHARED, fd_dest, 0); /* MAP_PRIVATE or MAP_SHARED */

        if(destFilePtr == MAP_FAILED){
            fprintf(stderr,"errno = %d\r\n", errno);
            perror("mmap: ");
            close(fd_src);
            exit(-1);
        }
        if( pid == 0 ){ /* child process */
            sleep(3);
            printf("child process:\n");
            i = 0;
            printf("fd_dest seek at i=%d : %ld\n", i , lseek(fd_dest, 0, SEEK_CUR) ); /* this shows that seek does not work with mmap*/
            for(i = 0; i < destFileSize ; i++){ /* Overflowing mapped size (srcFileStat.st_size)/2): Usually unsafe
                                                      but works here only because we still access the same mapped page
                                                      but generally will generate segmentation fault (SIGSEGV)
                                                */
                printf("%c", destFilePtr[i]);
                if(i == destFileSize/2){
                    printf("fd_dest seek at i=%d : %ld\n", i , lseek(fd_dest, 0, SEEK_CUR) );
                }
            }
            printf("\r\n");
            printf("fd_dest seek at i=%d : %ld\n", i , lseek(fd_dest, 0, SEEK_CUR) );
        }
        else{  /* parent process */
            /* Modify the 2nd half of the file */
            lseek(fd_dest, destFileSize/2, SEEK_SET); /* Does not work as we mapped the file from offset 0 */
            for(i=destFileSize/2; i < destFileSize  ; i++  ){ /* Start from desired offset */
                                                              /* Overflowing mapped size (srcFileStat.st_size)/2): Usually unsafe
                                                              but works here only because we still access the same mapped page
                                                              but generally will generate segmentation fault (SIGSEGV)
                                                              */
              int val = (int)destFilePtr[i];
              /*Swap case of each letter*/
              if( isalpha( val ) ) {
                  if( isupper(val) ){
                      destFilePtr[i] += ('a' - 'A');
                  }
                  else{
                      destFilePtr[i] -= ('a' - 'A');
                  }
              }
            }
            wait(NULL);
        }

        munmap(destFilePtr, destFileSize/2);
        close(fd_dest);
        printf("Program worked Successfully\r\n");
    }
    return 0;
}
