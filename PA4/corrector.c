#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#define LINE_LEN 1024


typedef struct Person{
    char name[50];
    char surname[50];
    char gender[10];
}Person;

Person people[100];
int lineCount = 0;
int checker=0;

int myfunct(char *path){

    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);

    if (dir == NULL) {
        printf("Error opening the directory %s\n", path);
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {

        char newPath[300] = "";
        strcat(newPath, path);
        strcat(newPath, "/");
        strcat(newPath, entry->d_name);

        if (entry->d_type == DT_REG) { // check if a regular file
            // check if the extension is .txt
            char *extension = strrchr(entry->d_name, '.');
            if (extension != NULL && strcmp(extension,".txt") == 0){
                // check if it is the database.txt
                // it will skip if its not in a directory with the help of the checker
                if (strcmp(entry->d_name, "database.txt") == 0 && checker == 0) {
                    continue;
                }

                //correction starts

                FILE *fp2 = fopen(newPath, "r+");

                if(fp2 == NULL){
                    printf("Reading file %s failed\n", newPath);
                    continue;
                }

                char worder[100];
                

                while (fscanf(fp2, "%s", worder)!= EOF){
                    //printf("I am from file: %s. Word is: %s\n", newPath, worder);

                    for(int i = 0; i < lineCount; i++){
                        if(strcmp(worder,people[i].name) == 0){
                            fseek(fp2,-4-strlen(worder),SEEK_CUR);
                            fputs(people[i].gender, fp2);
                            fseek(fp2,strlen(worder)+2,SEEK_CUR);
                            fputs(people[i].surname, fp2);

                        }
                    }
                }


                //correction ends
                fclose(fp2);
            }else{
                continue;
            }
            


        }else if(entry->d_type == DT_DIR){ //check if its a directory

            //since "." and ".." are also directories we need to omit them
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                //. or ..
                continue;
            }

            checker = 1; // make checker equal to 1, so we will be able to find database.txt in directories

            myfunct(newPath);
        }else{
            continue;
        }
    }

    closedir(dir);
    return 0;
}


int main(){



    FILE *fp;
    fp = fopen("./database.txt", "r");
    if (fp == NULL) {
        printf("Opening database.txt failed\n");
        return 1;
    }

    char line[LINE_LEN];
    
    while (fgets(line, LINE_LEN, fp) != NULL) {
        // split the line into words
        char *word;
        word = strtok(line, " ");
        if(strcmp(word,"m") == 0){
            word = "Mr.";
        }else if(strcmp(word,"f") == 0){
            word = "Ms.";
        }else{
            printf("Something is wrong about the gender.\n");
        }
        strcpy(people[lineCount].gender,word);
        word = strtok(NULL, " ");
        strcpy(people[lineCount].name,word);
        word = strtok(NULL, " ");
        //last word (surname) is including new line character so I am deleting it first
        size_t len = strcspn(word, "\n");
        char *clean_word = strndup(word, len);
        word = clean_word;
        //finished
        strcpy(people[lineCount].surname,word);

        free(clean_word);
        
        lineCount++;
        
    }
    

    
    myfunct(".");







    fclose(fp);
    return 0;
}