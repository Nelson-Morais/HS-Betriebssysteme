#include <getopt.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>




const char *optArgs = "aglo";
int aFlag = 0, gFlag = 0, lFlag = 0, oFlag = 0;
DIR *dir;
struct stat st;

char *userIdToName(uid_t uid){
    struct passwd  *pw_ptr = getpwuid(uid);
    return pw_ptr->pw_name;
}
char *gIdToName(gid_t gid){
     struct group *gr_ptr = getgrgid(gid);
     return gr_ptr->gr_name;
}

void modeToStr(int mode, char *str) {
    strcpy(str, "----------");
    if (S_ISDIR(mode))
        str[0] = 'd';
    if (mode & S_IRUSR)
        str[1] = 'r';
    if (mode & S_IWUSR)
        str[2] = 'r';
    if (mode & S_IXUSR)
        str[3] = 'r';
    if (mode & S_IRGRP)
        str[4] = 'r';
    if (mode & S_IWGRP)
        str[5] = 'w';
    if (mode & S_IXGRP)
        str[6] = 'x';
    if (mode & S_IROTH)
        str[7] = 'r';
    if (mode & S_IWOTH)
        str[8] = 'w';
    if (mode & S_IXOTH)
        str[9] = 'x';

}

void list_files(char *dirPath) {
    struct dirent *dptr;
    char str[10];
    char *ctime();
    int count = 0;
    if ((dir = opendir(dirPath)) == NULL) {
        fprintf(stderr, "ls: can not open %s\n", dirPath);
        exit(EXIT_FAILURE);
    } else {
        while ((dptr = readdir(dir)) != NULL) {
            if (dptr->d_name[0] == '.' && !aFlag) {
                continue;
            }
            stat(dptr->d_name, &st);
            if (lFlag || oFlag || gFlag) {
                modeToStr((int) st.st_mode, str);
                printf("%s ", str);
                printf("%4d",(int)st.st_nlink);
                if(!gFlag)
                printf("%8s",userIdToName(st.st_uid));
                if(!oFlag)
                printf("%8s",gIdToName(st.st_gid));
                printf("%8d",(int)st.st_size);
                printf(" %.12s ", 4+ctime(&st.st_mtime));
            }
            count++;
            printf("  %s", dptr->d_name);
            printf("  ");
            if (lFlag || oFlag || gFlag) {
                printf("\n");
            }
        }
    }
    printf("\n");
    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt = 0, count = 0;
    char *dirPath;
    while ((opt = getopt(argc, argv, optArgs)) != -1) {
        switch (opt) {
            case 'a':
                aFlag = 1;
                break;
            case 'g':
                gFlag = 1;
                break;
            case 'l':
                lFlag = 1;
                break;
            case 'o':
                oFlag = 1;
                break;
        }


    }

    if (optind == argc) {
        dirPath = ".";
    } else {
        dirPath = argv[optind];

    }

     list_files(dirPath);

    return 0;
}

