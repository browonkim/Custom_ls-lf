// 시스템 프로그래밍/ 2021/ ls -Rl programming
// B735137/ 김형원

#define DEBUG

#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <grp.h>
#include <time.h>
#define SIZE 1024

//helper functions
void check_permission(char *permissionStatus, struct stat * statBuffer);
void check_type(char *typeStatus, struct stat * statBuffer);
void sort(char **list, int list_size);
void help_printDirMembers(char * dirName);

//primary function
void printDirMembers(char * dirName);

//global variables
char dirPath[SIZE];
char absolutePath[SIZE];

int todo; 


int main(int argc, char** argv){
    char currentWorkingDir[SIZE];
    getcwd(currentWorkingDir, SIZE);

    //dirPath initialization
    dirPath[0] = '.';
    dirPath[1] = '\0';

    printDirMembers(currentWorkingDir);
    return 0;

}

void check_type(char *typeStatus, struct stat * statBuffer){
    mode_t t = statBuffer->st_mode;
    if(S_ISREG(t)) typeStatus[0] = '-';
    else if(S_ISDIR(t)) typeStatus[0] = 'd';
    else if(S_ISCHR(t)) typeStatus[0] = 'c';
    else if(S_ISBLK(t)) typeStatus[0] = 'b';
    else if(S_ISSOCK(t)) typeStatus[0] = 's';
    else if(S_ISFIFO(t)) typeStatus[0] = 'p';
    else if(S_ISLNK(t)) typeStatus[0] = 'l';
}

void check_permission(char *permissionStatus, struct stat * statBuffer){
    permissionStatus[1] = statBuffer->st_mode & S_IRUSR ? 'r' : '-';
    permissionStatus[2] = statBuffer->st_mode & S_IWUSR ? 'w' : '-';
    permissionStatus[3] = statBuffer->st_mode & S_IXUSR ? 'x' : '-';
    permissionStatus[4] = statBuffer->st_mode & S_IRGRP ? 'r' : '-';
    permissionStatus[5] = statBuffer->st_mode & S_IWGRP ? 'w' : '-';
    permissionStatus[6] = statBuffer->st_mode & S_IXGRP ? 'x' : '-';
    permissionStatus[7] = statBuffer->st_mode & S_IROTH ? 'r' : '-';
    permissionStatus[8] = statBuffer->st_mode & S_IWOTH ? 'w' : '-';
    permissionStatus[9] = statBuffer->st_mode & S_IXOTH ? 'x' : '-';
}

int compare(char* s1, char* s2){
    //-_. compare
    char buf_s1[SIZE];
    char buf_s2[SIZE];
    char temp[SIZE];
    strcpy(temp, s1);
    char *temp_s1 = strtok(temp, "-_.");
    strcpy(buf_s1, temp_s1);
    while(temp_s1 != NULL){temp_s1 = strtok(NULL, "-_."); if(temp_s1 != NULL)strcat(buf_s1, temp_s1);}
    strcpy(temp, s2);
    char *temp_s2 = strtok(temp,"-_.");
    strcpy(buf_s2, temp_s2);
    while(temp_s2 != NULL){temp_s2 = strtok(NULL, "-_."); if(temp_s2 != NULL)strcat(buf_s2, temp_s2);}
    return strcmp(buf_s1, buf_s2);
}

//insertion Sort
void sort(char ** list, int list_size){
    //sort by name | increasing order
    printf("debug: [Sorting]\t list size = %d\n",list_size);
    char* key;
    int i, j;
    for(i=1; i<list_size; i++){
        key = list[i];       //key = list[i];
        j = i-1;
        while(j >=0 && (strcmp(list[j], key) == 1)){
            list[j+1] = list[j];     //list[j+1] = list[j]
            j = j-1;
        }
        list[j+1] = key;
    }
    for(i=0;i<list_size;i++){
        printf("%s, ",list[i]);
    }
    printf("\n");
}

