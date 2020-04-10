#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define TRUE   1
#define FALSE  0
#define FILE_AKUN   "/home/ikta/akun.txt"
#define PORT 8080


typedef struct player_s{
    int socket_id;
    int * login;
    int * in_match;

    struct player_s * enemy;
}player_t;
typedef struct node_s{
    player_t * data;
    struct node_s * next;
}node_t;

typedef struct queue_s{
    struct node_s * back;
    struct node_s * front;
    int count;
}queue_t;

node_t * init_node(player_t * data){
    node_t * tmp = (node_t *)malloc(sizeof(node_t));
    tmp->data = data;
    tmp->next = NULL;
}

queue_t * init_queue(){
    queue_t * temp_queue = (queue_t *)malloc(sizeof(queue_t));
    temp_queue->count = 0;
    temp_queue->back = NULL;
    temp_queue->front = NULL;
    return temp_queue;
}


void enqueue(player_t * player, queue_t * queue){
    if(player == NULL) {
        printf("enqueueing null\n");
        return;
    }
    printf("%d is enqueued\n",player->socket_id);
    node_t * tmp = init_node(player);
    if(queue->back == NULL || queue->front == NULL){
        queue->front = tmp;
    }
    else if(queue->back == queue->front){
        queue->front->next = tmp;
    }
    else{
        queue->back->next = tmp;
    }
    queue->back = tmp;
    queue->count++;
}

player_t * dequeue(queue_t * queue){
    if(queue->front == NULL && queue->back == NULL)return NULL;
    node_t * temp = queue->front;
    player_t * tmp = temp->data;

    if(queue->front == queue->back){
        queue->front = NULL;
        queue->back = NULL;
    }else{
        queue->front = queue->front->next;
    }
    queue->count--;
    free(temp);
    return tmp;
}

void printQueue(queue_t * queue){
    node_t * temp = queue->front;
    while(temp != NULL){
        printf("%d ",temp->data->socket_id);
        temp = temp->next;
    }
    printf(" queue done printing, number of element : %d\n",queue->count);
}


typedef struct socket_s{
    int * server_fd, *new_socket;
    struct sockaddr_in * address;
    int * opt;
    int * addrlen;
}socket_t;

typedef struct server_s{
    socket_t * server_socket;
    queue_t * player_queue;
    pthread_mutex_t queue_lock;
}server_t;

typedef struct player_thread{
    player_t * player;
    server_t * server;
}player_args;

int readRequest(int fd,void * buf, size_t size){
    char buffer[2048];
    memset(buffer,0,sizeof(buffer));
    int val;
	if((val =read(fd, buffer, sizeof(buffer))) < 0){
		printf("read error");
		exit(EXIT_FAILURE);
	}
    // printf("reading %s\n",buffer);
	memcpy(buf, buffer, size);
    return val;
}

void sendResponse(int fd, void * buf, size_t size, int flag){
    char buffer[2048];
    memset(buffer,0,sizeof(buffer));
    memcpy(buffer,buf,size);
    // printf("sending %s\n",buffer);
    if(send(fd,buffer,sizeof(buffer),flag)< 0){
        perror("send error");
        exit(EXIT_FAILURE);
    }
}

void* match(void* args){
    server_t * serverMain = (server_t *) args;
    char * findmessage = "match_found";
    while(1){
        if(serverMain->player_queue->count >= 2){
            pthread_mutex_lock((&serverMain->queue_lock));
            // printf("dequque ing\n");
            player_t * player1 = dequeue(serverMain->player_queue);
            player_t * player2 = dequeue(serverMain->player_queue);
            player1->enemy = player2;
            player2->enemy = player1;
            sendResponse(player1->socket_id,findmessage,strlen(findmessage),0);
            sendResponse(player2->socket_id,findmessage,strlen(findmessage),0);
            *(player1->in_match) = 1;
            *(player1->in_match) = 1;
            pthread_mutex_unlock((&serverMain->queue_lock));
        }
    }
}

void add_account(char * username, char * password){
    FILE * temp = fopen(FILE_AKUN,"a+");
    char buffer[2048] = {0};
    strcat(buffer,username);
    strcat(buffer," | ");
    strcat(buffer,password);
    printf("key generated! %s\n",buffer);
    fprintf(temp,"%s\n",buffer);
    fclose(temp);
}

int check_account(char * username, char * password){
    char user_buf[100];
    char pass_buf[100];
    int auth = 0;
    FILE * temp = fopen(FILE_AKUN,"a+");
    while(fscanf(temp,"%s | %s",user_buf,pass_buf) != EOF){
        if(strcmp(username, user_buf) == 0){
            if(strcmp(password,pass_buf)==0){
                auth = 1;
                break;
            }
            continue;
        }
    }
    return auth;
}

