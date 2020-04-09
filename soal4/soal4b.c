#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/shm.h>
#include<string.h>
#include<sys/stat.h>


typedef struct mat{
    int * matinit;
    unsigned int mathasil[20];
    pthread_mutex_t ilock;
    pthread_mutex_t matlock;
    int * i;
}thread_items;



void* tambah(void* args)
{
    thread_items * items = (thread_items*)args;
    pthread_mutex_lock(&(items->ilock));
    int temp = *(items->i);
    *(items->i) += 1;
    pthread_mutex_unlock(&(items->ilock));
    int sum = 0;
    for(int j=0;j<=items->matinit[temp];j++){
        sum += j;
    }
    pthread_mutex_lock(&(items->matlock));
    items->mathasil[temp] = sum;
    pthread_mutex_unlock(&(items->matlock));
}

int main(void)
{
    int *mat;
    key_t key = 9876;
    int shmid = shmget(key, sizeof(int)*4*5, IPC_CREAT | 0666);
    mat = (int *)shmat(shmid, NULL, 0);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++)  {
            printf("%d ",mat[i*5+j]);
        }
        printf("\n");
    }
    pthread_t threads[20];

    thread_items * items = (thread_items*) malloc(sizeof(thread_items));
    items->matinit = mat;
    int i =0;
    items->i = &i;
    pthread_mutex_init(&(items->ilock),NULL);
    pthread_mutex_init(&(items->matlock),NULL);


    for (int i = 0; i < 20; i++){
        pthread_create(&threads[i], NULL, tambah, (void*)items);
    }

    for (int i = 0; i < 20; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++)  {
            printf("%d ",items->mathasil[i*5+j]);
        }
        printf("\n");
    }

    shmdt(mat);
    shmctl(shmid, IPC_RMID, NULL);

}