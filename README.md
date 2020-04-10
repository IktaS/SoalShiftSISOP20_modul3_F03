# SoalShiftSISOP20_modul3_F03

# Soal1

  Pada soal 1 diminta untuk membuat sebuah permainan berbasis teks yang mirip dengan PokemonGo. Permainan terdiri dari 2 buah code, yaitu soal1_traizone.c dan soal1_pokezone.c.

# a) soal1_traizone.c
     Link menuju kode: https://github.com/IktaS/SoalShiftSISOP20_modul3_F03/blob/master/soal1/soal1_traizone.c
     Pada soal1_traizone.c, diminta untuk membuat 2 buah mode: normal mode dan capture mode/

# Normal mode / main menu
     Berikut adalah fungsi main dari soal1_traizone:

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
    curscene->deact_lull = 0;
    pthread_mutex_init(&(curscene->lull_lock),NULL);
    pthread_mutex_init(&(curscene->state_lock),NULL);
    pthread_mutex_init(&(curscene->pokedex_lock),NULL);
    // printf("current pokemon : %s\n",cur_pokemon->name);
    int input;
    pthread_t render_thread;
    int iret = pthread_create(&render_thread,NULL,render_scene,(void*)curscene);
    if(iret){
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    while(1){
        input = getch();
        input = input -'0';
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

Sesuai dengan,
```
int iret = pthread_create(&render_thread,NULL,render_scene,(void*)curscene);
```
maka yang pertamakali akan ditampilkan secara default adalah main menu sesuai fungsi
```
void* render_scene(void* args){
    struct timespec ts;
    int rs;
    ts.tv_sec = 1/2;
    ts.tv_nsec = 500000000;
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
        rs = nanosleep(&ts,&ts);
        // sleep(1);
    }
}
```
dimana `render_mainmenu(curscene);` adalah:
```
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
```
yang akan menampilkan pilihan untuk: 1) Mencari/berhenti mencari pokemon, 2) Pokedex, dan 3) Shop.
Di saat yang bersaman, fungsi main akan tetap berjalan dan akan merubah tampilan berdasarkan input yang dimasukkan, karena secara default fungsi `input_mainmenu(curscene, input)` berjalan bersamaan dengan fungsi `render_mainmenu(curscene)` tadi.
`input_mainmenu(curscene)` sendiri berisi:
```
void input_mainmenu(pokezone_scene * curscene, int input){
    switch (input)
    {
    case 1:
        // printf("1. Berhenti mencari\n");
        if(*(curscene->state_cari) == 0) {
            pthread_mutex_lock(&(curscene->state_lock));
            *(curscene->state_cari) = 1;
            pthread_mutex_unlock(&(curscene->state_lock));
            start_cari_pokemon(curscene);
            break;
        }
        if(*(curscene->state_cari) == 1){
            pthread_mutex_lock(&(curscene->state_lock));
            *(curscene->state_cari) = 0;
            pthread_mutex_unlock(&(curscene->state_lock));
            break;
        }
        break;
    case 2:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 2;
        pthread_mutex_unlock(&(curscene->state_lock));
        break;
    case 3:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 3;
        pthread_mutex_unlock(&(curscene->state_lock));
        break;
    }
}
```
# Capture Mode & goto(scene peralihan)
Apabila pengguna memberi input "1" pada main menu, case 1 pada `input_mainmenu` akan menjalankan fungsi `start_cari_pokemon(curscene)` yang berisi:
```
void start_cari_pokemon(pokezone_scene * curscene){
    pthread_t thread_nyari;

    int iret = pthread_create(&thread_nyari,NULL,cari_pokemon,(void*)curscene);
    if(iret){
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

}
```
yang akan membuat thread baru yang berfungsi menjalankan:
```
void * cari_pokemon(void* args){
    pokezone_scene * passed_scene = (pokezone_scene*)args;
    while(1){
        if(passed_scene->state_cari == 0) break;
        int rando = rand()%10;
        if(rando < 6){
            memcpy(passed_scene->capturedmode_pokemon,passed_scene->available_pokemon,sizeof(pokemon));
            pthread_mutex_lock(&(passed_scene->state_lock));
            *(passed_scene->state_menu) = 5;
            pthread_mutex_unlock(&(passed_scene->state_lock));
            *(passed_scene->state_cari) = 0;
            break;
        }
        sleep(10);
    }
}
```
yang berfungsi mencari pokemon tiap 10 detik di belakang layar dengan cara menggunakan fungsi `rand()` untuk membuat angka random, yang akan dicocokkan dengan parameter yang diinginkan, sementara `render_mainmenu` akan menampilkan "1. Berhenti mencari" alih-alih "1. Cari Pokemon" selama fungsi `cari_pokemon` masih bekerja.
Apabila pada kondisi ini pengguna memberi input "1" lagi, maka pokemon akan berhenti dicari, dan tampilan akan kembali seperti semula.
Apabila `cari_pokemon` berhasil menemukan pokemon, maka case 5 pada `render_scene` akan aktif dan menjalankan:
```
case 5:
                render_goto(curscene);
                break;
```
dimana `render_goto(curscene)` adalah:
```
void render_goto(pokezone_scene * curscene){
    printf("Ketemu Pokemon!\n");
    printf("1. Pergi ke capture mode\n");
    printf("2. Kembali ke menu sebelumnya\n");
}
```
yang akan menampilkan pilihan berikutnya. Disaat yang bersamaan, case 5 pada `main` akan aktif di belakang layar dan menjalankan:
```
case 5:
            input_goto(curscene,input);
            break;
```
dimana `input_goto` adalah:
```
void input_goto(pokezone_scene * curscene, int input){
    // int before = *(curscene->state_menu);
    switch (input)
    {
    case 1:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 1;
        pthread_mutex_unlock(&(curscene->state_lock));
        break;
    
    case 2:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        *(curscene->state_cari) = 0;
        break;
    }
}
```
yang berfungsi mengolah input pada menu ini:
[FOtO]
Apabila pengguna memberi input "2", maka tampilan akan kembali seperti semula, seperti sebelum pokemon dicari.
Apabila pengguna memberi input "1", maka case 1 pada `render_scene` akan aktif dan menjalankan `render_capturemode(curscene)`
```
case 1:
                render_capturemode(curscene);
                break;
```
dimana `render_capturemode(curscene)` adalah:
```
void render_capturemode(pokezone_scene * curscene){
    if(curscene->capturedmode_pokemon != NULL)
        printf("Kamu bertemu dengan %s  --- %d\n",curscene->capturedmode_pokemon->name,curscene->capturedmode_pokemon->capture + 20 * (curscene->lullaby_state));
    if(curscene->lullaby_state != 0)
        printf("Lullaby aktif\n");
    printf("Pilih aksi :\n");
    printf("1. Tangkap\n");
    printf("2. Use Lullaby Powder\n");
    printf("3. To Mainmenu\n");
    int rando = rand()%100;
    if(rando < curscene->capturedmode_pokemon->escape && curscene->lullaby_state != 1){
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        *(curscene->state_cari) = 0;
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 0;
        pthread_mutex_unlock(&(curscene->lull_lock));
        curscene->deact_lull = 1;
    }
}
```
yang akan menampilkan tampilan 
[MASUKIN GAMBAR]
sekaligus mengacak kemungkinan apakah pokemon akan kabur dengan `int rando`. Apabila hasil yang diberikan memenuhi parameter, maka pokemon akan kabur, dan pemain akan dikembalikan ke tampilan semua.
Selama pokemon masih belum kabur, case 1 pada fungsi main akan berjalan di belakang layar dan menjalankan
```
case 1:
            input_capturemode(curscene, input);
            break;
```
dimana `input_capturemode` bertugas memproses input pada kondisi ini.
```
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
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 0;
        pthread_mutex_unlock(&(curscene->lull_lock));
        curscene->deact_lull = 1;
        break;
    }
}
```
dengan `start_capture_mode`:
```
void start_capture_mode(pokezone_scene * curscene){
    pthread_t pokemon_thread;
    curscene->deact_lull = 0;
    int iret = pthread_create(&pokemon_thread,NULL,calculate_escape_rate,(void*)curscene);
    if(iret){
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }
}
```
yang bertugas membuat thread baru yang menjalankan:
```
void* calculate_escape_rate(void* args){
    pokezone_scene * curscene = (pokezone_scene *)args;
    int escaperate = curscene->capturedmode_pokemon->escape;
    while(1){
        if(*(curscene->state_menu) != 1) break;
        curscene->capturedmode_pokemon->escape += escaperate;
        sleep(curscene->capturedmode_pokemon->rate);
    }
}
```
yang akan menghitung peluang pokemon kabur.
Pemain dapat memberi input "1" untuk mengaktifkan case 1 `input_capturemode` yang akan menjalankan:
```
void tangkap_action(pokezone_scene * curscene){
    if(curscene->player_stock->pokeball <= 0) {
        printf("habis");
        return;
    }
    int rando = rand()%100;
    if(rando < (curscene->capturedmode_pokemon->capture + 20 * curscene->lullaby_state)){
        if(curscene->sizepokedex >=7){
            curscene->pokedollar += curscene->capturedmode_pokemon->pokedollar;
            return;
        }
        pokemon * put_to_pokedex = (pokemon *)malloc(sizeof(pokemon));
        memcpy(put_to_pokedex,curscene->capturedmode_pokemon, sizeof(pokemon));
        pthread_mutex_lock(&(curscene->pokedex_lock));
        memcpy(curscene->pokedex + curscene->sizepokedex,put_to_pokedex,sizeof(pokemon));
        pthread_mutex_unlock(&(curscene->pokedex_lock));
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
        pthread_mutex_lock(&(curscene->pokedex_lock));
        curscene->sizepokedex++;
        pthread_mutex_unlock(&(curscene->pokedex_lock));
        curscene->player_stock->pokeball--;

        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        *(curscene->state_cari) = 0;

    }
}
```
Fungsi tersebut akan menampilkan "habis" apabila pokeball yang pemain miliki tidak mencukupi. Apabila pemain mempunyai cukup pokeball, maka akan dijalankan perhitungan untuk menentukan apakah pokemon berhasil ditangkap atau lolos. Apabila pokemon tertangkap, maka data akan diperbaharui. Pokeball pemain akan berkurang sebanyak 1 apapun hasil yang didapat, lalu pemain akan dikembalikan ke tampilan semula. Fungsi tersebut juga bertugas membuat thread baru `calculate_ap_pokemon`:
```
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
```
yang berfungsi menghitung ap pokemon di belakang layar. Tiap 10 detik, ap dari semua pokemon akan berkurang sebanyak 10. Apabila ap pokemon mencapai 0, maka akan dilakukan perhitungan untuk menentukan apakah pokemon tersebut akan dihilangkan dari kepemilikan pemain, atau justru pokemon tersebut akan mendapatkan 50 ap.
Apabila pemain memberi input "2" pada capturemode, maka case 2 akan aktif dan menjalankan:
```
case 2:
        activate_lull(curscene);
        break;
```
dimana `activate_lull` adalah:
```
void activate_lull(pokezone_scene * curscene){
    if(curscene->player_stock->lull_pow <= 0)return;
    pthread_t lull_thread;
    curscene->deact_lull = 0;
    int iret = pthread_create(&lull_thread,NULL,lull_powder,(void*)curscene);
    if(iret){
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }
    curscene->player_stock->lull_pow--;
}
```
yang akan mengecek lullaby powder milik pemain. Apabila lullaby powder milik pemain tidak mencukupi(0) maka tidak akan ada yang terjadi, namun apabila sebaliknya maka thread baru akan dibuat untuk menjalankan fungsi dibalik layar:
```
void *lull_powder(void* args){
    pokezone_scene * curscene = (pokezone_scene*)args;

    clock_t init;
    init = clock();
    while(1 && curscene->deact_lull != 1){
        init = clock() - init;
        int sec_taken =(int) ((int)init)/CLOCKS_PER_SEC;
        if(sec_taken == 10){
            break;
        }
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 1;
        pthread_mutex_unlock(&(curscene->lull_lock));
    }
    pthread_mutex_lock(&(curscene->lull_lock));
    curscene->lullaby_state = 0;
    pthread_mutex_unlock(&(curscene->lull_lock));
}
```
yang berfungsi menghitung lamanya lullaby powder aktif.
Selama lullaby powder aktif, pemain akan mendapatkan khasiat yang dapat dirasakan pada capturemode seperti berikut:
Pada `tangkap_action`, peluang tertangkap bertambah
```
...
if(rando < (curscene->capturedmode_pokemon->capture + 20 * curscene->lullaby_state)){
...

```
Pada `render_capturemode`, pokemon tidak akan kabur
```
...
if(rando < curscene->capturedmode_pokemon->escape && curscene->lullaby_state != 1){
...
```
Apabila pemain memberi input "3" pada capturemode, maka case 3 akan aktif dan menjalankan:
```
case 3:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        pthread_mutex_lock(&(curscene->lull_lock));
        curscene->lullaby_state = 0;
        pthread_mutex_unlock(&(curscene->lull_lock));
        curscene->deact_lull = 1;
        break;
    }
```
yang akan mematikan efek lullaby powder(jika ada), dan mengembalikan pemain ke main menu.

