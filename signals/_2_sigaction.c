#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

void myhandler(int signum){

        //printf("Signal  caught\n");
        printf("Signal %s caught\n", strsignal(signum));
}

int main(){
        struct sigaction action;

        action.sa_handler = myhandler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_RESTART; // Or 0;

        int fd[2], n;
        char buf [20];
        pipe(fd);

        sigaction(SIGINT, &action, NULL /* old sigaction ignored*/);
        sigaction(SIGHUP, &action, NULL /* old sigaction ignored*/);


      while(1){
              /* blocking system call as reading from the read end of pipe will block (not connected to any file/stream) */
              n = read(fd[0], buf, sizeof(buf )); 
              /* Won't be reached if action.sa_flags has SA_RESTART bit set*/
              perror("read: ");
              printf("read %d bytes, errno = %d\n", n , errno); 
        }

        return 0;
}
