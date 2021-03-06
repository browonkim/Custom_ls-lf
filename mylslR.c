// 시스템 프로그래밍/ 2021/ ls -Rl programming
// B735137/ 김형원

// #define DEBUG

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
#include <ctype.h>
#include <locale.h>
#define SIZE 4096
#define LINUXBLOCKSIZE 1024

#include <wchar.h>

//helper functions
void check_permission(char *permissionStatus, struct stat *statBuffer);
void check_type(char *typeStatus, struct stat *statBuffer);
int compare(char *s1, char *s2);
void sort(char **list, int list_size);
int isInGroup(char *username, char **groupList);
//primary function
void printDirMembers(char *dirName, int executePermission);
//global variables
char dirPath[SIZE];
char absolutePath[SIZE];
char myUserName[SIZE];

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    getlogin_r(myUserName, SIZE);
    printDirMembers(NULL, 1);
    return 0;
}

void check_type(char *typeStatus, struct stat *statBuffer)
{
    mode_t t = statBuffer->st_mode;
    if (S_ISREG(t))
        typeStatus[0] = '-';
    else if (S_ISDIR(t))
        typeStatus[0] = 'd';
    else if (S_ISCHR(t))
        typeStatus[0] = 'c';
    else if (S_ISBLK(t))
        typeStatus[0] = 'b';
    else if (S_ISSOCK(t))
        typeStatus[0] = 's';
    else if (S_ISFIFO(t))
        typeStatus[0] = 'p';
    else if (S_ISLNK(t))
        typeStatus[0] = 'l';
}