//DFS
void printDirMembers(char * dirName){
    if(dirName == NULL)
        return;
    else{
        char absolutePrev[SIZE];
        char prevPath[SIZE];

        strcpy(absolutePath,dirName);
        printf("%s:\n", dirPath);
        printf("total %d\n", todo);
        DIR *dir = opendir(dirName);
        if(dir == NULL){
            printf("ERROR! I GUESS TAHT IS THE PERMISSON ERROR!\n");
            exit(1);
        }
        int list_capacity = 50;
        char **list = (char **)malloc(sizeof(char *)*list_capacity);
        int list_size = 0;
        
        struct dirent * rdir = NULL;
        while((rdir=readdir(dir))!=NULL){
            if(rdir->d_name[0] == '.') continue;    //숨김파일및디렉토리 . .. .git .gitignore .vim 등등
            else{
                list[list_size] = rdir->d_name;
                list_size++;
                if(list_size >= list_capacity){
                    list_capacity *= 2;
                    if(realloc(list, sizeof(char *)*list_capacity) == NULL)exit(1);
                }
            }
        }
        sort(list, list_size);
        int i;      
        char stat_String[] = "----------";
        time_t t;
        struct tm lt;
        long long stat_size;
        unsigned short stat_nlink;
        struct passwd *pws;
        struct group *grp;
        char timebuf[100];
        for(i=0;i<list_size;i++){
            struct stat getStat;
            stat(list[i], &getStat);
            check_type(stat_String, &getStat);
            check_permission(stat_String, &getStat);
            stat_nlink = (unsigned short)getStat.st_nlink;
            stat_size = (long long)getStat.st_size;
            pws = getpwuid(getStat.st_uid);
            grp = getgrgid(getStat.st_gid);
            t = getStat.st_mtime;
            localtime_r(&t, &lt);
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", &lt);
            //각 멤버의 타입/권한 링크수 사용자이름 사용자그룹 파일크기 수정한시각 파일/디렉토리이름
            printf("%s %3hd %s %s %10lld %s %s\n", stat_String, stat_nlink, pws->pw_name, grp->gr_name, stat_size, timebuf, list[i]);
        }
        printf("\n");
        for(i=0;i<list_size;i++){
            struct stat getStat;
            stat(list[i], &getStat);
            if(S_ISDIR(getStat.st_mode)){
                help_printDirMembers(list[i]);
            }
        }
        free(list);
        return;
    }
}

void help_printDirMembers(char * dirName){
    if(dirName == NULL)
        return;
    else{
        char absolutePrev[SIZE];
        char prevPath[SIZE];

        strcpy(prevPath, dirPath);
        strcpy(absolutePrev, absolutePath);
        strcat(absolutePath, "/");
        strcat(absolutePath, dirName);
        strcat(dirPath, "/");
        strcat(dirPath, dirName);
        printf("%s:\n", dirPath);
        printf("total %d\n", todo);
        DIR *dir = opendir(absolutePath);
        if(dir == NULL){
            printf("ERROR! I GUESS TAHT IS THE PERMISSON ERROR!\n");
            exit(1);
        }
        int list_capacity = 50;
        char **list = (char **)malloc(sizeof(char *)*list_capacity);
        if(list == NULL){printf("error!\n");exit(1);}
        //char* list[SIZE] = {NULL};
        int list_size = 0;
        struct dirent * rdir = NULL;
        while((rdir=readdir(dir))!=NULL){
            if(rdir->d_name[0] == '.') continue;    //숨김파일및디렉토리 . .. .git .gitignore .vim 등등
            else{
                list[list_size] = rdir->d_name;
                list_size++;
                if(list_size >= list_capacity){
                    list_capacity *= 2;
                    if(realloc(list, sizeof(char *)*list_capacity) == NULL)exit(1);
                }
            }
        }
        sort(list, list_size);
        int i;      
        char stat_String[] = "----------";
        long long stat_size;
        time_t t;
        struct tm lt;
        unsigned short stat_nlink;
        struct passwd *pws = NULL;
        struct group *grp = NULL;
        char temp_forPrev[SIZE];
        char timebuf[100];
        for(i=0;i<list_size;i++){
            struct stat getStat;
            strcpy(temp_forPrev, absolutePath);
            strcat(absolutePath,"/");
            strcat(absolutePath,list[i]);
            stat(absolutePath, &getStat);
            strcpy(absolutePath, temp_forPrev);
            check_type(stat_String, &getStat);
            check_permission(stat_String, &getStat);
            stat_nlink = (unsigned short)getStat.st_nlink;
            stat_size = (long long)getStat.st_size;
            pws = getpwuid(getStat.st_uid);
            if(pws == NULL){printf("ERROR!\n");exit(1);}
            grp = getgrgid(getStat.st_gid);
            if(grp == NULL){printf("ERROR!\n");exit(1);}
            t = getStat.st_mtime;
            localtime_r(&t, &lt);
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", &lt);
            //각 멤버의 타입/권한 링크수 사용자이름 사용자그룹 파일크기 수정한시각 파일/디렉토리이름
            printf("%s %3hd %s %s %10lld %s %s\n", stat_String, stat_nlink, pws->pw_name, grp->gr_name, stat_size, timebuf, list[i]);
            }
        printf("\n");
        for(i=0;i<list_size;i++){
            struct stat getStat;
            strcpy(temp_forPrev, absolutePath);
            strcat(absolutePath,"/");
            strcat(absolutePath,list[i]);
            stat(absolutePath, &getStat);
            strcpy(absolutePath,temp_forPrev);
            if(S_ISDIR(getStat.st_mode)){
                help_printDirMembers(list[i]);
            }
        }
        free(list);
        strcpy(absolutePath, absolutePrev);
        strcpy(dirPath, prevPath);
        return;
    }
}