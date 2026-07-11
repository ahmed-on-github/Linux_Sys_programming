#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <errno.h>


#define SIGNAL_ERR_CHECK() \
        if(old_sig_handler == SIG_ERR){\
                perror("signal: ");\
                fprintf(stderr, "errno: %d, old_sig_handler: %p ", errno, old_sig_handler );\
        }

typedef void (*signal_handler_t) (int);



void sig_handler(int num){
        switch(num){
                case SIGINT:{
                        printf("caught SIGINT, SIGINT = %d\n", SIGINT);
                        break;
                             }
                case SIGQUIT:{
                        printf("caught SIGQUIT, SIGQUIT = %d, exiting ...\n", SIGQUIT);
                        exit(EXIT_SUCCESS);
                        break;
                              }
                case SIGFPE:
                case SIGSEGV:
                        printf("caught %s, errno = %d\n", strsignal(num), errno);
                        break;
                default: break; /* Nothing */

        }
}

int main( int argc, char **argv){
        signal_handler_t old_sig_handler;
        /* Sent as Ctrl+c */
        old_sig_handler = signal(SIGINT, sig_handler );
        SIGNAL_ERR_CHECK();

        /* Sent as Ctrl+\ */
        old_sig_handler = signal(SIGQUIT, sig_handler );
        SIGNAL_ERR_CHECK();
        /* Sent from CPU on FPE */
        old_sig_handler = signal(SIGFPE, sig_handler );
        SIGNAL_ERR_CHECK();
        /* Sent from kernel due to segmentation violation */
        old_sig_handler = signal(SIGSEGV, sig_handler );
        SIGNAL_ERR_CHECK();

        /* the program will retry to do these oprations again after signal handling,
         * without solving the issue, causing infinite signal handling*/
        //int x=10, y = 0;
        //x= *((int *)y++); /* casuses SIGSEGV signal  */
        //printf("x/y  = %d\n", x/y ); /* Causes SIGFPE signal */
        /* Use raise */
        raise(SIGSEGV);
        raise(SIGFPE);
        while(1){
        }
        return 0;
}