void * player_handler(void* args){
    printf("player handler in\n");
    player_args * t_args = (player_args *)args;
    server_t * server = t_args->server;
    player_t * player = t_args->player;


    while(1){
        if(player == NULL || player->socket_id <=0 ){
            printf("wtf\n");
        }
        char buffer[2048];
        memset(buffer,0,sizeof(buffer));
        printf("try reading request\n");
        int val = read(player->socket_id,buffer,sizeof(buffer));
        printf("got request!  %s\n",buffer);
        if(val == 0)break;
        if(strcmp(buffer,"login")==0){

            char user_[100];
            char pass_[100];

            printf("reading username...\n");
            readRequest(player->socket_id,user_,sizeof(user_));
            printf("got username! %s\n",user_);

            printf("reading password...\n");
            readRequest(player->socket_id,pass_,sizeof(pass_));
            printf("got password! %s\n",pass_);

            int auth = check_account(user_,pass_);
            if(auth){
                char * message = "login_success";
                sendResponse(player->socket_id,message,strlen(message),0);
                *(player->login) = 1;
                fflush(stdout);
            }else{
                char * message = "login_failed";
                sendResponse(player->socket_id,message,strlen(message),0);
                *(player->login) = 0;
                fflush(stdout);
            }
        }
        if(strcmp(buffer,"register")==0){
            char user_[100];
            char pass_[100];


            printf("reading username...\n");
            readRequest(player->socket_id,user_,sizeof(user_));
            printf("got username! %s\n",user_);


            printf("reading password...\n");
            readRequest(player->socket_id,pass_,sizeof(pass_));
            printf("got password! %s\n",pass_);


            add_account(user_,pass_);
            char * registersuccess = "register_success";
            sendResponse(player->socket_id,registersuccess,strlen(registersuccess),0);
            char buffer[2048];
            char user_buf[100];
            char pass_buf[100];
            int auth = 0;
            FILE * temp = fopen(FILE_AKUN,"a+");
            while(fscanf(temp,"%s | %s",user_buf,pass_buf) != EOF){
                printf("%s | %s\n",user_buf,pass_buf);
            }
            // sendResponse(player->socket_id,buffer,2048,0);
        }

        if(strcmp(buffer,"find")==0 && *(player->login) == 1 && *(player->in_match) == 0){
            char * finding = "finding...";
            enqueue(player,server->player_queue);
            pthread_mutex_lock((&server->queue_lock));
            printQueue(server->player_queue);
            pthread_mutex_unlock((&server->queue_lock));
            printf("enqueue ok\n");
            while(player->enemy == NULL){
                sendResponse(player->socket_id,finding,strlen(finding),0);
                sleep(1);
            }
            char * findmessage = "match_found";
            sendResponse(player->socket_id,findmessage,strlen(findmessage),0);
            *(player->in_match) = 1;
        }

        if(strcmp(buffer,"shoot")==0 && *(player->in_match) == 1 && *(player->login) == 1 && player->enemy != NULL){
            char * damagemessage = "damage";
            if(player->enemy != NULL)
                sendResponse(player->enemy->socket_id,damagemessage,strlen(damagemessage),0);
        }

        if(strcmp(buffer,"logout")==0 && *(player->login) == 1 && *(player->in_match) == 0){
            *(player->login) = 0;
            char * logoutmessage = "logout";
            sendResponse(player->socket_id,logoutmessage,strlen(logoutmessage),0);
        }

        if(strcmp(buffer,"lose") == 0 && *(player->login) == 1 && *(player->in_match) == 1 && player->enemy != NULL){
            char * winid = "win";
            sendResponse(player->enemy->socket_id,winid,strlen(winid),0);
        }
        if(strcmp(buffer,"endmatch") == 0 && *(player->login) == 1 && *(player->in_match) == 1 && player->enemy != NULL){
            *(player->in_match) = 0;
            // *(player->enemy->in_match) = 0;
            player->enemy = NULL;
        }
    }
}

void* listen_thread(void* args){
    server_t * serverMain = (server_t *)args;
    int * server_fd = serverMain->server_socket->server_fd;
    struct sockaddr_in * address = serverMain->server_socket->address;
    int * addrlen = serverMain->server_socket->addrlen;
    while(1){

        printf("listening...\n");
        if( listen(*server_fd, 3) < 0){
            perror("listen");
            exit(EXIT_FAILURE);
        }

        int in_match = 0;
        int login = 0;
        player_t * temp_player = (player_t *)malloc(sizeof(player_t));
        temp_player->enemy = NULL;
        temp_player->in_match = &in_match;
        temp_player->login = &login;

        printf("accepting...\n");
        if((temp_player->socket_id = accept(*server_fd,(struct sockaddr *) address, (socklen_t*)addrlen)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("got player socket id %d\n",temp_player->socket_id);

        player_args * thread_args = (player_args*)malloc(sizeof(player_args));
        thread_args->player = temp_player;
        thread_args->server = serverMain;

        printf("making player thread...\n");
        pthread_t p_thread;

        int iret = pthread_create(&p_thread,NULL,player_handler,(void*)thread_args);
        if(iret){
            perror("p_thread");
            exit(EXIT_FAILURE);
        }
        printf("player thread made!\n");
    }
}


int main(){
    socket_t * server_socket = (socket_t *)malloc(sizeof(socket_t));
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    server_socket->server_fd = &server_fd;
    server_socket->new_socket = &new_socket;
    server_socket->address = &address;
    server_socket->opt = &opt;
    server_socket->addrlen = &addrlen;

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
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    server_t * serverMain = (server_t *)malloc(sizeof(server_t));
    serverMain->server_socket = server_socket;
    serverMain->player_queue = init_queue();
    pthread_mutex_init(&(serverMain->queue_lock),NULL);

    pthread_t listen_pt;
    int iret = pthread_create(&listen_pt,NULL,listen_thread,(void*)serverMain);
    if(iret){
        perror("listen thread");
        exit(EXIT_FAILURE);
    }
    printf("listen thread on\n");

    pthread_t match_pt;
    int iret1 = pthread_create(&match_pt,NULL,match,(void*)serverMain);
    if(iret1){
        perror("match thread");
        exit(EXIT_FAILURE);
    }
    printf("match thread on\n");

    pthread_join(listen_pt,NULL);
    pthread_join(match_pt,NULL);

    return 0;

}