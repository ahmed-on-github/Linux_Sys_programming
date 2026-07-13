/*

To find info about any linux command, function, use the man 'command'
    man 1 gets the user command info entry
    man 2 gets the system call info entry
    man 3 gets the library routines info entry
    man 7 gets miscellaneous info entry

Ex: man chmod
    man 2 chmod
    man 3 chmod --> no manual entry for chmod in section 3

Ex: man 2 write

WRITE(2)

NAME
       write - write to a file descriptor

SYNOPSIS
       #include <unistd.h>

       ssize_t write(int fd, const void *buf, size_t count);
    where ssize_t(long int) is signed size_t(unsigned long int), which includes the sign for errors

Ex: chmod 400 foo

This changes the permissions for 'foo' to only be readable by its owner
Then if we run this code again, we get an error (fd = -1): permission denied ( errno = 13)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* For write()*/
#include <unistd.h>
/* For open() function*/
#include <fcntl.h>
/* For global 'errno' variable*/
#include <errno.h>
/* For alloca()*/
#include <alloca.h>

int main(int argc, char ** argv)
{
    int fd ;
    /*Open for write only, create if not existing, permissions are rw- r-- r--*/
    if( (fd = open("foo", O_WRONLY | O_CREAT , 0644 ) ) >=  0 ){
        write(fd, "Hello world!\n", 12);
    }
    else{/*Typically sys calls return -1 on failure*/
        /*
        Notice:
            1- write() sends only pure string (no format specifiers, as it's not a variadic function
            2- fd = 0 (stdin) , = 1 (stdout), =2 (stderr)
        */
        write( 1, "Error number = " , strlen("Error number = "));  /*Write pure text to stdout*/
        printf("%d\n", errno); /*The global variable 'errno' is declared in errno.h */
        write(2, "error in foo\n" , strlen("error in foo\n")); /*Write pure text to stderr*/
        perror(""); /*Prints a message that descripes the error*/
        printf("\nerrno = %d\n", errno);
  
    printf("fd = %d\n", fd);
//    /*Typically any printed address is virtual :/ */
//    printf("Address of errno declared by errno.h = %p\n" , &errno);
//    printf("printf address = %p\n", printf); /*Sys call to Std library glibc*/
//    printf("write address = %p\n", write); /*Sys call, not direct to the kernel, but a wrapper */
//
//
//    printf("Allocating 0 bytes from current SP = %p\n", alloca(0)); /* not actual SP !!*/
//    alloca(4) ;
//    printf("Allocating 4 bytes from current SP = %p\n", alloca(0));
//    alloca(4) ;
//    printf("Allocating 4 bytes from current SP = %p\n", alloca(0));
//    printf("sqrt(5.0f) = %f", (float)sqrt(5.0));

/*----------------------------------1- Unbuffered IO------------------------------------------*/

    /*int open(const char *pathname, int flags, mode_t mode);
    *   flags must include one of  O_RDONLY , O_WRONLY, O_RDWR
        flags may contain one of O_APPEND, O_CREAT, O_TRUNC
        flags are ORed together

    *   mode is only passed when file is being created
    */

    /*closing file*/
    /* extern int close (int __fd);
        returns 0 on success
    */
    /*Ex1: closing stdout*/
    if( (close(STDOUT_FILENO),-1) != 0){ /*  */
        perror("Invalid file descriptor fd to close"); /*Prints to stderr*/
    }
    else{
        printf("Printing after closing stdout :|\n"); /* this branch is not executed as condition issss always true */
    }
    printf("Printing after closing stdout :|\n"); /*Does not print to stdout */

    close(fd);
    /*file read*/
    /*
    ssize_t read (int __fd, void *__buf, size_t __nbytes)
    */

    /*file write*/
    /*
    ssize_t write(int fd, const void *buf, size_t count);
    */
    return 0;
}