# Pokedex
Apabila pada main menu pengguna memasukkan input "2", maka case 2 akan aktif pada `main` dan `render_scene`.
Pada `main`:
```
 case 2:
            input_pokedex(curscene, input);
            break;
```
di mana `input_pokedex` adalah:
```
void input_pokedex(pokezone_scene * curscene, int input){
    switch (input)
    {
    case 1:
        give_berry(curscene);
        break;
    case 2:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 4;
        pthread_mutex_unlock(&(curscene->state_lock));
        break;
    case 3:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        break;
    }
}
```
yang berfungsi memproses input dari pengguna pada menu pokedex.
Pada `render_scene`, case 2 adalah:
```
 case 2:
                render_pokedex(curscene);
                break;
```
di mana `render_pokedex` adalah:
```
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
```
yang berfungsi memberi tampilan pada menu pokedex menjadi:
[GAMBER]
Apabila pengguna sudah menangkap pokemon, maka pokemon yang tersedia akan ditampilkan juga.
[Gambar]
Sesuai tampilan, pengguna dapat memberi input "1","2", atau "3", di mana nilai dari input tersebut akan memicu case sesuai input pada `input_pokedex`
Pada case 1, fungsi `give_berry` akan dijalankan, di mana `give_berry` adalah:
```
void give_berry(pokezone_scene * curscene){
    if(curscene->player_stock->berry <= 0) return;
    for(int i=0;i<curscene->sizepokedex;i++){
        pthread_mutex_lock(&(curscene->pokedex[i].lock));
        curscene->pokedex[i].ap += 10;
        pthread_mutex_unlock(&(curscene->pokedex[i].lock));
    }
    curscene->player_stock->berry--;
}
```
`give_berry` akan mengecek berry milik pemain dan tidak melakukan apa-apa apabila pemain tidak mempunya cukup berry(0). Kebalikannya, apabila berry yang pemain miliki mencukupi, semua pokemon milik pemain akan mendapatkan 10 ap, dan jumlah berry milik pemain akan berkurang sebanyak 1.
[before after]
Pada case 2, sesuai dengan `*(curscene->state_menu) = 4;`, maka case 4 untuk `main` dan `render_scene` akan aktif.
Pada `main`:
```
case 4:
            input_lepas(curscene, input);
            break;
```
`input_lepas` akan dijalankan, di mana `input_lepas` berisi:
```
void input_lepas(pokezone_scene * curscene, int input){
    curscene->pokedollar += curscene->pokedex[input-1].pokedollar;
    remove_pokemon_from_pokedex(curscene,input-1);

    pthread_mutex_lock(&(curscene->state_lock));
    *(curscene->state_menu) = 2;
    pthread_mutex_unlock(&(curscene->state_lock));
}
```
yang berfungsi memproses input dari pengguna pada menu tersebut.
Pada `render_scene`:
```
case 4:
                render_lepas(curscene);
                break;
```
di mana `render_lepas` adalah:
```
void render_lepas(pokezone_scene * curscene){
    for(int i=0; i<curscene->sizepokedex;i++){
        printf("%d. %s - %d\n",i+1,curscene->pokedex[i].name,curscene->pokedex[i].pokedollar);
    }
    printf("\n");
    printf("Pilih nomor lepas : \n");
}
```
yang berfungsi memberi tampilan pada menu tersebut:
[Gambar]
Pada kondisi ini, sesuai dengan `input_lepas`, pemain dapat memasukkan nomor urutan pokemon yang ingin dilepas sekaligus mendapatkan pokedollar. Setelah itu, tampilan akan kembali ke menu pokedex.
Pada case 3, sesuai dengan `*(curscene->state_menu) = 0;`, maka baik `main` maupun `render_scene` akan kembali ke case semula(0), yaitu main menu.