void check_permission(char *permissionStatus, struct stat *statBuffer)
{
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

int compare(char *s1, char *s2)
{
    int s1_length = strlen(s1);
    int s2_length = strlen(s2);
    int min = s1_length < s2_length ? s1_length : s2_length;
    int i, result;
    for (i = 0; i < min; i++)
    {
        if (tolower(s1[i]) > tolower(s2[i]))
        {
            return 1;
        }
        else if (tolower(s1[i]) < tolower(s2[i]))
        {
            return -1;
        }
    }
    if (min == s1_length && s1_length != s2_length)
        return -1;
    else if (min == s2_length && s1_length != s2_length)
        return 1;
    else
        return 0;
}

//insertion Sort
void sort(char **list, int list_size)
{
    int i, j;
    char *key;
    for (i = 1; i < list_size; i++)
    {
        key = list[i];
        j = i - 1;
        while (j >= 0 && compare(list[j], key) == 1)
        {
            list[j + 1] = list[j];
            j = j - 1;
        }
        list[j + 1] = key;
    }
}

int isInGroup(char *username, char **groupList)
{
    int i;
    for (i = 0; groupList[i] != NULL; i++)
    {
        if (strcmp(username, groupList[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

//DFS
void printDirMembers(char *dirName, int executePermission)
{
    char absolutePrev[SIZE];
    char prevPath[SIZE];

    if (dirName == NULL)
    {
        strcpy(prevPath, "");
        strcpy(absolutePrev, "");
        char strBuf[SIZE];
        getcwd(strBuf, SIZE);
        strcpy(dirPath, ".");
        strcpy(absolutePath, strBuf);
    }
    else
    {
        strcpy(prevPath, dirPath);
        strcpy(absolutePrev, absolutePath);
        //absolute Path
        strcat(absolutePath, "/");
        strcat(absolutePath, dirName);
        //Path for print
        strcat(dirPath, "/");
        strcat(dirPath, dirName);
    }
    char tempForDirPath[SIZE];
    long long total = 0;

    DIR *dir = opendir(absolutePath);
    if (dir == NULL)
    {
#ifdef DEBUG
        fprintf(stderr, "ERROR!! I GUESS THIS IS FILE NAME ERROR!\n");
        fprintf(stderr, "<%s>\n", absolutePath);
#endif
        strcpy(absolutePath, absolutePrev);
        strcpy(dirPath, prevPath);
        return;
    }

    int list_capacity = 50;
    char **list = (char **)malloc(sizeof(char *) * list_capacity);
    if (list == NULL)
    {
        fprintf(stderr, "ERROR! malloc FAILED!\n");
        exit(1);
    }

    int list_size = 0;
    struct dirent *rdir = NULL;
    char forCalcBlock[SIZE];
    struct stat forCalcStat;
    printf("%s:\n", dirPath);
    while ((rdir = readdir(dir)) != NULL)
    {
        if (rdir->d_name[0] == '.')
            continue; //숨김파일및디렉토리 . .. .git .gitignore .vim 등등
        else
        {
            list[list_size] = rdir->d_name;
            list_size++;
            strcpy(forCalcBlock, absolutePath);
            strcat(forCalcBlock, "/");
            strcat(forCalcBlock, rdir->d_name);
            if (executePermission)
            {
                if (lstat(forCalcBlock, &forCalcStat) >= 0)
                    total += (long long)(forCalcStat.st_blocks / 2); //st_blocks : 512byte blocks Centos/Ubuntu : 1024byte blocks
            }
            else
            {
                strcpy(tempForDirPath, dirPath);
                strcat(tempForDirPath, "/");
                strcat(tempForDirPath, rdir->d_name);
                fprintf(stderr, "ls: cannot access %s: Permission denied\n", tempForDirPath);
            }
            if (list_size >= list_capacity)
            {
                list_capacity *= 2;
                if ((list = (char **)realloc(list, sizeof(char *) * list_capacity)) == NULL)
                {
                    fprintf(stderr, "realloc Error!\n");
                    exit(1);
                }
            }
        }
    }

    fprintf(stdout, "total %lld\n", total);

    sort(list, list_size);

    int i;
    char stat_String[] = "----------";
    long long stat_size;
    time_t t;
    struct tm lt;
    unsigned short stat_nlink;
    struct passwd *pws = NULL;
    struct group *grp = NULL;
    char temp_forAbsolute[SIZE];
    char timebuf[100];
    if (executePermission)
    {
        for (i = 0; i < list_size; i++)
        {
            if (list[i][0] < 32 || list[i][0] > 126)
                continue;
            pws = NULL;
            grp = NULL;
            struct stat getStat;
            strcpy(temp_forAbsolute, absolutePath);
            strcat(temp_forAbsolute, "/");
            strcat(temp_forAbsolute, list[i]);
            if (lstat(temp_forAbsolute, &getStat) < 0)
                continue;
            check_type(stat_String, &getStat);
            check_permission(stat_String, &getStat);
            stat_nlink = (unsigned short)getStat.st_nlink;
            stat_size = (long long)getStat.st_size;
            pws = getpwuid(getStat.st_uid);
            if (pws == NULL)
            {
                continue;
            }
            grp = getgrgid(getStat.st_gid);
            if (grp == NULL)
            {
                continue;
            }
            t = getStat.st_mtime;
            localtime_r(&t, &lt);
            strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", &lt);
            //각 멤버의 타입/권한 링크수 사용자이름 사용자그룹 파일크기 수정한시각 파일/디렉토리이름
            if (S_ISCHR(getStat.st_mode) || S_ISBLK(getStat.st_mode))
            {
                int32_t majorDevNum, minorDevNum;
                majorDevNum = major(getStat.st_rdev);
                minorDevNum = minor(getStat.st_rdev);
                fprintf(stdout, "%s %3hd %s %s %5d, %5d %s %s", stat_String, stat_nlink, pws->pw_name, grp->gr_name, majorDevNum, minorDevNum, timebuf, list[i]);
            }
            else
            {
                fprintf(stdout, "%s %3hd %s %s %12lld %s %s", stat_String, stat_nlink, pws->pw_name, grp->gr_name, stat_size, timebuf, list[i]);
            }
            if (S_ISLNK(getStat.st_mode))
            {
                int readSize;
                char linkSource[SIZE];
                if ((readSize = readlink(temp_forAbsolute, linkSource, SIZE)) > 0)
                {
                    linkSource[readSize] = '\0';
                    fprintf(stdout, " -> %s", linkSource);
                }
                else
                {
                    //printf("error!");
                }
            }
            fprintf(stdout, "\n");
        }
    }
    else
    {
        for (i = 0; i < list_size; i++)
            fprintf(stdout, "-????????? ? ? ? ? ? %s\n", list[i]);
    }

    int flag, flagForExecute;
    for (i = 0; i < list_size; i++)
    {
        if (list[i][0] < 32 || list[i][0] > 126)
            continue;
        flag = 0;
        flagForExecute = 0;
        struct stat getStat;
        strcpy(temp_forAbsolute, absolutePath);
        strcat(temp_forAbsolute, "/");
        strcat(temp_forAbsolute, list[i]);
        if (lstat(temp_forAbsolute, &getStat) < 0)
        {
#ifdef DEBUG
            fprintf(stderr, "lstat error: %s\n", temp_forAbsolute);
#endif
            continue;
        }
        pws = getpwuid(getStat.st_uid);
        if (pws == NULL)
        {
            continue;
        }
        grp = getgrgid(getStat.st_gid);
        if (grp == NULL)
        {
            continue;
        }
        if (S_ISDIR(getStat.st_mode))
        {
            if (strcmp(pws->pw_name, myUserName) == 0 && getStat.st_mode & S_IRUSR)
                flag = 1;
            else if (getStat.st_mode & S_IROTH)
                flag = 1;
            else if (isInGroup(myUserName, grp->gr_mem) && getStat.st_mode & S_IRGRP)
                flag = 1;
            if (flag == 1)
            {
                if (strcmp(pws->pw_name, myUserName) == 0 && getStat.st_mode & S_IXUSR)
                    flagForExecute = 1;
                else if (getStat.st_mode & S_IXOTH)
                    flagForExecute = 1;
                else if (isInGroup(myUserName, grp->gr_mem) && getStat.st_mode & S_IXGRP)
                    flagForExecute = 1;
                printf("\n");
                printDirMembers(list[i], flagForExecute);
            }
            else
            {
                strcpy(tempForDirPath, dirPath);
                strcat(tempForDirPath, "/");
                strcat(tempForDirPath, list[i]);
                fprintf(stderr, "ls: cannot open directory %s: Permission denied\n", tempForDirPath);
            }
        }
    }
    free(list);
    closedir(dir);
    strcpy(absolutePath, absolutePrev);
    strcpy(dirPath, prevPath);
    return;
}