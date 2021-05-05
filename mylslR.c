// 시스템 프로그래밍/ 2021/ ls -Rl programming
// B735137/ 김형원

#define DEBUG

#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
char prevPath[SIZE];
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

void sort(char ** list, int list_size){
    //sort by name | increasing order
}

void printDirMembers(char * dirName){
    if(dirName == NULL)
        return;
    else{
        printf("%s:\n", dirPath);
        printf("total %d\n", todo);
        //반복문을 써서 매개변수로 준 디렉토리의 멤버를 전부 조회한다.
        int list_capacity = 50;
        char **list = (char **)malloc(sizeof(char *)*list_capacity);
        int list_size = 0;
        DIR *dir = opendir(dirName);
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
        //이후 sort함수를 호출하여 이름순으로 정렬한다.
        sort(list, list_size);
        //각 멤버의 타입/권한 링크수 사용자이름 사용자그룹 파일크기 수정한시각 파일/디렉토리이름
        int i;      
        char stat_String[] = "----------";
        long long stat_size;
        unsigned short stat_nlink;
        for(i=0;i<list_size;i++){
            struct stat getStat;
            stat(list[i], &getStat);
            check_type(stat_String, &getStat);
            check_permission(stat_String, &getStat);
            stat_nlink = (unsigned short)getStat.st_nlink;
            stat_size = (long long)getStat.st_size;
            printf("%s %hd %lld %s\n",stat_String,stat_nlink,stat_size,list[i]);
        }
        printf("\n");
        //이후 다시 반복문을 써서 directory에 대해서만 함수 재귀 호출
        //printDirMemebers();
        //이것은 DFS임. Depth First Search
        for(i=0;i<list_size;i++){
            struct stat getStat;
            stat(list[i], &getStat);
            if(S_ISDIR(getStat.st_mode)){
                help_printDirMembers(list[i]);
            }
        }
        free(list);
        strcpy(dirPath, prevPath);
        return;
    }
}

void help_printDirMembers(char * dirName){
    if(dirName == NULL)
        return;
    else{
        strcpy(prevPath, dirPath);
        strcat(dirPath, "/");
        strcat(dirPath, dirName);
        printf("%s:\n", dirPath);
        printf("total %d\n", todo);
        //반복문을 써서 매개변수로 준 디렉토리의 멤버를 전부 조회한다.
        int list_capacity = 50;
        char **list = (char **)malloc(sizeof(char *)*list_capacity);
        int list_size = 0;
        DIR *dir = opendir(dirPath);
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
        //이후 sort함수를 호출하여 이름순으로 정렬한다.
        sort(list, list_size);
        //각 멤버의 타입/권한 링크수 사용자이름 사용자그룹 파일크기 수정한시각 파일/디렉토리이름
        int i;      
        char stat_String[] = "----------";
        long long stat_size;
        unsigned short stat_nlink;
        for(i=0;i<list_size;i++){
            struct stat getStat;
            stat(list[i], &getStat);
            check_type(stat_String, &getStat);
            check_permission(stat_String, &getStat);
            stat_nlink = (unsigned short)getStat.st_nlink;
            stat_size = (long long)getStat.st_size;
            printf("%s %hd %lld %s\n",stat_String,stat_nlink,stat_size,list[i]);
        }
        printf("\n");
        //이후 다시 반복문을 써서 directory에 대해서만 함수 재귀 호출
        //printDirMemebers();
        //이것은 DFS임. Depth First Search
        for(i=0;i<list_size;i++){
            struct stat getStat;
            stat(list[i], &getStat);
            if(S_ISDIR(getStat.st_mode)){
                help_printDirMembers(list[i]);
            }
        }
        free(list);
        strcpy(dirPath, prevPath);
        return;
    }
}