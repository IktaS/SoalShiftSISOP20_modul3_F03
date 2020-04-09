#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#define POKEMON_SHM    "/tmp/pokemon1"
#define SHOP_SHM       "/tmp/shop1"

typedef struct shop{
    pthread_mutex_t lock;
    pthread_mutexattr_t shrlock;
    int lull_pow;
    int pokeball;
    int berry;
}shop_stock;

typedef struct poke{
    pthread_mutex_t lock;
    pthread_mutexattr_t shrlock;

    int escape;
    int rate;
    int capture;
    int pokedollar;
    int ap;
    char name[2048];
}pokemon;



void get_pokemon(pokemon * buffer){
    memset(buffer->name,'\0',sizeof(buffer->name));


    char name[5][10][20] = {
        {"Bulbasaur","Charmander","Squirtle","Rattata","Caterpie"},
        {"Pikachu","Eevee","Jigglypuff","Snorlax","Dragonite"},
        {"Mew","Mewtwo","Moltres","Zapdos","Articuno"}
    };
    srand(time(NULL));
    int type = rand()%100;
    int shiny = rand()%8000;


    if(type < 80){
        if(shiny == 0){
            int name_rand = rand()%5;
            buffer->escape = 10;
            buffer->rate = 20;
            buffer->capture = 50;
            buffer->pokedollar = 5080;
            buffer->ap = 100;
            strcat(buffer->name,"shiny ");
            strcat(buffer->name,name[0][name_rand]);
        }else{
            int name_rand = rand()%5;
            buffer->escape = 5;
            buffer->rate = 20;
            buffer->capture = 70;
            buffer->pokedollar = 80;
            buffer->ap = 100;
            strcat(buffer->name,name[0][name_rand]);
        }
    }
    if(type < 95 && type >= 80){
        if(shiny == 0){
            int name_rand = rand()%5;
            buffer->escape = 15;
            buffer->rate = 20;
            buffer->capture = 30;
            buffer->pokedollar = 5100;
            buffer->ap = 100;
            strcat(buffer->name,"shiny ");
            strcat(buffer->name,name[1][name_rand]);
        }else{
            int name_rand = rand()%5;
            buffer->escape = 10;
            buffer->rate = 20;
            buffer->capture = 50;
            buffer->pokedollar = 100;
            buffer->ap = 100;
            strcat(buffer->name,name[1][name_rand]);
        }
    }
    if(type < 100 && type >=95){
        if(shiny == 0){
            int name_rand = rand()%5;
            buffer->escape = 25;
            buffer->rate = 20;
            buffer->capture = 10;
            buffer->pokedollar = 5200;
            buffer->ap = 100;
            strcat(buffer->name,"shiny ");
            strcat(buffer->name,name[2][name_rand]);
        }else{
            int name_rand = rand()%5;
            buffer->escape = 5;
            buffer->rate = 20;
            buffer->capture = 30;
            buffer->pokedollar = 200;
            buffer->ap = 100;
            strcat(buffer->name,name[2][name_rand]);
        }
    }
}

void* pokemon_api(void* args){
    pokemon * poke = (pokemon *) args;
    while(1){
        pthread_mutex_lock(&poke->lock);
        get_pokemon(poke);
        // printf("%s -- %d\n",poke->name,poke->ap);
        pthread_mutex_unlock(&poke->lock);
        sleep(1);
    }
}

void* shop_api(void* args){
    shop_stock * stock = (shop_stock*) args;
    while(1){
        sleep(10);
        pthread_mutex_lock(&stock->lock);
        if(stock->lull_pow <190){
            stock->lull_pow += 10;
        }else{
            stock->lull_pow += 200 - stock->lull_pow;
        }
        if(stock->pokeball <190){
            stock->pokeball += 10;
        }else{
            stock->pokeball += 200 - stock->pokeball;
        }
        if(stock->berry <190){
            stock->berry += 10;
        }else{
            stock->berry += 200 - stock->berry;
        }
        printf("lull pow : %d\n",stock->lull_pow);
        printf("pokeball : %d\n",stock->pokeball);
        printf("berry : %d\n",stock->berry);
        pthread_mutex_unlock(&stock->lock);

    }
}

void* runserver(void* args){
    pthread_t poke_thread, shop_thread;

    int * state = (int *) args;

    key_t key1 = 1234;
    pokemon * cur_pokemon;
    int shmid = shmget(key1,sizeof(pokemon),IPC_CREAT | 0666);
    cur_pokemon = (pokemon * )shmat(shmid,NULL,0);


    pthread_mutexattr_init(&cur_pokemon->shrlock);
    pthread_mutexattr_setpshared(&cur_pokemon->shrlock,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&cur_pokemon->lock,&cur_pokemon->shrlock);

    int iret = pthread_create(&poke_thread, NULL, pokemon_api, (void*) cur_pokemon);
    if(iret) //jika eror
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }


    key_t key2 = 5678;
    shop_stock * stock;
    int shmid1 = shmget(key2,sizeof(shop_stock),IPC_CREAT | 0664);
    stock = (shop_stock * )shmat(shmid1,NULL,0);


    pthread_mutexattr_init(&stock->shrlock);
    pthread_mutexattr_setpshared(&stock->shrlock,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&stock->lock,&stock->shrlock);

    stock->lull_pow = 100;
    stock->pokeball = 100;
    stock->berry = 100;

    int iret2 = pthread_create(&shop_thread, NULL, shop_api, (void*)stock);
    if(iret2) //jika eror
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
        exit(EXIT_FAILURE);
    }
    while(1){
        if(*state == 1){
            shmdt(cur_pokemon);
            shmctl(shmid,IPC_RMID,NULL);
            shmdt(stock);
            shmctl(shmid1,IPC_RMID,NULL);
        }
    }

}

void forkAndKillAll(){
    pid_t child_id;
    int status;

    child_id = fork();
    if(child_id < 0){
        exit(EXIT_FAILURE);
    }
    if(child_id == 0){
        char *argv[] = {"killall","soal1_pokezone","soal1_traizone",NULL};
        execv("/usr/bin/killall",argv);
    }else{
        return;
    }
}


int main(int argc, char const *argv[]) {
    pthread_t server_thread;

    int* state;
    *state = 0;
    int iret = pthread_create(&server_thread,NULL,runserver,(void*)state);
    if(iret) //jika eror
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    char input;
    printf("Server is running ...\n");
    printf("Press any button to shutdown\n");
    scanf("%c",&input);
    *state = 0;
    forkAndKillAll();
}