# Shop
Apabila pemain memasukkan input "3" pada main menu, case 3 dari `int` dan `render_scene` akan dijalankan:
Pada `main`:
```
case 3:
            input_shop(curscene, input);
            break;
```
di mana `input_shop`:
```
void input_shop(pokezone_scene * curscene, int input){
    switch (input)
    {
    case 1:
        //fungsi beli lullaby
        buy_lullaby(curscene);
        break;
    case 2:
        //fungsi belu pokeball
        buy_pokeball(curscene);
        break;
    case 3:
        //fungsi beli berry
        buy_berry(curscene);
        break;
    case 4:
        pthread_mutex_lock(&(curscene->state_lock));
        *(curscene->state_menu) = 0;
        pthread_mutex_unlock(&(curscene->state_lock));
        break;
    }
}
```
berfungsi mengolah input pemain pada menu shop.
Pada `render_scene`:
```
case 3:
                render_shop(curscene);
                break;
```
di mana `render_shop`:
```
void render_shop(pokezone_scene * curscene){
    printf("Pokedollar ---- %d\n",curscene->pokedollar);
    printf("Pilih barang :\n");
    printf("1. Lullaby Powder  ---- 60\n");
    printf("2. Pokeball        ---- 5\n");
    printf("3. Berry           ---- 15\n");
    printf("4. To Mainmenu\n");
}
```
berfungsi memberi tampilan pada menu shop, yang akan menampilkan pokedollar pemain dan item yang dapat dipilih.
Pada menu shop, pengguna dapat memasukkan input "1","2","3", atau "4", yang akan menjalankan case dengan kode serupa pada `input_shop`.
Pada case 1, `buy_lullaby` adalah:
```
void buy_lullaby(pokezone_scene * curscene){
    if(curscene->player_stock->lull_pow >= 99 || curscene->stock->lull_pow <= 0) return;
    if(curscene->pokedollar >= 60){
        curscene->pokedollar -= 60;
        curscene->player_stock->lull_pow++;
    }
}
```
yang berfungsi mengecek stok lullaby powder pada shop dan pada player. Tergantung dari hasil pengecekan, fungsi akan melakukan return atau melanjutkan pengecekan selanjutnya terhadap pokedollar pemain. Apabila pokedollar mencukupi, maka lullaby powder milik pemain akan bertambah sebanyak 1, dan pokedollar pemain berkurang sebanyak 60.
Pada case 2, `buy_pokeball` adalah:
```
void buy_pokeball(pokezone_scene * curscene){
    if(curscene->player_stock->pokeball >= 99 || curscene->stock->pokeball <= 0) return;
    if(curscene->pokedollar >= 5){
        curscene->pokedollar -= 5;
        curscene->player_stock->pokeball++;
    }
}
```
yang melakukan prosedur seperti pada lullaby powder, hanya saja terhadap variabel dan dengan nilai yang berbeda.
Pada case 3, `buy_berry` adalah:
```
void buy_berry(pokezone_scene * curscene){
    if(curscene->player_stock->berry >= 99 || curscene->stock->berry <= 0) return;
    if(curscene->pokedollar >= 15){
        curscene->pokedollar -= 15;
        curscene->player_stock->berry++;
    }
}
```
yang melakukan prosedur seperti pada lullaby powder dan pokeball, hanya saja terhadap variabel dan dengan nilai yang berbeda.
Pada case 4, sesuai dengan `*(curscene->state_menu) = 0;`, maka baik `main` maupun `render_scene` akan kembali ke case semula(0), yaitu main menu.

