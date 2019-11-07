#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <queue>
#include <list>
#include <iostream>
#include <pthread.h>

using namespace std;

//global variables
queue<int> work_q;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_MUTEX_INITIALIZER;
list<pthread_t> thread_list;
const static int NUM_OF_WORKERS = 5;

//function declaration
void* readInput(void*);
void* collatz(void*);
int collatzSteps(int);


int main(){
  
  pthread_t inputThread;

  pthread_create(&inputThread, NULL,readInput, NULL);

  for(int i = 0; i < NUM_OF_WORKERS; i++){
    pthread_t worker_thread;
    pthread_create(&worker_thread, NULL, collatz, NULL);
  }
  
  pthread_join(inputThread, NULL);
  
  return 0;
}

void* readInput(void* arg){
  int input;

  do{
    cin >> input;

  pthread_mutex_lock(&lock);
  work_q.push(input);
  pthread_cond_signal(&not_empty);
  pthread_mutex_unlock(&lock);
  
  }while(input != 0);

  return NULL;
}

int collatzSteps(int number){
  int count = 0;

  while(number !=1){

    if(number % 2 == 0){
      number /= 2;
    }

    else{
      number *= 3;
      number++;
    }
    count++;
  }

  return count;
}

void *collatz (void * arg){ 
  int number;

  do{

    pthread_mutex_lock(&lock);    
    while(work_q.empty()){
      pthread_cond_wait(&not_empty,&lock);
    }
    
    number = work_q.front();
    work_q.pop();
    pthread_mutex_unlock(&lock);
    
    if( number != 0){
      int steps = collatzSteps(number);
    
      cout << number << ": " << steps;
    }

  } while(number != 0);

  return NULL;
}

