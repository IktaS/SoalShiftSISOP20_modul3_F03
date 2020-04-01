#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#define POKEZONE_PORT 8080
#define SHOP_PORT 8090

typedef struct shop{
    int lull_pow;
    int pokeball;
    int berry;
}shop_stock;

pthread_mutex_t lock1,lock2;

void request_parser(char * buffer, char *request[]){
    char * token = strtok(buffer," | ");
    strcpy(request[0],token);
    token = strtok(NULL," | ");
    strcpy(request[1],token);
}

void get_pokemon(char * buffer){

    char name[5][10][20] = {
        {"Bulbasaur","Charmander","Squirtle","Rattata","Caterpie"},
        {"Pikachu","Eevee","Jigglypufff","Snorlax","Dragonite"},
        {"Mew","Mewtwo","Moltres","Zapdos","Articuno"}
    };
    srand(time(NULL));
    int type = rand()%100;
    int shiny = rand()%8000;
    if(type < 80){
        if(shiny == 0){
            int name_rand = rand()%5;
            strcat(buffer,"10 20 50 5080 ");
            strcat(buffer,"shiny_");
            strcat(buffer,name[0][name_rand]);
        }else{
            int name_rand = rand()%5;
            strcat(buffer,"5 20 70 80 ");
            strcat(buffer,name[0][name_rand]);
        }
    }
    if(type < 95 && type >= 80){
        if(shiny == 0){
            int name_rand = rand()%5;
            strcat(buffer,"15 20 30 5100 ");
            strcat(buffer,"shiny_");
            strcat(buffer,name[1][name_rand]);
        }else{
            int name_rand = rand()%5;
            strcat(buffer,"10 20 50 100 ");
            strcat(buffer,name[1][name_rand]);
        }
    }
    if(type < 100 && type >=95){
        if(shiny == 0){
            int name_rand = rand()%5;
            strcat(buffer,"25 20 10 5200 ");
            strcat(buffer,"shiny_");
            strcat(buffer,name[2][name_rand]);
        }else{
            int name_rand = rand()%5;
            strcat(buffer,"5 20 30 200 ");
            strcat(buffer,name[2][name_rand]);
        }
    }
}

void* pokemon_api(void* args){
    char * buffer = (char *) args;
    while(1){
        pthread_mutex_lock(&lock1);
        memset(buffer,0,sizeof(buffer));
        get_pokemon(buffer);
        pthread_mutex_unlock(&lock1);
        sleep(1);
    }
}

void* shop_api(void* args){
    shop_stock * stock = (shop_stock*) args;
    while(1){
        sleep(10);
        pthread_mutex_lock(&lock2);
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
        pthread_mutex_unlock(&lock2);
    }
}

void* runserver(void* args){
    pthread_t poke_thread, shop_thread;

    if (pthread_mutex_init(&lock1, NULL) != 0) {
        printf("\n mutex init has failed\n");
        // return 1;
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&lock2, NULL) != 0) {
        printf("\n mutex init has failed\n");
        // return 1;
        exit(EXIT_FAILURE);
    }

    char cur_pokemon[2048];
    int iret = pthread_create(&poke_thread, NULL, pokemon_api, (void*) cur_pokemon);
    if(iret) //jika eror
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    shop_stock * stock = (shop_stock *)malloc(sizeof(shop_stock));
    stock->lull_pow = 100;
    stock->pokeball = 100;
    stock->berry = 100;

    int iret2 = pthread_create(&shop_thread, NULL, shop_api, (void*)stock);
    if(iret2) //jika eror
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
        exit(EXIT_FAILURE);
    }




    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(POKEZONE_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    char buffer[2048] = {0};
    char * success = "success";
    char * failed = "failed";

    while(1){
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        int valread = read(new_socket,buffer,2048);

        char request[2][512];
        char * token = strtok(buffer," | ");
        strcpy(request[0],token);
        token = strtok(NULL," | ");
        strcpy(request[1],token);

        if(strcmp(request[0], "poke") == 0){
            send(new_socket,cur_pokemon,strlen(cur_pokemon),0);
        }else if(strcmp(request[0],"shop")==0){
            if(strcmp(request[1],"lull_pow")==0){
                if(stock->lull_pow > 0){
                    pthread_mutex_lock(&lock2);
                    stock->lull_pow -= 1;
                    pthread_mutex_unlock(&lock2);
                    send(new_socket,success,strlen(success),0);
                }else{
                    send(new_socket,failed,strlen(failed),0);
                }
            }
            if(strcmp(request[1],"pokeball")==0){
                if(stock->pokeball > 0){
                    pthread_mutex_lock(&lock2);
                    stock->pokeball -= 1;
                    pthread_mutex_unlock(&lock2);
                    send(new_socket,success,strlen(success),0);
                }else{
                    send(new_socket,failed,strlen(failed),0);
                }
            }
            if(strcmp(request[1],"berry")==0){
                if(stock->berry > 0){
                    pthread_mutex_lock(&lock2);
                    stock->berry -= 1;
                    pthread_mutex_unlock(&lock2);
                    send(new_socket,success,strlen(success),0);
                }else{
                    send(new_socket,failed,strlen(failed),0);
                }
            }
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
        char *argv[] = {"killall","soal2_pokezone","soal2_traizone",NULL};
        execv("/usr/bin/killall",argv);
    }else{
        return;
    }
}


int main(int argc, char const *argv[]) {
    pthread_t server_thread;

    int iret = pthread_create(&server_thread,NULL,runserver,NULL);
    if(iret) //jika eror
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    char input;
    printf("Server is running ...\n");
    printf("Press any button to shutdown\n");
    scanf("%c",&input);
    forkAndKillAll();
}