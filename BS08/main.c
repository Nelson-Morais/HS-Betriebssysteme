//
// Created by Nelson Morais on 24.05.20.
//

#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include "include/ustarFunctions.h"

int main(int argc, char *argv[]) {
    int fd;
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        printf("Datei kann nicht geöffnet werden.");
    } else  {
        read(fd, &buffer, NUMBER_BYTES);
    }

    if (checkUstar() == 1)
        readTar(fd);
    close(fd);
    return 0;
}

