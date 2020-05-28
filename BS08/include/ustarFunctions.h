//
// Created by Nelson Morais on 24.05.20.
//

#ifndef BS08_USTARFUNCTIONS_H
#define BS08_USTARFUNCTIONS_H

#define NUMBER_BYTES 512
#define CHECK_USTAR "ustar"
#define USTAR_REGTYPE '0'            /* regular file */
#define USTAR_AREGTYPE '\0'          /* regular file */
#define USTAR_LNKTYPE '1'            /* Hard link */
#define USTAR_SYMTYPE '2'            /* Symbolic link */
#define USTAR_CHRTYPE '3'            /* character special */
#define USTAR_BLKTYPE '4'            /* block special */
#define USTAR_DIRTYPE '5'            /* directory */
#define USTAR_FIFOTYPE '6'           /* FIFO special */
#define USTAR_CONTTYPE '7'           /* Contiguous file  */

struct posix_header {
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    /* 500 */
} buffer;

int checkUstar();
void readTar(int fd);
int ISDIR();
int ISBLK();
int ISCHR();
int ISLNK();
int ISFIFO();
int ISREG();
int IRUSR();
int IWUSR();
int IXUSR();
int IRGRP();
int IWGRP();
int IXGRP();
int IROTH();
int IWOTH();
int IXOTH();
void formatModeBits(char *strMode);
void writeToConsole(int STD,char *str);


#endif //BS08_USTARFUNCTIONS_H
