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

typedef struct scene{
    pthread_mutex_t state_lock;
    int * state_menu;
    int * state_cari;

    shop_stock * stock;
    pokemon * available_pokemon;

    shop_stock * player_stock;
    pokemon * capturedmode_pokemon;

    int pokedollar;
    pokemon pokedex[10];
    int sizepokedex;

    pthread_mutex_t lull_lock;
    int lullaby_state;
}pokezone_scene;

typedef struct pokedexpoke{
    pokezone_scene * curscene;
    pokemon * pokemon;
    int index;
}pokedex_pokemon;

void render_mainmenu(pokezone_scene * curscene){
    printf("Pilih aksi :\n");
    switch (*(curscene->state_cari))
    {
        case 1:
            printf("1. Berhenti mencari\n");
            break;
        case 0:
            printf("1. Cari Pokemon\n");
            break;
    }
    printf("2. Pokedex\n");
    printf("3. Shop\n");
}

void render_capturemode(pokezone_scene * curscene){
    if(curscene->capturedmode_pokemon != NULL)
        printf("Kamu bertemu dengan %s\n",curscene->capturedmode_pokemon->name);
    printf("Pilih aksi :\n");
    printf("1. Tangkap\n");
    printf("2. Use Lullaby Powder\n");
    printf("3. To Mainmenu\n");
    int rando = rand()%100;
    if(rando < curscene->capturedmode_pokemon->escape && curscene->lullaby_state != 1){
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        *(curscene->state_cari) = 0;
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 0;
        pthread_mutex_unlock(&(curscene->lull_lock));
    }
}

void render_shop(pokezone_scene * curscene){
    printf("Pokedollar ---- %d\n",curscene->pokedollar);
    printf("Pilih barang :\n");
    printf("1. Lullaby Powder  ---- 60\n");
    printf("2. Pokeball        ---- 5\n");
    printf("3. Berry           ---- 15\n");
    printf("4. To Mainmenu\n");
}

void render_pokedex(pokezone_scene * curscene){
    for(int i=0; i<curscene->sizepokedex;i++){
        printf("%s - %d\n",curscene->pokedex[i].name,curscene->pokedex[i].ap);
    }
    printf("\n");
    printf("Pilih aksi :\n");
    printf("1. Beri Berry\n");
    printf("2. Lepas Pokemon\n");
    printf("3. To Mainmenu\n");
}

void render_lepas(pokezone_scene * curscene){
    for(int i=0; i<curscene->sizepokedex;i++){
        printf("%d. %s - %d\n",i+1,curscene->pokedex[i].name,curscene->pokedex[i].pokedollar);
    }
    printf("\n");
    printf("Pilih nomor lepas : ");
}

void render_goto(pokezone_scene * curscene){
    printf("Ketemu Pokemon!\n");
    printf("1. Pergi ke capture mode\n");
    printf("2. Kembali ke menu sebelumnya\n");
}

void * cari_pokemon(void* args){
    pokezone_scene * passed_scene = (pokezone_scene*)args;
    while(1){
        if(passed_scene->state_cari == 0) break;
        int rando = rand()%10;
        if(rando < 6){
            memcpy(passed_scene->capturedmode_pokemon,passed_scene->available_pokemon,sizeof(pokemon));
            // pthread_mutex_lock(&(passed_scene->state_lock));
            *(passed_scene->state_menu) = 5;
            // pthread_mutex_unlock(&(passed_scene->state_lock));
            *(passed_scene->state_cari) = 0;
            break;
        }
        sleep(10);
    }
}

