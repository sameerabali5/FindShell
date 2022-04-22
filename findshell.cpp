//sameera balijepalli
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <iostream>
#include <string>
using namespace std;
#include <fstream>
using std::ofstream;

#define BILLION  1000000000.0

int fd[2];
int ride = 0;
int count=0;

//for indentifying commands 
char command[1000]; //quit, kill, find, list
char type[1000]; //file or text 
char flag1[1000]; //-s
char flag2[1000]; //-f and -s
char command1[1000];
char type1[1000];
char flagv1[1000];
char flagv2[1000];
char findings[1000];

//for outputs
char output[1000];
int found = 0;

void removeQuotes(char *type, char *text)
{
    strncpy(text, type + 1, strlen(type));
    for(int i = 0; i < strlen(text); i++)
    {
        if(text[i] == '"')
        {
            text[i] = '\0';
            break;
        }
    }
}

void createFlags(char* input, int start, int i, int count)
{
    int len = i - start;
    char *p = input + start;
    if(count == 1)
    {
        strncpy(command, input, i);
    }
    else if(count == 2)
    {
        strncpy(type, p, len);
        char *z = type + 1;
        if(len)
        {
            for(int i = 0; i < strlen(type); i++)
            {
                strncpy(type1, z + i, 1000);
            }
        }
    }
    else if(count == 3)
    {
        strncpy(flag1, p, len);
        char *z = flag1 + 1;
        if(len)
        {
            for(int i = 0; i < strlen(flag1); i++)
            {
                strncpy(flagv1, z + i, 1000);
            }
        }
    }
    else if(count == 4)
    {
        strncpy(flag2, p, len);
        char *z = type + 1;
        if(len)
        {
            for(int i = 0; i < strlen(flag2); i++)
            {
                strncpy(flagv2, z + i, 1000);
            }
        }
    }
}

void concat(char* output, char* findings)
{
    int in1 = 0;
    while(findings[in1] != '\0')
    {
        in1 += 1;
    }
    int in2 = 0;
    int in3 = in1;
    while(output[in2] != '\0')
    {
        findings[in3] = output[in2];
        in2 += 1;
        in3 += 1;
    }
}

void getInputInfo(char* input)
{
    int args = 0;
    int start = 0;
    for(int i = 0; i <= strlen(input); i++)
    {
        //seperate input by spaces and assign numbers to each space
        if(input[i] == ' ' || input[i] == '\0')
        {
            args += 1;
            if(args == 2)
            {
                //trying to find quote seperated by spaces (doesnt rly work)
                if(input[i] == '"')
                {
                    for(int j = i + 1; i <= strlen(input); i++)
                    {
                        if(input[j] == '"')
                        {
                            createFlags(input, start, j, args);
                        }
                    }
                }
                else
                {
                    createFlags(input, start, i, args);
                }
            }
            else
            {
                createFlags(input, start, i, args);

            }
            start = i + 1;
        }
    }
}

void findFile(char* type, char* directory, int subDir)
{
    struct dirent *dp;
    char path[1000];
    DIR *file = opendir(directory);
    while((dp = readdir(file)) != NULL)
    {
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            if(strcmp(dp->d_name, type) == 0)
            {
                found = 1;
                strcat(output, "\n");
                strcat(output, type);
                strcat(output, " found in: ");
                strcat(output, directory);
                strcat(output, "\n");
                concat(output,findings);
                memset(output, 0, 1000); //try with and without

            }
            if(subDir == 1)
            {
                if(dp->d_type == DT_DIR)
                {
                    strcpy(path, directory);
                    strcat(path, "/");
                    strcat(path, dp->d_name);
                    strcat(output, "\0");
                    findFile(type, path, subDir);
                }
            }
        }
    }
    closedir(file);
}

int recursiveText(char* file2, char* text, char* directory)
{
    struct dirent *dp;
    DIR *file = opendir(directory);
    const char *s = file2;
    std::string str(s);
    while((dp = readdir(file)) != NULL)
    {
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            //compare file2 name with current direcotry file name, if they are equal then proceed
            if(strcmp(s, dp->d_name) == 0)
            {
                char path[1000];
                char temp[1000];
                strcpy(path, directory);
                strcat(path, "/");
                strcat(path, dp->d_name);
                FILE *fprt = fopen(path, "r");
                char *pos;
                //read contents of file into temp
                while((fgets(temp, 1000, fprt)) != NULL)
                {
                    //check if text is contained within temp, 
                    //if yes then we found text and return 1! 
                    //recursive function success
                    pos = strstr(temp, text);
                    if(pos != NULL)
                    {
                        //found string in file
                        return 1;
                        break;
                    }
                }
            }
        }
    }
    //not found
    return 0;
}

