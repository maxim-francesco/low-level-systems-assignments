#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include "a2_helper.h"

typedef struct{
    int p;
    int id;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
}TH_STRUCT;

//pentru thread6
sem_t sem1;
sem_t sem2;
//pentru thread5
sem_t sem3;
sem_t sem4;
sem_t sem5;
sem_t sem6;
int flag = 0;
int flag_istoric = 0;
int flag_ajutator = 0;
int current_threads = 0;
int history = 0;
//
sem_t *sem7;
sem_t *sem8;

void* for_thread_6(void *arg){
    TH_STRUCT *s = (TH_STRUCT*)arg;
    if(s->id == 2){
        sem_wait(&sem1);
    }
    if(s->id == 1){
        sem_wait(sem7);
    }
    info(BEGIN, s->p, s->id);
    if(s->id == 4){
        sem_post(&sem1);
        sem_wait(&sem2);
    }
    info(END, s->p, s->id);
    if(s->id == 1){
        sem_post(sem8);
    }
    if(s->id == 2){
        sem_post(&sem2);
    }
    return NULL;
}

void* for_thread_5(void *arg){
    TH_STRUCT *s = (TH_STRUCT*)arg;

    sem_wait(&sem3);

    info(BEGIN, s->p, s->id);

    if(flag == 1){
        pthread_mutex_lock(s->lock);
        current_threads = current_threads + 1;
        //printf("Current threads : %d, entered %d and flag is %d\n", current_threads, s->id, flag);
        if(current_threads == 6 && flag == 1){
            pthread_cond_signal(s->cond);
        }
        pthread_mutex_unlock(s->lock);
    }

    if(flag == 1){
        sem_wait(&sem6);
        sem_post(&sem6);
    }

    info(END, s->p, s->id);

    pthread_mutex_lock(s->lock);
        history = history + 1;
    pthread_mutex_unlock(s->lock);

    sem_post(&sem3);


    return NULL;
}

void* CHUNGHA(void *arg){
    TH_STRUCT *s = (TH_STRUCT*)arg;

    sem_wait(&sem3);

        pthread_mutex_lock(s->lock);

            info(BEGIN, s->p, s->id);

            flag = 1;
            current_threads = current_threads + 1;
            //printf("Current threads : %d, entered %d and flag is %d\n", current_threads, s->id, flag);

            while (current_threads != 6)
            {
                pthread_cond_wait(s->cond, s->lock);
            }

            info(END, s->p, s->id);

            sem_post(&sem6);

            flag = 0;
            //printf("Current threads : %d, exited %d and flag is %d\n", current_threads, s->id, flag);

        pthread_mutex_unlock(s->lock);

    sem_post(&sem3);

    return NULL;
}

void* for_thread_7(void *arg){
    TH_STRUCT *s = (TH_STRUCT*)arg;
    if(s->id == 3){
        sem_wait(sem8);
    }
    info(BEGIN, s->p, s->id);
    info(END, s->p, s->id);
    if(s->id == 6){
        sem_post(sem7);
    }
    return NULL;
}

int main(void){

    sem7 = sem_open("/semaforBB", O_CREAT, 0644, 0);
    sem8 = sem_open("/semaforXX", O_CREAT, 0644, 0);

    pid_t pid2 = -1;
    pid_t pid3 = -1;
    pid_t pid4 = -1;
    pid_t pid5 = -1;
    pid_t pid6 = -1;
    pid_t pid7 = -1;

    init();

    info(BEGIN, 1, 0);

    pid2 = fork();

    if(pid2 == 0){
        info(BEGIN, 2, 0);
        pid3 = fork();
        if(pid3 == 0){
            info(BEGIN, 3, 0);
            pid5 = fork();
            if(pid5 == 0){
                info(BEGIN, 5, 0);

                pthread_t tids[39];
                TH_STRUCT params[39];
                sem_init(&sem3, 0, 6);
                sem_init(&sem4, 0, 1);
                sem_init(&sem5, 0, 0);
                sem_init(&sem6, 0, 0);
                pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
                pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

                for(int i = 1; i <= 38; i++){
                    params[i].p = 5;
                    params[i].id = i;
                    params[i].cond = &cond;
                    params[i].lock = &lock;
                    if(i != 11){
                        pthread_create(&tids[i], NULL, for_thread_5, &params[i]);
                    }else{
                        pthread_create(&tids[i], NULL, CHUNGHA, &params[i]);
                    }
                }

                for(int i = 1; i <= 38; i++){
                    pthread_join(tids[i], NULL);
                }

                info(END, 5, 0);
            }else{
                pid7 = fork();
                if(pid7 == 0){
                    info(BEGIN, 7, 0);

                    pthread_t tids[7];
                    TH_STRUCT params[7];
                    for(int i = 1; i <= 6; i++){
                        params[i].p = 7;
                        params[i].id = i;
                        pthread_create(&tids[i], NULL, for_thread_7, &params[i]);
                    }

                    for(int i = 1; i <= 6; i++){
                        pthread_join(tids[i], NULL);
                    }

                    info(END, 7, 0);
                }else{
                    waitpid(pid7, NULL, 0);
                    waitpid(pid5, NULL, 0);
                    info(END, 3, 0);
                }
            }
        }else{
            pid4 = fork();
            if(pid4 == 0){
                info(BEGIN, 4, 0);
                info(END, 4, 0);
            }else{
                waitpid(pid3, NULL, 0);
                waitpid(pid4, NULL, 0);
                info(END, 2, 0);
            }
        }
    }else{
        pid6 = fork();
        if(pid6 == 0){
            info(BEGIN, 6, 0);

            pthread_t tids[5];
            TH_STRUCT params[5];
            sem_init(&sem1, 0, 0);
            sem_init(&sem2, 0, 0);
            for(int i = 1; i <= 4; i++){
                params[i].p = 6;
                params[i].id = i;
                pthread_create(&tids[i], NULL, for_thread_6, &params[i]);
            }

            for(int i = 1; i <= 4; i++){
                pthread_join(tids[i], NULL);
            }

            info(END, 6, 0);
        }else{
            waitpid(pid2, NULL, 0);
            waitpid(pid6, NULL, 0);
            info(END, 1, 0);    
        }
    }

    return(0);
}