void start_cari_pokemon(pokezone_scene * curscene){
    pthread_t thread_nyari;

    int iret = pthread_create(&thread_nyari,NULL,cari_pokemon,(void*)curscene);
    if(iret){
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

}

void input_mainmenu(pokezone_scene * curscene, int input){
    switch (input)
    {
    case 1:
        // printf("1. Berhenti mencari\n");
        if(*(curscene->state_cari) == 0) {
            // pthread_mutex_lock(&(curscene->state_lock));
            *(curscene->state_cari) = 1;
            // pthread_mutex_unlock(&(curscene->state_lock));
            start_cari_pokemon(curscene);
            break;
        }
        if(*(curscene->state_cari) == 1){
            // pthread_mutex_lock(&(curscene->state_lock));
            *(curscene->state_cari) = 0;
            // pthread_mutex_unlock(&(curscene->state_lock));
            break;
        }
        break;
    case 2:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 2;
        // pthread_mutex_unlock(&(curscene->state_lock));
        break;
    case 3:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 3;
        // pthread_mutex_unlock(&(curscene->state_lock));
        break;
    }
}

void* calculate_escape_rate(void* args){
    pokezone_scene * curscene = (pokezone_scene *)args;
    int escaperate = curscene->capturedmode_pokemon->capture;
    while(1){
        if(*(curscene->state_menu) != 1) break;
        curscene->capturedmode_pokemon->capture += escaperate;
        sleep(curscene->capturedmode_pokemon->rate);
    }
}

void remove_pokemon_from_pokedex(pokezone_scene * curscene,int index){
    for(int i = index; i < curscene->sizepokedex; i++){
        memcpy(curscene->pokedex + i,curscene->pokedex + i + 1,sizeof(pokemon));
    }
    curscene->sizepokedex--;
}
void start_capture_mode(pokezone_scene * curscene){
    pthread_t pokemon_thread;

    int iret = pthread_create(&pokemon_thread,NULL,calculate_escape_rate,(void*)curscene);
    if(iret){
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }
}
void* calculate_ap_pokemon(void* args){
    pokedex_pokemon * pokemon = (pokedex_pokemon *)args;
    while(1){
        if(*(pokemon->curscene->state_menu) == 1) continue;
        sleep(10);
        if(pokemon->pokemon->ap <= 0){
            int rando = rand()%10;
            if(rando == 0){
                pthread_mutex_lock(&(pokemon->pokemon->lock));
                pokemon->pokemon->ap = 50;
                pthread_mutex_unlock(&(pokemon->pokemon->lock));
            }else{
                remove_pokemon_from_pokedex(pokemon->curscene,pokemon->index);
            }
        }
        pthread_mutex_lock(&(pokemon->pokemon->lock));
        pokemon->pokemon->ap -= 10;
        pthread_mutex_unlock(&(pokemon->pokemon->lock));
    }
}

void tangkap_action(pokezone_scene * curscene){
    if(curscene->player_stock->pokeball <= 0) {
        printf("habis");
        return;
    }
    int rando = rand()%100;
    if(rando > (curscene->capturedmode_pokemon->capture + 20 * curscene->lullaby_state)){
        if(curscene->sizepokedex >=7){
            curscene->pokedollar += curscene->capturedmode_pokemon->pokedollar;
            return;
        }
        printf("ketangkap");
        pokemon * put_to_pokedex = (pokemon *)malloc(sizeof(pokemon));
        memcpy(put_to_pokedex,curscene->capturedmode_pokemon, sizeof(pokemon));
        memcpy(curscene->pokedex + curscene->sizepokedex,put_to_pokedex,sizeof(pokemon));

        pthread_mutex_init(&((curscene->pokedex + curscene->sizepokedex)->lock),NULL);

        pokedex_pokemon * pokedekusu = (pokedex_pokemon*)malloc(sizeof(pokedex_pokemon));
        pokedekusu->curscene = curscene;
        pokedekusu->pokemon = curscene->pokedex + curscene->sizepokedex;
        pokedekusu->index = curscene->sizepokedex;

        pthread_t new_thread;

        int iret = pthread_create(&new_thread,NULL,calculate_ap_pokemon,(void*)pokedekusu);
        if(iret){
            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
            exit(EXIT_FAILURE);
        }

        curscene->sizepokedex++;
        curscene->player_stock->pokeball--;

        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        *(curscene->state_cari) = 0;

    }
}
void *lull_powder(void* args){
    int init_time = time(NULL);
    pokezone_scene * curscene = (pokezone_scene*)args;

    while(1){
        if((init_time - time(NULL)) == 10){
            pthread_mutex_lock(&(curscene->lull_lock));
            curscene->lullaby_state = 0;
            pthread_mutex_unlock(&(curscene->lull_lock));
        }
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 1;
        pthread_mutex_unlock(&(curscene->lull_lock));
    }
}
void activate_lull(pokezone_scene * curscene){
    if(curscene->player_stock->lull_pow <= 0)return;
    pthread_t lull_thread;

    int iret = pthread_create(&lull_thread,NULL,lull_powder,(void*)curscene);
    if(iret){
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }
    curscene->player_stock->lull_pow--;
}
void input_capturemode(pokezone_scene * curscene, int input){
    start_capture_mode(curscene);
    switch (input)
    {
    case 1:
        tangkap_action(curscene);
        break;
    case 2:
        activate_lull(curscene);
        break;
    case 3:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        // pthread_mutex_unlock(&(curscene->state_lock));
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 0;
        pthread_mutex_unlock(&(curscene->lull_lock));
        break;
    }
}

void input_shop(pokezone_scene * curscene, int input){
    switch (input)
    {
    case 1:
        //fungsi beli lullaby
        break;
    case 2:
        //fungsi belu pokeball
        break;
    case 3:
        //fungsi beli berry
        break;
    case 4:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        // pthread_mutex_unlock(&(curscene->state_lock));
        break;
    }
}

void give_berry(pokezone_scene * curscene){
    for(int i=0;i<curscene->sizepokedex;i++){
        pthread_mutex_lock(&(curscene->pokedex[i].lock));
        curscene->pokedex[i].ap += 10;
        pthread_mutex_unlock(&(curscene->pokedex[i].lock));
    }
}

void input_pokedex(pokezone_scene * curscene, int input){
    switch (input)
    {
    case 1:
        give_berry(curscene);
        break;
    case 2:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 4;
        // pthread_mutex_unlock(&(curscene->state_lock));
        break;
    case 3:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        // pthread_mutex_unlock(&(curscene->state_lock));
        break;
    }
}

void input_lepas(pokezone_scene * curscene, int input){
    printf("%d %d\n",curscene->pokedollar,curscene->pokedex[input-1].pokedollar);
    curscene->pokedollar += curscene->pokedex[input-1].pokedollar;
    remove_pokemon_from_pokedex(curscene,input-1);

    // pthread_mutex_lock(&(curscene->state_lock));
    *(curscene->state_menu) = 2;
    // pthread_mutex_unlock(&(curscene->state_lock));
}

void input_goto(pokezone_scene * curscene, int input){
    // int before = *(curscene->state_menu);
    switch (input)
    {
    case 1:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 1;
        // pthread_mutex_unlock(&(curscene->state_lock));
        break;
    
    case 2:
        // pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        // pthread_mutex_unlock(&(curscene->state_lock));
        *(curscene->state_cari) = 0;
        break;
    }
}

void* render_scene(void* args){
    struct timespec ts;
    int rs;

    ts.tv_sec = 1/30;
    ts.tv_nsec = 33333333;
    pokezone_scene * curscene = (pokezone_scene *)args;
    while(1){
        system("clear");
        switch (*(curscene->state_menu)){
            case 1:
                render_capturemode(curscene);
                break;
            case 2:
                render_pokedex(curscene);
                break;
            case 3:
                render_shop(curscene);
                break;
            case 4:
                render_lepas(curscene);
                break;
            case 5:
                render_goto(curscene);
                break;

            default:
                render_mainmenu(curscene);
                break;
        }
        // usleep(166666);
        // rs = nanosleep(&ts,&ts);
        sleep(1);
    }
}


int main(int argc, char const *argv[]){
    srand(time(NULL));
    // system("/bin/stty raw");
    //state stuff
    int state_menu = 0;
    int state_cari = 0;


    //shared stuff
    key_t key1 = 1234;
    pokemon * cur_pokemon;
    int shmid = shmget(key1,sizeof(pokemon),IPC_CREAT | 0666);
    cur_pokemon = (pokemon * )shmat(shmid,NULL,0);

    key_t key2 = 5678;
    shop_stock * stock;
    int shmid1 = shmget(key2,sizeof(shop_stock),IPC_CREAT | 0664);
    stock = (shop_stock * )shmat(shmid1,NULL,0);

    pokemon captured_pokemon;

    shop_stock player_stock;
    player_stock.lull_pow = 10;
    player_stock.pokeball = 100;
    player_stock.berry = 0;

    pokezone_scene * curscene = (pokezone_scene *)malloc(sizeof(pokezone_scene));
    curscene->state_menu = &state_menu;
    curscene->state_cari = &state_cari;
    curscene->available_pokemon = cur_pokemon;
    curscene->capturedmode_pokemon = &captured_pokemon;
    curscene->stock = stock;
    curscene->player_stock = &player_stock;
    curscene->pokedollar = 0;
    curscene->sizepokedex = 0;
    curscene->lullaby_state = 0;
    pthread_mutex_init(&(curscene->lull_lock),NULL);
    pthread_mutex_init(&(curscene->state_lock),NULL);

    // printf("current pokemon : %s\n",cur_pokemon->name);
    int input;

    pthread_t render_thread;

    int iret = pthread_create(&render_thread,NULL,render_scene,(void*)curscene);
    if(iret){
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    while(1){
        scanf("%d",&input);
        switch (state_menu){
        case 1:
            input_capturemode(curscene, input);
            break;
        case 2:
            input_pokedex(curscene, input);
            break;
        case 3:
            input_shop(curscene, input);
            break;
        case 4:
            input_lepas(curscene, input);
            break;
        case 5:
            input_goto(curscene,input);
            break;

        default:
            input_mainmenu(curscene, input);
            break;
        }
    }
    // system("/bin/stty raw");
}