void findText(char* text, char* directory, int subDir)
{
    struct dirent *dp;
    DIR *file = opendir(directory);
    char path[1000];

    while((dp = readdir(file)) != NULL)
    {   
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            if(dp->d_type != DT_DIR)
            {
                int j = recursiveText(dp->d_name, text, directory);
                if(j == 1)
                {
                    found = 1;
                    strcat(output, "\n");
                    strcat(output, text);
                    strcat(output, " found in: ");
                    strcat(output, directory);
                    strcat(output, "/");
                    strcat(output, dp->d_name);
                    strcat(output, "\n");
                    concat(output,findings);
                    memset(output, 0, 1000);

                }
            }
            if(subDir == 1)
            {
                if(dp->d_type == DT_DIR)
                {
                    strcpy(path, directory);
                    strcat(path, "/");
                    strcat(path, dp->d_name);
                    strcat(output, "\0");
                    findText(text, path, subDir);
                }
            }
        }
    }
    closedir(file);
}

void findTextFS(char* text, char* flag1, char* directory, int subDir)
{
    struct dirent *dp;
    DIR *file = opendir(directory);
    char path[1000];

    while((dp = readdir(file)) != NULL)
    {
        if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            char *e;
            int index;
            e = strchr(dp->d_name, '.');
            index = (int)(e - dp->d_name);
            char name[100];
            int start = 0;
            //get just the file ending
            for(int i = index + 1; i <= strlen(dp->d_name); i++)
            {
                strncpy(name, dp->d_name + index + 1, strlen(dp->d_name) - (index + 1));
                start++;
            }
            if(strncmp(flag1, name, strlen(flag1)) == 0)
            {
                if(dp->d_type != DT_DIR)
                {
                //printf("%s", dp->d_name);
                    int j = recursiveText(dp->d_name, text, directory);
                    if(j == 1)
                    {
                        found = 1;
                        strcat(output, "\n");
                        strcat(output, text);
                        strcat(output, " found in: ");
                        strcat(output, directory);
                        strcat(output, "/");
                        strcat(output, dp->d_name);
                        strcat(output, "\n");
                        concat(output,findings);
                        memset(output, 0, 1000);

                    }
                }
            }
            if(subDir == 1)
            {
                if(dp->d_type == DT_DIR)
                {
                    strcpy(path, directory);
                    strcat(path, "/");
                    strcat(path, dp->d_name);
                    strcat(output, "\0");
                    findTextFS(text, flag1, path, subDir);
                }
            }
        }
    }
    closedir(file);
}

void quitCommand(int *childPID, int parentPID)
{
    for(int i = 0; i < 10; i++)
    {
        kill(childPID[i], SIGKILL);
        wait(0);
    }
    kill(parentPID, SIGKILL);
    munmap(childPID, sizeof(int)*10);
}

void listCommand(char* input, int *childPID)
{
    getInputInfo(input);
    for(int i = 0; i < 10; i++)
    {
        if(childPID[i] != 0)
        {
            printf("\nProcess %d -> Child PID %d -> Currently Active\n", i, childPID[i]);
        }
    }
}

void sig(int i)
{
    dup2(fd[0], STDIN_FILENO);
    ride = 1;
}

void killCommand(int pidNum, int *childPID)
{
    for(int i = 0; i < 10; i++)
    {
        if(childPID[i] == pidNum)
        {
            kill(childPID[i], SIGKILL);
            childPID[i] = 0;
            wait(0);
            break;
        }
    }
}

void reprintToTerm(char* input)
{
    printf("User Input: %s\n", input);
    memset(input, 0, 1000);
}

void killForSure(int *childPID)
{
    for(int i=0;i<10;i++) 
    {
        if(childPID[i]!=0) 
        {     
            waitpid(childPID[i],0,WNOHANG);
        }
    }
}

void checkFound(char* result)
{
    if(found == 1)
    {
        strcat(result, "\n");
        strcat(result, findings);
        strcat(result, "\n");
    }
    else
    {
        strcat(result, "\n");
        strcat(result, type);
        strcat(result, " was not found anywhere.\n");
        strcat(result, "\n");
    }
}

