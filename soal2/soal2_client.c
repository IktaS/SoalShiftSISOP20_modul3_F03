#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <termios.h>
#define PORT 8080

static struct termios old, current;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  current = old; /* make new settings same as old settings */
  current.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      current.c_lflag |= ECHO; /* set echo mode */
  } else {
      current.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void)
{
  return getch_(0);
}

/* Read 1 character with echo */
char getche(void)
{
  return getch_(1);
}


typedef struct socket_s{
    int * server_fd, *new_socket;
    struct sockaddr_in * address;
    int * opt;
    int * addrlen;
}socket_t;

typedef struct scene{
    int * scene_state;
    int * health;
    int * is_alive;
    int * is_win;
    int * is_over;
    socket_t * socket;
}scene_data;

void sendRequest(int fd, void * buf, size_t size, int flag){
    char buffer[2048] = {0};
    memcpy(buffer,buf,size);
    // printf("sending %s\n",buffer);
    if(send(fd,buffer,sizeof(buffer),flag)< 0){
        perror("send error");
        exit(EXIT_FAILURE);
    }
}

int readResponse(int fd,void * buf, size_t size){
    char buffer[2048] = {0};
    int val;
	if((val =read(fd, buffer, sizeof(buffer))) < 0){
		printf("read error");
		exit(EXIT_FAILURE);
	}
    // printf("reading %s\n",buffer);
	memcpy(buf, buffer, size);
    return val;
}


void render_mainmenu(scene_data * curscene){
    char request[2048];
    printf("1. Login\n");
    printf("2. Register\n");
    printf("   Choices : ");
    scanf("%s",request);
    if(strcmp(request,"login")==0){
        *(curscene->scene_state) = 1;
    }else if(strcmp(request,"register")==0){
        *(curscene->scene_state) = 2;
    }
}

void render_login(scene_data * curscene){
    char * request = "login";
    sendRequest(*(curscene->socket->server_fd),request,strlen(request),0);
    char userbuf[100];
    char passbuf[100];
    char buffer[2048] = {0};
    printf("Username : ");
    scanf("%s",userbuf);
    sendRequest(*(curscene->socket->server_fd),userbuf,strlen(userbuf),0);
    printf("Password : ");
    scanf("%s",passbuf);
    sendRequest(*(curscene->socket->server_fd),passbuf,strlen(passbuf),0);
    readResponse(*(curscene->socket->server_fd),buffer,sizeof(buffer));
    if(strcmp(buffer,"login_success")==0){
        printf("login success\n");
        *(curscene->scene_state) = 3;
    }else{
        printf("login failed\n");
        *(curscene->scene_state) = 0;
    }
}

void render_register(scene_data * curscene){
    char * request = "register";
    sendRequest(*(curscene->socket->server_fd),request,strlen(request),0);
    char userbuf[100];
    char passbuf[100];
    char buffer[2048] = {0};
    printf("Username : ");
    scanf("%s",userbuf);
    sendRequest(*(curscene->socket->server_fd),userbuf,strlen(userbuf),0);
    printf("Password : ");
    scanf("%s",passbuf);
    sendRequest(*(curscene->socket->server_fd),passbuf,strlen(passbuf),0);
    readResponse(*(curscene->socket->server_fd),buffer,sizeof(buffer));
    if(strcmp(buffer,"register_success")==0){
        printf("register success\n");
    }
    memset(buffer,0,sizeof(buffer));
    readResponse(*(curscene->socket->server_fd),buffer,sizeof(buffer));
    printf("%s",buffer);
    *(curscene->scene_state) = 0;
}

void render_menu2(scene_data * curscene){
    char request[2048];
    printf("1. Find Match\n");
    printf("2. Logout\n");
    printf("   Choices : ");
    scanf("%s",request);
    if(strcmp(request,"find")==0){
        *(curscene->scene_state) = 4;
    }else if(strcmp(request,"logout")==0){
        sendRequest(*(curscene->socket->server_fd),request,strlen(request),0);
        *(curscene->scene_state) = 0;
    }
}

void render_waiting(scene_data * curscene){
    char * request = "find";
    sendRequest(*(curscene->socket->server_fd),request,strlen(request),0);
    char buffer[2048] = {0};
    do{
        readResponse(*(curscene->socket->server_fd),buffer,sizeof(buffer));
        printf("Waiting for player....\n");
        sleep(1);
    }while(strcmp(buffer,"match_found")!=0);
    *(curscene->scene_state) = 5;
}

void* reading_thread(void* args){
    scene_data * curscene = (scene_data *)args;
    char buffer[2048] = {0};
    int win;
    char * lose = "lose";
    do{
        int val = readResponse(*(curscene->socket->server_fd),buffer,sizeof(buffer));
        if(val == 0)break;
        if(strcmp(buffer,"damage")==0){
            *(curscene->health) -= 10;
            if(*(curscene->health) <= 0){
                sendRequest(*(curscene->socket->server_fd),lose,strlen(lose),0);
                *(curscene->is_over) = 1;
                *(curscene->is_win) = 0;
                break;
            }
            continue;
        }
        if(strcmp(buffer,"win") == 0){
            *(curscene->is_over) = 1;
            *(curscene->is_win) = 1;
        }else if(strcmp(buffer,"lose")==0){
            *(curscene->is_over) = 1;
            *(curscene->is_win) = 0;
        }
    }while(*(curscene->is_over) == 0);
}

void render_match(scene_data * curscene){
    *(curscene->health) = 100;
    *(curscene->is_alive) = 1;
    *(curscene->is_win) = 0;
    *(curscene->is_over) = 0;

    pthread_t readingthread;

    int iret = pthread_create(&readingthread,NULL,reading_thread,(void*)curscene);
    if(iret){
        perror("thread error");
        exit(EXIT_FAILURE);
    }
    char * shoot = "shoot";
    printf("Game dimulai silahkan tap tap secepat mungkin !!\n");
    char input;

    while(*(curscene->is_over) == 0){
        input = getch();
        if(input == ' '){
            printf("%d health\n",*(curscene->health));
            sendRequest(*(curscene->socket->server_fd),shoot,strlen(shoot),0);
            printf("hit!!\n");
        }
    }
    pthread_join(readingthread,NULL);
    if(*(curscene->is_win) == 1){
        printf("Game berakhir kamu menang\n");
    }else if(*(curscene->is_win) == 0){
        printf("Game berakhir kamu kalah\n");
    }
    char * endgame = "endmatch";
    sendRequest(*(curscene->socket->server_fd),endgame,strlen(endgame),0);
    *(curscene->scene_state) = 3;
}


int main(int argc, char const *argv[]) {
    socket_t * server = (socket_t*)malloc(sizeof(socket_t));
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    server->address = &serv_addr;
    server->server_fd = &sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    int scene_state = 0;
    int health = 100;
    int is_alive = 1;
    int is_win = 0;
    int is_over = 0;
    scene_data * curscene = (scene_data *)malloc(sizeof(scene_data));
    curscene->scene_state = &scene_state;
    curscene->socket = server;
    curscene->health = &health;
    curscene->is_alive = &is_alive;
    curscene->is_win = &is_win;
    curscene->is_over = &is_over;

    while(1){
        switch (scene_state)
        {
        case 0:
            render_mainmenu(curscene);
            break;
        case 1:
            render_login(curscene);
            break;
        case 2:
            render_register(curscene);
            break;
        case 3:
            render_menu2(curscene);
            break;
        case 4:
            render_waiting(curscene);
            break;
        case 5:
            render_match(curscene);
            break;
        }
    }
}