# b) soal1_pokezone.c
Link menuju kode: https://github.com/IktaS/SoalShiftSISOP20_modul3_F03/blob/master/soal1/soal1_pokezone.c
soal1_pokezone.c memiliki tugas untuk: 1) mematikan permainan, 2) menjual item, dan 3) menyediakan random pokemon
berikut adalah fungsi main dari soal1_pokezone.c:
```
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
```
Fungsi main akan membuat thread yang menjalankan `runserver`, sementara pada terminal fungsi main akan menampilkan "Server is running ..." dan "Press any button to shutdown", dimana ketika diberi input akan mematikan permainan dengan `forkAndKillAll()` :
```
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
```
yang akan membunuh semua "soal1_pokezone" dan "soal1_traizone".
Berikut adalah isi dari `runserver`:
```
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
```
dengan fungsi `pokemon_api`:
```
void* pokemon_api(void* args){
    pokemon * poke = (pokemon *) args;
    while(1){
        pthread_mutex_lock(&poke->lock);
        get_pokemon(poke);
        pthread_mutex_unlock(&poke->lock);
        sleep(1);
    }
}
```
dimana `get_pokemon(poke)` adalah:
```
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
```
yang bertugas menentukan pokemon yang akan muncul beserta atributnya.
dan `shop_api` adalah:
```
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
        pthread_mutex_unlock(&stock->lock);
    }
}
```
yang bertugas menjalankan shop beserta mengatur stocknya tiap 10 detik.

