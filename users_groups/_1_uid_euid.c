/**
 *      Testing user ID and effective user ID on /etc/hosts
 *      We compile this file as:
 *              $ gcc uid_euid1.c -o ~/bin/uid_test
 *      We need to make root as the owner of the executable
 *              $ sudo chown root $(which uid_test)
 *      We need to set the suid bit for the executable file
 *              $ sudo chmod u+s $(which uid_test)
 *      We run it once as same user (All will fail after changing euid to uid -- not root anymore):
 *              $ uid_test
 *      Then we run it as root (All will pass):
 *      (use which as uid_test will not be found under any directory of the  $PATH for root)
 *              $ sudo $(which uid_test)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#define TEST_ETC_HOSTS_PERMS() do{ \
        \
        printf("Real       UID:         %d\n", getuid());  \
        printf("Effective  UID:         %d\n", geteuid()); \
        \
        /* Testing permissions on file */               \
        /* Tests against effective UID, (succeeds and returns 0 when runs as root) */\
        printf("open:   %d\n",    open("/etc/hosts", O_RDWR));\
        /* Tests against real UID (R_OK, W_OK and X_OK) (fails if not running as root and  returns -1) */ \
        printf("access: %d\n",    access("/etc/hosts", W_OK));    \
}while(0);

int main(int argc, char **argv){
        TEST_ETC_HOSTS_PERMS();

        /* Drop root priviliges and try again*/
        seteuid(getuid());

        TEST_ETC_HOSTS_PERMS();
        return 0;
}
