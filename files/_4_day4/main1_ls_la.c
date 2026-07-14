/** 
  * A file that performs ls -l
  * Build as:
  *    $ mkdir -p ~/bin
  *    $ PATH=$PATH:~/bin
  *    $ gcc main1_ls_la.c -o ~/bin/ls_la
  * Run it as:
  *    $ ./ls_la                          # For current directory, output to stdout
  *    $ ./ls_la /usr/sbin                # for argv[1] as directory, output to stdout
  *    $ ./ls_la /etc/ssh ./outputfile    # for argv[1] as directory, output to argv[2]
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <time.h>
#include <limits.h>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#define INPUT_DIRECTORY     (argv[1])
#define OUTPUT_RESULT_FILE  (argv[2])


typedef struct stat stat_t;
typedef struct dirent dirent_t;

static char fullfilepathbuf[2048] ;

/*
(sb.st_mode & S_IFMT) == S_IFREG

           S_IFMT     0170000   bit mask for the file type bit field

           S_IFSOCK   0140000   socket

           S_IFLNK    0120000   symbolic link
           S_IFREG    0100000   regular file
           S_IFBLK    0060000   block device
           S_IFDIR    0040000   directory
           S_IFCHR    0020000   character device
           S_IFIFO    0010000   FIFO
*/

void list_ll( stat_t *statBuffPtr , FILE *outputFile , char *fileName){
    char dateTimeBuff [50U] = {0};
    if( statBuffPtr && outputFile ){
        /*1st: File type*/
        switch( (statBuffPtr->st_mode) & S_IFMT ){
            case S_IFREG :{
                fprintf(outputFile,"-"); /**/
                break;
            }
            case S_IFDIR :{
                fprintf(outputFile,"d"); /**/
                break;
            }
            case S_IFLNK :{
                fprintf(outputFile,"l"); /**/
                break;
            }
            case S_IFBLK :{
                fprintf(outputFile,"b"); /**/
                break;
            }
            case S_IFCHR :{
                fprintf(outputFile,"c"); /**/
                break;
            }
            case S_IFIFO :{
                fprintf(outputFile,"p"); /**/
                break;
            }
            case S_IFSOCK :{
                fprintf(outputFile,"s"); /**/
                break;
            }
            default: {
                fprintf(outputFile, "-");
            }

        }
        /*2- Permissions*/
        fprintf(outputFile, "%c%c%c%c%c%c%c%c%c ",
        /*Owner user permissions*/
        (statBuffPtr->st_mode & S_IRUSR)?'r':'-',
        (statBuffPtr->st_mode & S_IWUSR)?'w':'-',
        (statBuffPtr->st_mode & S_IXUSR)?'x':'-',
        /*Owner group permissions*/
        (statBuffPtr->st_mode & S_IRGRP)?'r':'-',
        (statBuffPtr->st_mode & S_IWGRP)?'w':'-',
        (statBuffPtr->st_mode & S_IXGRP)?'x':'-',
        /*Others permissions*/
        (statBuffPtr->st_mode & S_IROTH)?'r':'-',
        (statBuffPtr->st_mode & S_IWOTH)?'w':'-',
        (statBuffPtr->st_mode & S_IXOTH)?'x':'-'
        );
        /*3- Number of hard links*/
        fprintf(outputFile, " %lu", statBuffPtr->st_nlink);
        /*4- Owner user and owner group -- ?
            Starts from 1000 as below values are for installed software as users
        */
        fprintf(outputFile, " %u", statBuffPtr->st_uid);
        fprintf(outputFile, " %u", statBuffPtr->st_gid);

        /*5- File size in bytes*/
        fprintf(outputFile, " %ld", statBuffPtr->st_size);
        /*

        struct timespec ts;
clock_gettime(clk_id, &ts);
struct tm *my_tm = localtime(&ts.tv_sec);

        */
        /*6- Last modified time*/
        strcpy( dateTimeBuff, ctime(&(statBuffPtr->st_atim.tv_sec)));
        dateTimeBuff[ strlen(dateTimeBuff) - 1] = ' '; /*Replace \n with space*/
        fprintf(outputFile, " %s", dateTimeBuff );
        // time_t -> long int
        // timespec
        /*7- file name*/
        fprintf(outputFile, "%s", fileName );
        fprintf(outputFile, "\r\n" );

    }

}
int main(int argc, char **argv){

    dirent_t *direntPtr = NULL;
    stat_t statBuff;

    FILE *output_file = stdout; /* default value */
    //char Dirpath [PATH_MAX];
    DIR *dir = NULL;
    char *dirpath = NULL;

    if( unlikely(argc < 2) ){
        /*
        fprintf(stderr,"no input directory passed\n");
        exit(-1);
        */
        dirpath = getcwd(fullfilepathbuf, sizeof(fullfilepathbuf));
    }
    else{
        output_file = ( (argc ==  2)?stdout: output_file); /*If no output path specified, use stdout*/

        /*Open output file*/
        if(argc > 2 && argv && argv[2] != NULL ){
            if( likely((output_file = fopen(OUTPUT_RESULT_FILE, "w")) != NULL ) ){

            }
            else{
                fprintf(stderr, "Could not open output file specified by argv[2]\r\n");
                exit(-1);
            }
        }
        else{}
        dirpath = argv[1];
    }

    /*Open directory to read files*/
    if( likely( (dir = opendir(dirpath) ) != NULL ) ){
        size_t dirpath_len = strlen(dirpath);
        memset(fullfilepathbuf, 0 , sizeof(fullfilepathbuf));
        memcpy(fullfilepathbuf, dirpath, dirpath_len);
        fullfilepathbuf[ strlen(fullfilepathbuf) ] = '/';

        /*Read Next file/directory inside the specified input directory in argv[1]*/
        while( likely( (direntPtr = readdir(dir)) != NULL ) ){
            memset(fullfilepathbuf + dirpath_len + 1  , 0,  sizeof(fullfilepathbuf) - (dirpath_len +1) );
            memcpy(fullfilepathbuf + dirpath_len + 1, direntPtr->d_name, strlen(direntPtr->d_name) );
            if( likely( (stat(fullfilepathbuf, &statBuff) >= 0 ) ) ){
                list_ll(&statBuff, output_file, direntPtr->d_name);

            }
            else{
                fprintf(stderr,"errno = %d\r\n", errno);
                perror("stat");
                exit(-1);
            }
        }


    }
    else{
        fprintf(stderr,"Could not open specified input directory (%s) in argv[1]\r\n", argv[1]);
        fprintf(stderr,"errno = %d\r\n", errno);
        perror("opendir");
        exit(-1);
    }
    return 0;
}
