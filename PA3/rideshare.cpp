#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sstream>
#include <pthread.h>
using namespace std;

pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locker2 = PTHREAD_MUTEX_INITIALIZER;


typedef struct semaphore{
    int val;
    pthread_mutex_t lock;
    pthread_cond_t c;
} sem_t;

void sem_wait(sem_t *s){
    pthread_mutex_lock(&s->lock);
    s->val--;
    pthread_mutex_unlock(&locker); // 200 iq move   
    if(s->val < 0){
        pthread_cond_wait(&s->c, &s->lock);
    }
    pthread_mutex_unlock(&s->lock);
}

void sem_post(sem_t *s){
    pthread_mutex_lock(&s->lock);
    s->val++;
    pthread_cond_signal(&s->c);
    pthread_mutex_unlock(&s->lock);
}

void sem_init(sem_t *s, int stat, int init_val){
    pthread_mutex_init(&s->lock, NULL); 
    pthread_cond_init(&s->c, NULL);
    s->val=init_val;
}

pthread_barrier_t barrier;
sem_t teamAsemaphores;
sem_t teamBsemaphores;

void *myfunct(void* args){
    pthread_mutex_lock(&locker);
    cout << "Thread ID: "<< pthread_self() << ", Team: " << (char *) args << ", I am looking for a car" << endl;
    bool driver = false;

    /*if(teamer == "A"){
        if((teamAsemaphores.val > -1 || teamBsemaphores.val > -2) && (teamAsemaphores.val > -3 || teamBsemaphores.val > 0)){
            sem_wait(teamAsemaphores);  

        }
    }else{
        if((teamAsemaphores.val > -2 || teamBsemaphores.val > -1) && (teamAsemaphores.val > 0 || teamBsemaphores.val > -3)){
            sem_wait(teamBsemaphores);
        }
    }*/
    
    //checking the fans's team
    bool teamAChecker = false;
    string teamName = (char *) args;
    if(teamName == "A"){
        teamAChecker = true;
    }else{
        teamAChecker = false;
    }
    //finished

    //cout << "A val " << teamAsemaphores.val << " B val: " << teamBsemaphores.val << endl;
    if(teamAChecker){
        if(teamAsemaphores.val + teamBsemaphores.val <= -3){ //3 or more fans are waiting in total
            if(teamAsemaphores.val <= -1 && teamBsemaphores.val <= -2){ //we have at least 1 A, 2 B
                sem_post(&teamBsemaphores);
                sem_post(&teamBsemaphores);
                sem_post(&teamAsemaphores);
                driver = true;

            }else if(teamAsemaphores.val <= -3 && teamBsemaphores.val <= 0){ //we have at least 3 A

                sem_post(&teamAsemaphores);
                sem_post(&teamAsemaphores);
                sem_post(&teamAsemaphores);
                driver = true;

            }else{
                sem_wait(&teamAsemaphores); 
            }
            
        }else{
            sem_wait(&teamAsemaphores); 
        }
    }else{
        if(teamAsemaphores.val + teamBsemaphores.val <= -3){ //3 or more fans are waiting in total

            if(teamAsemaphores.val <= -2 && teamBsemaphores.val <= -1){ //we have at least 2 A, 1 B 

                sem_post(&teamAsemaphores);
                sem_post(&teamAsemaphores);
                sem_post(&teamBsemaphores);
                driver = true;

            }else if(teamAsemaphores.val <= 0 && teamBsemaphores.val <= -3){ //we have at least 3 B 

                sem_post(&teamBsemaphores);
                sem_post(&teamBsemaphores);
                sem_post(&teamBsemaphores);
                driver = true;

            }else{
                sem_wait(&teamBsemaphores);
            }
        }else{
            sem_wait(&teamBsemaphores);
        }
    }
    pthread_mutex_lock(&locker2); //two threads can enter this print statement.. not nice so mutex is needed
    cout << "Thread ID: " << pthread_self() << ", Team: " << (char *) args << ", I have found a spot in a car" << endl;
    pthread_mutex_unlock(&locker2); // unlocks
    pthread_barrier_wait(&barrier);
    pthread_mutex_lock(&locker2); //somehow I am scared of 2 threads will enter driver == true statement so one more mutex to be safe
    if(driver == true){
        driver = false;
        cout << "Thread ID: " << pthread_self() << ", Team: "<< (char *) args <<", I am the captain and driving the car" << endl;
        pthread_mutex_unlock(&locker);
    }
    pthread_mutex_unlock(&locker2); // unlocks

    return NULL;
}

int main(int argc, char *argv[]){
    if(argc != 3){ // argument count checked
        cout << "The main terminates" << endl;
        return 1;
    }
    int teamACount =atoi(argv[1]);
    int teamBCount =atoi(argv[2]);
    int allCount = teamACount + teamBCount;
    pthread_t threadArray[200];
    if( (atoi(argv[1])%2 != 0)  ||  (atoi(argv[2])%2 != 0)  || ((atoi(argv[1]) + atoi(argv[2]))%4 != 0)){ //rules are checked
        cout << "The main terminates" << endl;
        return 1;
    }


    pthread_barrier_init(&barrier, NULL, 4);

    sem_init(&teamAsemaphores,0,0);
    sem_init(&teamBsemaphores,0,0);

    char TeamA[8] ="A";
    char TeamB[8] ="B";
    
    //if threads are needed to be created randomly
    /*for(int i = 0; i < allCount; i++){ //all threads will be created after this
        if(teamACount!= 0 && teamBCount!= 0){
                if(rand() % 2 == 0){ //randomizing creating threads
                                
                    pthread_create(&threadArray[i], NULL, myfunct, TeamA);
                    teamACount--;

                }else{
                    pthread_create(&threadArray[i], NULL, myfunct, TeamB);
                    teamBCount--;
                }
        }else if(teamACount== 0){
            pthread_create(&threadArray[i], NULL, myfunct, TeamB);
            teamBCount--;
        }else{
            pthread_create(&threadArray[i], NULL, myfunct, TeamA);
            teamACount--;
        }
    }*/

    //if threads are needed to be created in order
    for(int i = 0; i < teamACount; i++){
        pthread_create(&threadArray[i], NULL, myfunct, TeamA);
    }
    for(int i = 0; i < teamBCount; i++){
        pthread_create(&threadArray[i+teamACount], NULL, myfunct, TeamB);
    }


    for(int a = 0; a < allCount; a++){
        pthread_join(threadArray[a], NULL);
    }
    cout << "The main terminates" << endl;
    return 0;
}