Soal2

Pada soal 2, diminta untuk membuat text based game dengan 2 program, yaitu server dan player.\
a) server.c
memiliki fungsi main:
```
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
```

b) client.c mempunyai fungsi main:
```
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
```

Soal 3

Pada nomor 3 diminta untuk membuat sebuah program yang dapat mengkategorikan file berdasarkan ekstensinya dan memindahkan file-file tersebut sesuai ekstesi ke dalam folder.
Berikut adalah fungsi main:
```
int main(int argc, char * argv[]){
    char curDir[PATH_MAX+1];
    getcwd(curDir,sizeof(curDir));
    // return 0;
    if(strcmp(argv[1],"-f")==0){
        pthread_t copy_thread[argc];
        for(int i=2;i<argc;i++){
            file_t * filenow = (file_t*)malloc(sizeof(file_t));
            filenow->curDir = curDir;
            char * copy = (char*)malloc(sizeof(char)*strlen(argv[i]));
            memset(copy,0,sizeof(char)*strlen(argv[i]));
            strcpy(copy,argv[i]);
            filenow->filename = copy;
            // printf("%s\n",get_filename_ext(filenow->filename));
            int iret = pthread_create(&copy_thread[i],NULL,checkFolderAndCopy,(void*)filenow);
            if(iret){
                perror("thread1");
                exit(EXIT_FAILURE);
            }
        }
        for(int i=2;i<argc;i++){
            pthread_join(copy_thread[i],NULL);
        }
    }else if(strcmp(argv[1],"*")==0){
        int count=0;
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d){
            char buffer[PATH_MAX +1];
            memset(buffer,0,sizeof(buffer));
            while ((dir = readdir(d)) != NULL){
                if(is_regular_file(dir->d_name)){
                    realpath(dir->d_name,buffer);
                    if(strcmp(curDir,buffer)== 0)continue;
                    count++;
                }else{
                }
            }
            closedir(d);
        }
        pthread_t copy_thread[count];
        int i=0;
        d = opendir(".");
        if (d){
            char buffer[PATH_MAX +1];
            memset(buffer,0,sizeof(buffer));
            while ((dir = readdir(d)) != NULL){
                if(is_regular_file(dir->d_name)){
                    realpath(dir->d_name,buffer);
                    if(strcmp(curDir,buffer)== 0)continue;
                    file_t * filenow = (file_t*)malloc(sizeof(file_t));
                    filenow->curDir = curDir;
                    char * copy = (char*)malloc(sizeof(char)*strlen(buffer));
                    memset(copy,0,sizeof(char)*strlen(argv[i]));
                    strcpy(copy,buffer);
                    filenow->filename = copy;
                    int iret = pthread_create(&copy_thread[i],NULL,checkFolderAndCopy,(void*)filenow);
                    if(iret){
                        perror("thread1");
                        exit(EXIT_FAILURE);
                    }
                    i++;
                }else{
                }
            }
            closedir(d);
        }
        for(i=0;i<count;i++){
            pthread_join(copy_thread[i],NULL);
        }
    }else if(strcmp(argv[1],"-d")==0){
        int count=0;
        DIR *d;
        struct dirent *dir;
        d = opendir(argv[2]);
        if (d){
            while ((dir = readdir(d)) != NULL){
                if(is_regular_file(dir->d_name)){
                    count++;
                }else{
                }
            }
            closedir(d);
        }
        pthread_t copy_thread[count];
        int i=0;
        d = opendir(argv[2]);
        if (d){
            char buffer[PATH_MAX +1];
            memset(buffer,0,sizeof(buffer));
            while ((dir = readdir(d)) != NULL){
                if(is_regular_file(dir->d_name)){
                    realpath(dir->d_name,buffer);
                    file_t * filenow = (file_t*)malloc(sizeof(file_t));
                    filenow->curDir = curDir;
                    char * copy = (char*)malloc(sizeof(char)*strlen(buffer));
                    memset(copy,0,sizeof(char)*strlen(buffer));
                    strcpy(copy,buffer);
                    filenow->filename = copy;
                    int iret = pthread_create(&copy_thread[i],NULL,checkFolderAndCopy,(void*)filenow);
                    if(iret){
                        perror("thread1");
                        exit(EXIT_FAILURE);
                    }
                }else{
                }
            }
            closedir(d);
        }
        for(i=0;i<count;i++){
            pthread_join(copy_thread[i],NULL);
        }
    }else{
        return 0;
    }
}
```
Akan tetapi, untuk program Soal nomor 3 ini masih memiliki kelemahan tidak dapat menangani karakter khusus seperti *

