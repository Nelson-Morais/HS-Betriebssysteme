#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <string.h>
#include "include/web_request.c"
#include <sys/time.h>

#define maxChar 256
#define QUEUESIZE 30

int threadsize = 10;
char input[maxChar];
char *parse[maxChar];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int test;

typedef struct {
    char *buf[QUEUESIZE];
    long head, tail;
    int full, empty;

} queue;




int queueSize(queue *q) {
    int rtr = (q->tail) - (q->head);
    return rtr;
}

char *queuePrint(queue *q) {
    int i = q->head;
    int j = q->tail;
    char *tmp;
    while (i != j) {
        tmp = q->buf[i++];
        printf("%s\n", tmp);
    }
}

queue *queueInit(void) {
    queue *q;

    q = (queue *) malloc(sizeof(queue));
    if (q == NULL) return (NULL);

    q->empty = 1;
    q->full = 0;
    q->head = 0;
    q->tail = 0;

    return (q);
}

void queueDelete(queue *q) {
    free(q);
}

void queueDel(queue *q, char *out) {
    *out = (char) q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;


}

char *queueRead(queue *q) {
    char *out = q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return out;
}

void queueAdd(queue *q, char *in) {
    q->buf[q->tail] = strdup(in);
    q->tail++;
    if (q->tail == QUEUESIZE)
        q->tail = 0;
    if (q->tail == q->head)
        q->full = 1;
    q->empty = 0;


}

char strParse(char *input, char **parse) {

    for (int i = 0; i < maxChar; i++) {
        parse[i] = strsep(&input, " ");
        if (parse[i] == NULL) {
            break;
        }
        if (strlen(parse[i]) == 0) {
            i--;
        }
    }
}

void *readFd(void *q) {

    queue *q2 = (queue *) q;

    FILE *fd_in;
    fd_in = fopen(parse[0], "r");
    if (fd_in == NULL) {
        perror("failed to open input file");
        exit(EXIT_FAILURE);
    }

    char url[maxChar];
    char *pos;

    while (fgets(url, sizeof(url), fd_in) != NULL) {
        if ((pos = strchr(url, '\n')) != NULL) {
            *pos = '\0';
        }
        pthread_mutex_lock(&lock);
        queueAdd(q2, url);
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
    }
    return NULL;
}

void *writeFd(void *q) {
    queue *tmp = (queue *) q;
    char *argv[2];
    argv[0] = "--webreq-delay 0";
    argv[1] = "--webreq-path download";

    webreq_init(2, argv);
    int i = 0;

    pthread_mutex_lock(&lock);
    while (!(tmp->empty)) {
        char *url = strdup(queueRead(q));
        char *downloadUrl = strdup(url);


        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
        strtok(url, "/");
        char *domain = strtok(NULL, "/");

        int id = (int) pthread_self();
        printf("%d",id);

        char filename[64];
        snprintf(filename, sizeof(filename), "%i_%s.html", id, domain);
        printf("Downloading URL: %s\n", downloadUrl);

        webreq_download(downloadUrl, filename);
    }



}

int main() {
    char **args;
    int anzahlThreads;
    queue *q = NULL;


    printf("Filename:");
    fgets(input, MAX_INPUT, stdin);
    input[strcspn(input, "\n")] = '\0';
    if ((void *) input[0] == NULL || *input == ' ') {
        printf("No input");
        exit(0);
    } else {
        strParse(input, parse);
    }

    q = queueInit();

    pthread_t th;
    pthread_create(&th, NULL, readFd, q);
    pthread_join(th, NULL);


//download sites
    printf("Anzahl threads:");
    scanf("%d",&anzahlThreads);


    struct timeval tvbegin, tvend;
    gettimeofday(&tvbegin,NULL);

    pthread_t threadArr[anzahlThreads];
    for (int i = 0; i < anzahlThreads; i++) {
        printf("Create thread %d\n", i);
        pthread_create(&threadArr[i], NULL, writeFd, q);
    }

    for (int i = 0; i < anzahlThreads; i++) {
        pthread_join(threadArr[i], NULL);
    }

    gettimeofday(&tvend, NULL);

    printf("%lu", (tvend.tv_sec - tvbegin.tv_sec)*1000 +(tvend.tv_usec-tvbegin.tv_usec)/1000); //print duration
    printf("ms\n");

    return 0;

}

