#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

typedef struct dirent dirent_t;
volatile char cwdStr [200];

void printDirEntryInfo(dirent_t *dirEntryPtr){
  if(dirEntryPtr){
    struct stat s;
    printf("file name = %s\r\n", dirEntryPtr->d_name);
    stat(dirEntryPtr->d_name,&s);
    printf("file size = %ld bytes\r\n", s.st_size/*dirEntryPtr->d_reclen*/);

    printf("file inode number = %ld\r\n", dirEntryPtr->d_ino);
    printf("file type: ");
    switch(dirEntryPtr->d_type){
      case DT_BLK : printf("Block device"); break;
      case DT_CHR : printf("Character device"); break;
      case DT_DIR : printf("Directory"); break;
      case DT_FIFO : printf("Named pipe (FIFO)"); break;
      case DT_LNK : printf("Symbolic link"); break;
      case DT_REG : printf("Regular file"); break;
      case DT_SOCK : printf("Socket"); break;
      default: printf("Unknown"); break;
    }
    printf("\r\n");
  }
  else{}
}

int main(int argc, char **argv){
  dirent_t *dirEntryPtr = 0x00;
  printf("Current process directory: %s\r\n", getcwd(cwdStr,sizeof(cwdStr)));

  DIR *dir = opendir(".");
  if(dir){
    while( (dirEntryPtr = readdir(dir) ) != NULL){
      printDirEntryInfo(dirEntryPtr);
    }
  }
  else{
    fprintf(stderr, "Could not read current directory\r\n");
    exit(-1);
  }
  return 0;
}