Soal 4

Pada soal 4, diminta untuk membuat 3 buah program yang dapat menjalankan fungsi sebagai berikut:\
a) Program 4a 
berfungsi untuk melakukan perkalian matrix dan menampilkan hasil yang didapatkan
```
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
```
angka yang hendak dihitung beserta ukuran matriks sudah berada di dalam program, sehingga apabila ingin merubah angka yang dihitung, bagian:
```
    threadstuff->matA[0][0] = 1;
	threadstuff->matA[0][1] = 2;
	threadstuff->matA[1][0] = 3;
	threadstuff->matA[1][1] = 4;
	threadstuff->matA[2][0] = 5;
	threadstuff->matA[2][1] = 6;
	threadstuff->matA[3][0] = 7;
	threadstuff->matA[3][1] = 8;
 ```
 dan
 ```
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
```
perlu diubah.\

b) Program 4b
yang berfungsi mengambil hasil perkalian program 4a dan menampilkannya, lalu menghitung dan menampilkan hasil penjumlahan dari n sampai 1 dari tiap angka pada matriks
```
void* tambah(void* args)
{
    thread_items * items = (thread_items*)args;
    pthread_mutex_lock(&(items->ilock));
    int temp = *(items->i);
    *(items->i) += 1;
    pthread_mutex_unlock(&(items->ilock));
    int sum = 0;
    for(int j=0;j<items->matinit[temp];j++){
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
            printf("%d ",items->mathasil[i+j]);
        }
        printf("\n");
    } 
    shmdt(mat);
    shmctl(shmid, IPC_RMID, NULL);
}
```

c) Program 4c
yang mengetahui jumlah file dan folder di direktori saat ini dengan command "ls | wc -l"
```
#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);
void ForkAndLSAndPipeOutput(int *linkout){
	pid_t pid;
	int status;
	pid = fork();
	if(pid < 0){
		die("forkandls");
	}
	if(pid == 0){
		//make the output pipes to linkout
		close(linkout[0]);
		dup2(linkout[1],STDOUT_FILENO);
		close(linkout[1]);
		char * argv[] = {"ls",NULL};
		execv("/usr/bin/ls",argv);
	}else{
		//make the input to linkin
		close(linkout[1]);
		dup2(linkout[0],STDIN_FILENO);
		close(linkout[0]);
		char * argv[] = {"wc","-l", NULL};
		execv("/usr/bin/wc",argv);
		return;
	}
}
int main() {
	int link1[2];
	pid_t pid;
	char out[4096];
	if (pipe(link1)<0)
		die("pipe1");
	ForkAndLSAndPipeOutput(link1);
	return 0;
}
```
