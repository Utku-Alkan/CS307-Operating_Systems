#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <pthread.h>
using namespace std;

struct Line{
    string whole;
    string command;
    string inputs;
    string options;
    string redirection;
    string backgroundJob;
    string outputFileName;
    string optionCommand;
    string optionString;
};

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;


void *printer(void* args){
    
    pthread_mutex_lock(&lock);
    
    cout << "---- "<< pthread_self() << endl;

    cout << (char *) args;

    cout << "---- "<< pthread_self() << endl;

    fflush(stdout);
    pthread_mutex_unlock(&lock);
    return NULL;
}



int main(){
    char buf[1024];
    char buf2[1024];
    char bufArray[64][1024];
    string myText;
    ifstream MyReadFile("commands.txt");
    ofstream MyWriteFile("parse.txt");
    vector<string> lines;
    vector<Line> linesStruct;

    while (getline (MyReadFile, myText)) {
        //cout << myText << endl;
        lines.push_back(myText);
    }

    

    for (string liner : lines){
        Line linedata;
        linedata.backgroundJob = "";
        linedata.command = "";
        linedata.inputs = "";
        linedata.optionCommand = "";
        linedata.options = "";
        linedata.optionString = "";
        linedata.outputFileName = "";
        linedata.redirection = "";
        linedata.whole = "";

        linedata.whole = liner;  
         
        vector<string> words = split(liner,' ');
        
        linedata.command = words[0];
        


        if(words.size() > 1){ // check if its not finished

            if(words[words.size()-1] == "&"){ //if there exists & at the end
                linedata.backgroundJob = words[words.size()-1];
                words.pop_back(); // & removed
            }

            if(words[1][0] == '-'){ // check if first letter is '-' . If it is, this is an option and there is no input
                linedata.inputs = "";
                linedata.options = words[1];
                linedata.optionCommand = words[1];
                if(words.size() > 2){ //check if its not finished
                    
                    if(words[2][0] == '<' ){ // options finished and redirection < exists
                        linedata.redirection = words[2];
                        linedata.outputFileName = words[3]; // if redirection exists filename must also exist

                    }else if(words[2][0] == '>' ){ // options finished and redirection > exists
                        linedata.redirection = words[2];
                        linedata.outputFileName = words[3]; // if redirection exists filename must also exist
                    }else{ // there is an option string after '-' like "-f grep"
                        linedata.options = words[1] + " " + words[2];
                        linedata.optionCommand = words[1];                
                        linedata.optionString = words[2];
                        if(words.size() > 3){ //we are ending it

                            if(words[3][0] == '<' ){ // options finished and redirection < exists
                                linedata.redirection = words[3];
                                linedata.outputFileName = words[4]; // if redirection exists filename must also exist

                            }else if(words[3][0] == '>' ){ // options finished and redirection > exists
                                linedata.redirection = words[3];
                                linedata.outputFileName = words[4]; // if redirection exists filename must also exist
                            }else{
                                cout<<"SOMETHING IS WRONG. THERE EXISTS SOMETHING AFTER OPTIONS THAT DOESN'T BELONG TO REDIRECTION"<<endl;
                            }
                        }
                    }
                }

            }else if(words[1][0] == '<'){ //no input no option
                linedata.inputs = "";
                linedata.options = "";
                linedata.redirection = words[1];
                linedata.outputFileName = words[2]; // if redirection exists filename must also exist
            
            
            }else if(words[1][0] == '>'){ //no input no option
                linedata.inputs = "";
                linedata.options = "";
                linedata.redirection = words[1];
                linedata.outputFileName = words[2]; // if redirection exists filename must also exist
            
            
            }else{ // there is an input
                linedata.inputs = words[1];

                if(words.size() > 2){ //check if its not finished
                    

                
                    if(words[2][0] == '-'){ // check if first letter is '-' . If it is, this is an option
                        linedata.options = words[2];
                        linedata.optionCommand = words[2];
                        if(words.size() > 3){ //check if its not finished
                            
                            if(words[3][0] == '<' ){ // options finished and redirection < exists
                                linedata.redirection = words[3];
                                linedata.outputFileName = words[4]; // if redirection exists filename must also exist

                            }else if(words[3][0] == '>' ){ // options finished and redirection > exists
                                linedata.redirection = words[3];
                                linedata.outputFileName = words[4]; // if redirection exists filename must also exist
                            }else{ // there is an option string after '-' like "-f grep"
                                linedata.options = words[2] + " " + words[3];
                                linedata.optionCommand = words[2];                
                                linedata.optionString = words[3];

                                if(words.size() > 3){ //we are ending it

                                    if(words[4][0] == '<' ){ // options finished and redirection < exists
                                        linedata.redirection = words[4];
                                        linedata.outputFileName = words[5]; // if redirection exists filename must also exist

                                    }else if(words[4][0] == '>' ){ // options finished and redirection > exists
                                        linedata.redirection = words[4];
                                        linedata.outputFileName = words[5]; // if redirection exists filename must also exist
                                    }else{
                                        cout<<"SOMETHING IS WRONG. THERE EXISTS SOMETHING AFTER OPTIONS THAT DOESN'T BELONG TO REDIRECTION"<<endl;
                                    }
                                }
                            }
                        }
                    }else{ //then remaining is redirection
                        linedata.redirection = words[2];
                        linedata.outputFileName = words[3]; // if redirection exists filename must also exist
                    }
                }
            }
        }

        
        linesStruct.push_back(linedata);
        
        MyWriteFile << "----------" << endl;
        MyWriteFile << "Command: " << linedata.command << endl;
        MyWriteFile << "Inputs: " <<linedata.inputs << endl;
        MyWriteFile << "Options: " <<linedata.options << endl;
        if(linedata.redirection == ""){
            MyWriteFile << "Redirection: -" << endl;
        }else{
            MyWriteFile << "Redirection: " <<linedata.redirection << endl;}
        if(linedata.backgroundJob == "&"){
            MyWriteFile << "Background Job: y" << endl;
        }else{
            MyWriteFile << "Background Job: n" << endl;
        }
        
        MyWriteFile << "----------" << endl;

    }

    //parsing and printing has finished

    pthread_t threads[64];
    int threadCount = 0;
    int threadCountBackground = 0;

    pthread_t threadArray[64];


    for(int i = 0; i < linesStruct.size(); i++){
        int length = 0;
        const char * char1 = linesStruct[i].command.c_str();
        const char * char2 = linesStruct[i].inputs.c_str();
        const char * char3 = linesStruct[i].options.c_str();
        const char * char4 = linesStruct[i].redirection.c_str();
        const char * char5 = linesStruct[i].outputFileName.c_str();
        const char * char6 = linesStruct[i].backgroundJob.c_str();
        const char * char7 = linesStruct[i].optionCommand.c_str();
        const char * char8 = linesStruct[i].optionString.c_str();
        string escapedOption = "\\" + linesStruct[i].optionCommand; // dont use is this rn
        //didnt understand why but it works without escape sequence so I am not using this, it is probably because of c++
        const char * char9 = escapedOption.c_str();


        if(linesStruct[i].command != ""){
            length++;
        }
        
        if(linesStruct[i].inputs != ""){
            length++;
        }

        if(linesStruct[i].optionCommand != ""){
            length++;
        }

        if(linesStruct[i].optionString != ""){
            length++;
        }
        length++; //for null

        char *myArgs[length];

        int put = 0;

        //instead strdup I can use const_cast<char*> too
        if(linesStruct[i].command != ""){
            myArgs[put] = strdup(char1);
            put++;
        }
        
        if(linesStruct[i].inputs != ""){
            myArgs[put] = strdup(char2);
            put++;
        }

        if(linesStruct[i].optionCommand != ""){
            myArgs[put] = strdup(char7); //char9 mi kullanmaliyim acaba -> hayir c++ sag olsun
            put++;
        }

        if(linesStruct[i].optionString != ""){
            myArgs[put] = strdup(char8);
            put++;
        }
        myArgs[put] = NULL;
        
    
        int fd[2];
        pipe(fd); 

        if (linesStruct[i].command == "wait"){
            
            for(int a = 0; a < threadCountBackground; a++){
                pthread_join(threadArray[a], NULL);
            }
            continue;
        }
        int rc = fork();


        if (rc<0){
            
            fprintf(stderr, "fork failed\n");
            exit(1);
            
        }else if (rc == 0){ //child
             
            
            if(linesStruct[i].redirection == ">"){ //there is a redirection

                close(STDOUT_FILENO);

                open(char5, O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);

                execvp(myArgs[0],myArgs);

            }else if(linesStruct[i].redirection == "<"){ 
                //input redirectioning
                close (STDIN_FILENO);
                int filer = open(char5, O_RDONLY);
                dup2(fd[1], 1); // writes to the pipe

                close(fd[0]);
                close(fd[1]);
                execvp(myArgs[0],myArgs);

            }else{ //no redirection
                
                dup2(fd[1], 1); // writes to the pipe

                close(fd[0]);
                close(fd[1]);
                execvp(myArgs[0],myArgs);
                
            }  

        }else{ //parent


            if (linesStruct[i].backgroundJob != "&"){ //if its not background wait for the child
                memset(buf, 0, 1024);
                
                wait(NULL);
                if (linesStruct[i].redirection == "" || linesStruct[i].redirection == "<") {
                    
                    close(fd[1]);

                    dup2(fd[0], 0); //reads from the pipe
                                    
                    read(fd[0], buf, sizeof(buf));
                    pthread_create(&threads[threadCount],NULL, printer, buf);
                    pthread_join(threads[threadCount], NULL); //join right after since it is not a background job

                    threadCount++;

                } 

            }else{ //if it is background do not wait

                if (linesStruct[i].redirection == "" || linesStruct[i].redirection == "<") {
                
                    close(fd[1]);

                    dup2(fd[0], 0); //reads from the pipe
                    read(fd[0], bufArray[threadCountBackground], sizeof(bufArray[threadCountBackground]));

                    pthread_create(&threadArray[threadCountBackground],NULL, printer, bufArray[threadCountBackground]);
                    //since this is background we are not joining. We will join them in main.
                    threadCountBackground++;
                }
            }      
        }
    }

    for(int a = 0; a < threadCountBackground; a++){
        pthread_join(threadArray[a], NULL);
    }

    // Close the files
    MyWriteFile.close();
    MyReadFile.close();

    return 0;
}