int main() 
{
    int *childPID = (int *)mmap(0,sizeof(int)*10,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    signal(SIGUSR1,sig);

    printf("Enter one of the following in the exact format:\n");
    printf("    [find <file>]\n");
    printf("    [find <file> -s]\n");
    printf("    [find <\"text\">]\n");
    printf("    [find <\"text\"> -s]\n");
    printf("    [find <\"text\"> -f:txt]\n");
    printf("    [find <\"text\"> -f:txt -s]\n");
    printf("    [list]\n");
    printf("    [kill <pid>]\n");
    printf("    [q] or [quit]\n");

    char input[1000];
    int parentPID = getpid();
    pipe(fd);
    int saveSTDIN = dup(STDIN_FILENO);

    for(;;)
    {
        //shell  to green
        printf("\033[1;32m");
        printf("findstuff");
        //reset to original
        printf("\033[0m");
        printf("$ ");
        fflush(0);

        dup2(saveSTDIN,STDIN_FILENO);
        ride = 0;
        int r = read(STDIN_FILENO, input, 1000);
        if(ride == 0)
        {
            input[r -1] = 0;
        }
        if(strcmp(input, "q") == 0 || strcmp(input, "quit") == 0)
        {
            quitCommand(childPID, parentPID);
            break;  
        }
        else if(strncmp(input, "kill", 4) == 0)
        {
            getInputInfo(input);
            int pidNum = atoi(type);
            killCommand(pidNum, childPID);
        }
        else if(strncmp(input, "list", 4) == 0)
        {
            listCommand(input, childPID);
        }
        else if(strncmp(input, "find", 4) == 0)
        {
            if(fork() == 0)
            {
                struct timespec start, end;
                clock_gettime(CLOCK_REALTIME, &start);
                char result[1000];
                int childIdx = 0;
                int maxLimit = 0;
                for(int i = 0; i < 10; i++)
                {
                    if(childPID[i] == 0)
                    {
                        maxLimit = 1;
                        childPID[i] = getpid();
                        childIdx = i;
                        break;
                    }
                }
                if(maxLimit == 0)
                {
                    printf("\nAhh max limit of children has been reached!\n");
                }
                close(fd[0]);
                char directory[1000]; 
                char* originalPath = getcwd(directory, sizeof(directory)); //current path
                getInputInfo(input);

                if((strncmp("-f", flag1, 2) == 0) && (strncmp("-s", flag2, 2) == 0))
                {
                    if(strncmp("\"", type, 1) == 0) //validate that type is a text with quotation
                    {
                        char text[100];
                        removeQuotes(type, text);
                        //skip first 3 characters
                        memmove(flag1, flag1 + 3, (1000 - 3) / sizeof(input[0]));
                        findTextFS(text, flag1, directory, 1);

                    }
                }
                else if(strncmp("-f", flag1, 2) == 0)
                {
                    if(strncmp("\"", type, 1) == 0) //validate that type is a text with quotation
                    {
                        char text[100];
                        removeQuotes(type, text);
                        //skip first 3 characters
                        memmove(flag1, flag1 + 3, (1000 - 3) / sizeof(input[0]));
                        findTextFS(text, flag1, directory, 0);

                    }
                }
                else if(strncmp("-s", flag1, 2) == 0)
                {
                    if(strncmp("\"", type, 1) == 0)
                    {
                        //text search
                        char text[100];
                        removeQuotes(type, text);
                        findText(text, originalPath, 1);
                    }
                    else
                    {
                        //file search
                        findFile(type, originalPath, 1);
                    }
                }
                else
                {
                    if(strncmp("\"", type, 1) == 0)
                    {
                        //text search
                        char text[100];
                        removeQuotes(type, text);
                        findText(text, originalPath, 0);
                    }
                    else
                    {
                        findFile(type, originalPath, 0);  
                    }

                }
                clock_gettime(CLOCK_REALTIME, &end);
                double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;
                
                sprintf(result, "\n\nProcess %d reporting after %f seconds!", childIdx, time_spent);
                checkFound(result);
                found = 0;
                write(fd[1],result,strlen(result));
                close(fd[1]);

                kill(parentPID,SIGUSR1);
                return 0;
            }
        }
        killForSure(childPID);
        reprintToTerm(input);
    }
    munmap(childPID, sizeof(int)*10);
    return 0;
}
