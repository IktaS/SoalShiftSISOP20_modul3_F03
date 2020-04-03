#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<string.h>
#include<sys/stat.h>


typedef struct mat{
    int matA[4][2];
    int rowA;
    int colA;
    int matB[2][5];
    int rowB;
    int colB;
    pthread_mutex_t matClock;
    int matC[5][5];
    pthread_mutex_t ilock;
    int *i;
}thread_items;



void* multi(void* args)
{
    thread_items* threadstuff = (thread_items*)args;
    int sum =0;
    pthread_mutex_lock(&(threadstuff->ilock));
    int i = *(threadstuff->i);
    *(threadstuff->i) += 1;
    pthread_mutex_unlock(&(threadstuff->ilock));
    for(int j=0;j<5;j++){
        for (int k = 0; k < 2; k++) {
            sum = sum + threadstuff->matA[i][k] * threadstuff->matB[k][j];
        }
        pthread_mutex_lock(&(threadstuff->matClock));
        threadstuff->matC[i][j] = sum;
        pthread_mutex_unlock(&(threadstuff->matClock));
        sum = 0;
    }
\
}

int main(void)
{
    thread_items * threadstuff = (thread_items *)malloc(sizeof(thread_items));
    threadstuff->rowA = 4;
    threadstuff->colA = 2;

    threadstuff->matA[0][0] = 1;
	threadstuff->matA[0][1] = 2;
	threadstuff->matA[1][0] = 3;
	threadstuff->matA[1][1] = 4;
	threadstuff->matA[2][0] = 5;
	threadstuff->matA[2][1] = 6;
	threadstuff->matA[3][0] = 7;
	threadstuff->matA[3][1] = 8;

    threadstuff->rowB = 2;
    threadstuff->colB = 5;

    threadstuff->matB[0][0] = 1;
	threadstuff->matB[0][1] = 2;
	threadstuff->matB[0][2] = 3;
	threadstuff->matB[0][3] = 4;
	threadstuff->matB[0][4] = 5;
	threadstuff->matB[1][0] = 6;
	threadstuff->matB[1][1] = 7;
	threadstuff->matB[1][2] = 8;
	threadstuff->matB[1][3] = 9;
	threadstuff->matB[1][4] = 10;

    threadstuff->matC[0][0] = 0;

    int init=0;
    threadstuff->i = &init;
    pthread_mutex_init(&(threadstuff->ilock),NULL);
    pthread_mutex_init(&(threadstuff->matClock),NULL);


    pthread_t threads[5];

    for (int i = 0; i < 4; i++){
        pthread_create(&threads[i], NULL, multi, (void*)threadstuff);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    // return 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++)  {
            printf("%d ", threadstuff->matC[i][j]);
        }
        printf("\n");
    }

    int (*temp)[5];
    key_t key = 9876;
    int shmid = shmget(key, (sizeof(int[5][5])), IPC_CREAT | 0666);
    if(shmid < 0){
        perror("shmid");
        exit(EXIT_FAILURE);
    }
    temp = shmat(shmid,0,0);
    if(temp == (void*)-1){
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++)  {
            temp[i][j] = threadstuff->matC[i][j];
        }
    }
}