#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fts.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>

void tolowerstr(char * str){
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}

mode_t getumask()
{
    mode_t mask = umask(0);
    umask (mask);
    return mask;
}


typedef struct file{
    char * curDir;
    char * filename;
}file_t;


int is_regular_file( char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}


char * get_filename_ext(char *filename) {
    char * extname = (char*)malloc(sizeof(char)*(strlen(filename)));
    memset(extname,0,sizeof(char)*(strlen(filename)));
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename){
        strcpy(extname,"Unknown");
        return extname;
    }
    // if(strcmp(dot+1,"*")==0){
    //     strcpy(extname,"\\");
    //     strcat(extname,dot+1);
    //     tolowerstr(extname);
    //     return extname;
    // }
    // strcpy(extname,"'");
    strcpy(extname,dot+1);
    // strcat(extname,"'");
    tolowerstr(extname);
    printf("extension : %s\n",extname);
    return extname;
}

void* checkFolderAndCopy(void* args){
    // printf("enter\n");
    file_t * filenow = (file_t*)args;
    char * extensionName = get_filename_ext(filenow->filename);
    char * pathname = (char*)malloc(sizeof(char) * (strlen(filenow->curDir) + strlen(extensionName)));
    memset(pathname,0,sizeof(char) * (strlen(filenow->curDir) + strlen(extensionName)));
    strcpy(pathname,filenow->curDir);
    strcat(pathname,"/");
    strcat(pathname,extensionName);
    printf("making directory...%s\n",pathname);
    mkdir(pathname,0777);
    int val = mkdir(pathname,0777);
    if(val == EEXIST){
        printf("Directory made!\n");
    }
    strcat(pathname,"/");
    char buffer[PATH_MAX +1];
    memset(buffer,0,sizeof(buffer));
    strcpy(buffer,pathname);
    strcat(buffer,basename(filenow->filename));
    rename(filenow->filename,buffer);
    // remove(filenow->filename);
}


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