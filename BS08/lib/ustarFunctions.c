//
// Created by Nelson Morais on 25.05.20.
//


#include <zconf.h>
#include <string.h>
#include <time.h>

#define TIME_SIZE 32


#include "../include/ustarFunctions.h"




time_t parse_time(const char time[]) {
    time_t ret = 0;
    unsigned long mult = 1;
    for (int i = TIME_SIZE; i >= 0; i--) {
        if (time[i] != '\000') {
            ret += (time[i] - '0') * mult;
            mult *= 8;
        }
    }
    return ret;
}


void writeToConsole(int STD, char str[]) {
    write(STD, str, sizeof(char) * strlen(str));
}


int checkUstar() {
    for (int i = 0; i < sizeof(buffer.magic); i++) {
        if (buffer.magic[i] != CHECK_USTAR[i]) {
            return 0;
        }
    }
    return 1;
}


void readTar(int fd) {
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &buffer, NUMBER_BYTES) > 0) {

        if (checkUstar() == 1) {
            char strMode[10];
            formatModeBits(strMode);
            writeToConsole(STDOUT_FILENO, strMode);
            writeToConsole(STDOUT_FILENO, " ");
            writeToConsole(STDOUT_FILENO, buffer.uname);
            writeToConsole(STDOUT_FILENO, "/");
            writeToConsole(STDOUT_FILENO, buffer.gname);
            writeToConsole(STDOUT_FILENO, " ");
            writeToConsole(STDOUT_FILENO, buffer.size);
            writeToConsole(STDOUT_FILENO, " ");
            time_t time;
            time = parse_time(buffer.mtime);
            char timeStr[TIME_SIZE];
            strftime(timeStr, TIME_SIZE, "%Y-%m-%d %H:%M", localtime(&time));
            writeToConsole(STDOUT_FILENO, timeStr);
            writeToConsole(STDOUT_FILENO, " ");
            writeToConsole(STDOUT_FILENO, buffer.name);
            writeToConsole(STDOUT_FILENO, "\n");
        }
    }
}


int ISDIR() {
    return (buffer.typeflag == USTAR_DIRTYPE);
}

int ISBLK() {
    return (buffer.typeflag == USTAR_BLKTYPE);
}

int ISCHR() {
    return (buffer.typeflag == USTAR_CHRTYPE);
}

int ISLNK() {
    return (buffer.typeflag == USTAR_LNKTYPE);
}

int ISFIFO() {
    return (buffer.typeflag == USTAR_FIFOTYPE);
}

int ISREG() {
    return (buffer.typeflag == USTAR_REGTYPE || buffer.typeflag == USTAR_AREGTYPE);
}

int IRUSR() {
    return (buffer.mode[4] == '4' || buffer.mode[4] == '5' || buffer.mode[4] == '6' || buffer.mode[4] == '7');
}

int IWUSR() {
    return (buffer.mode[4] == '2' || buffer.mode[4] == '3' || buffer.mode[4] == '6' || buffer.mode[4] == '7');
}

int IXUSR() {
    return (buffer.mode[4] == '1' || buffer.mode[4] == '3' || buffer.mode[4] == '5' || buffer.mode[4] == '7');
}

int IRGRP() {
    return (buffer.mode[5] == '4' || buffer.mode[5] == '5' || buffer.mode[5] == '6' || buffer.mode[5] == '7');
}

int IWGRP() {
    return (buffer.mode[5] == '2' || buffer.mode[5] == '3' || buffer.mode[5] == '6' || buffer.mode[5] == '7');
}

int IXGRP() {
    return (buffer.mode[5] == '1' || buffer.mode[5] == '3' || buffer.mode[5] == '5' || buffer.mode[5] == '7');
}

int IROTH() {
    return (buffer.mode[6] == '4' || buffer.mode[6] == '5' || buffer.mode[6] == '6' || buffer.mode[6] == '7');
}

int IWOTH() {
    return (buffer.mode[6] == '2' || buffer.mode[6] == '3' || buffer.mode[6] == '6' || buffer.mode[6] == '7');
}

int IXOTH() {
    return (buffer.mode[6] == '1' || buffer.mode[6] == '3' || buffer.mode[6] == '5' || buffer.mode[6] == '7');
}


void formatModeBits(char *strMode) {
    if (ISDIR(buffer)) {
        strMode[0] = 'd';
    } else if (ISBLK()) {
        strMode[0] = 'b';
    } else if (ISCHR()) {
        strMode[0] = 'c';
    } else if (ISLNK()) {
        strMode[0] = 'l';
    } else if (ISFIFO()) {
        strMode[0] = 'p';
    } else if (ISREG()) {
        strMode[0] = '-';
    } else {
        strMode[0] = '?';
    }

    if (IRUSR()) {
        strMode[1] = 'r';
    } else {
        strMode[1] = '-';
    }

    if (IWUSR()) {
        strMode[2] = 'w';
    } else {
        strMode[2] = '-';
    }

    if (IXUSR()) {
        strMode[3] = 'x';
    } else {
        strMode[3] = '-';
    }

    if (IRGRP()) {
        strMode[4] = 'r';
    } else {
        strMode[4] = '-';
    }

    if (IWGRP()) {
        strMode[5] = 'w';
    } else {
        strMode[5] = '-';
    }

    if (IXGRP()) {
        strMode[6] = 'x';
    } else {
        strMode[6] = '-';
    }

    if (IROTH()) {
        strMode[7] = 'r';
    } else {
        strMode[7] = '-';
    }

    if (IWOTH()) {
        strMode[8] = 'w';
    } else {
        strMode[8] = '-';
    }

    if (IXOTH()) {
        strMode[9] = 'x';
    } else {
        strMode[9] = '-';
    }
    strMode[10] = '\0';

}