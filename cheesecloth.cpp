
//This is a test for github
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <queue>
#include <math.h>
#include <list>

using namespace std;
/*you can change the names of the Queues and locks, but i think we need a lock for every queue*/

//queues
queue<long long> inputQueue; //all numbers
queue<long long> oddQueue; //only odd numbers
queue<long long> fibQueue; //only fibonacci numbers
queue<long long> finalQueue; //final numbers after colatz filter

//locks
pthread_mutex_t inputLock = PTHREAD_MUTEX_INITIALIZER; //locks queue being loaded from console
pthread_mutex_t oddLock = PTHREAD_MUTEX_INITIALIZER; //locks odd queue
pthread_mutex_t fibLock = PTHREAD_MUTEX_INITIALIZER; //locks fib queue
pthread_mutex_t finalLock = PTHREAD_MUTEX_INITIALIZER; //locks colatz queue

pthread_cond_t input_queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t odd_queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fib_queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t collatz_queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t final_queue_not_empty = PTHREAD_COND_INITIALIZER;

//bool quit = false;
int N_THREADS = 0;

//prototypes
void* readInput(void*);
void* filterOdds(void*);
void* filterFib(void*);
void* filterCollatz(void*);
void* readFinalQueue(void*);

void checkForErrors();

bool isOdd(long long);
bool inFibonacci(long long);
bool collatzSteps(long long);

int main(int argc, char** argv)
{
  if(argc <= 1 || stoi(argv[1]) <= 0)    // Check that user has given a number_of_threads arg AND that it is greater than 0
    {
      cout << endl << "USAGE: \"./cheesecloth <number_of_threads>\"  (number_of_threads must be greater than 0)" << endl;
      exit(-1);
    }
    
  N_THREADS = stoi(argv[1]);
  
  pthread_t inputThread;
  if(pthread_create(&inputThread, NULL, readInput, NULL) != 0) checkForErrors();   // pthread_create and pthread_join return 0 on success, anything else and we've got a problem
  
  pthread_t filterThread1[N_THREADS];
  for(int i = 0; i < N_THREADS; i++)
    {
      if(pthread_create(&filterThread1[i], NULL, filterOdds, NULL) != 0) checkForErrors();
    }
  
  pthread_t filterThread2[N_THREADS];
  for(int i = 0; i < N_THREADS; i++)
    {
      if(pthread_create(&filterThread2[i], NULL, filterFib, NULL) != 0) checkForErrors();
    }
  
  pthread_t filterThread3[N_THREADS];
  for(int i = 0; i < N_THREADS; i++)
    {
      if(pthread_create(&filterThread3[i], NULL, filterCollatz, NULL) != 0) checkForErrors();
    }
	
  pthread_t printThread;
  if(pthread_create(&printThread, NULL, readFinalQueue, NULL) != 0) checkForErrors();
  
  pthread_join(inputThread, NULL);
  for(int i = 0; i < N_THREADS; i++)
  {
    if(pthread_join(filterThread1[i], NULL) != 0) checkForErrors();
    if(pthread_join(filterThread2[i], NULL) != 0) checkForErrors();
    if(pthread_join(filterThread3[i], NULL) != 0) checkForErrors();
  }
  if(pthread_join(printThread, NULL) != 0)  checkForErrors();
	
  return 0;
}

void checkForErrors()
{
  char* error;
  error = strerror_r(errno, error, 512);
  cout << "ERROR: " << error << endl;
  // Now lock the input queue, send in a zero, signal, and unlock
  pthread_mutex_lock(&inputLock);
  inputQueue.push(0);
  pthread_cond_signal(&input_queue_not_empty);
  pthread_mutex_unlock(&inputLock);
}

void* readInput(void* arg){
  long long input;

  do{
    cin >> input;
    //locking the first queue
    if(pthread_mutex_lock(&inputLock) != 0) checkForErrors();
    inputQueue.push(input);
    if(pthread_cond_signal(&input_queue_not_empty) != 0) checkForErrors();
    if(pthread_mutex_unlock(&inputLock) != 0)  checkForErrors();
  }while(input != 0);//stops when 0 comes up

  for(int i = 0; i < N_THREADS && input == 0; i++)
  {
    inputQueue.push(input);
    if(pthread_cond_signal(&input_queue_not_empty) != 0) checkForErrors();
  }
return NULL;
}

void* filterOdds(void* arg){
  long long number;
  bool quit = false;
  do{
    if(pthread_mutex_lock(&inputLock) != 0) checkForErrors();
    while(inputQueue.empty())
      {
	if(pthread_cond_wait(&input_queue_not_empty, &inputLock) != 0) checkForErrors();
      }
    number = inputQueue.front();
    inputQueue.pop();
    if(pthread_mutex_unlock(&inputLock) != 0) checkForErrors();

    quit = (number == 0);
		
    if(isOdd(number) || number == 0){
      if(pthread_mutex_lock(&oddLock) != 0) checkForErrors();
      oddQueue.push(number);//transfer num from first queue to second
      if(pthread_cond_signal(&odd_queue_not_empty) != 0) checkForErrors();
      if(pthread_mutex_unlock(&oddLock) != 0) checkForErrors();
      //cout << "Made it through odd filter :" << number << endl;
    }

  }while(!quit);

return NULL;
}

void* filterFib(void* arg){ //almost the exact same as filterOdds()
  long long number;
  bool quit = false;
  do{
    if(pthread_mutex_lock(&oddLock) != 0) checkForErrors();
    while(oddQueue.empty()){
      if(pthread_cond_wait(&odd_queue_not_empty, &oddLock) != 0) checkForErrors();
    }
    number = oddQueue.front();
    oddQueue.pop();
    if(pthread_mutex_unlock(&oddLock) != 0) checkForErrors();

    quit = (number == 0);
		
    if(inFibonacci(number) || number == 0){
      if(pthread_mutex_lock(&fibLock) != 0) checkForErrors();
      fibQueue.push(number);
      if(pthread_cond_signal(&fib_queue_not_empty) != 0) checkForErrors();
      if(pthread_mutex_unlock(&fibLock) != 0) checkForErrors();
      //cout << "made it through fibonacci: " << number << endl; 
    }

  }while(!quit);
	
return NULL;
}

void* filterCollatz(void* arg)
{
  long long number;
  bool quit = false;
  do{
    if(pthread_mutex_lock(&fibLock) != 0) checkForErrors();
    //cout << "collatz is about to start spinning " << endl;
    while(fibQueue.empty()){
      if(pthread_cond_wait(&fib_queue_not_empty, &fibLock) != 0) checkForErrors();
    }
    number = fibQueue.front();
    fibQueue.pop();
    if(pthread_mutex_unlock(&fibLock) != 0) checkForErrors();
    //cout << "collatz made it past spin lock " << endl;
    quit = (number == 0);

    //cout << "collatz made it past quit statement" << endl;
    if(collatzSteps(number) || number == 0){
      if(pthread_mutex_lock(&finalLock) != 0) checkForErrors();
      finalQueue.push(number);
      if(pthread_cond_signal(&final_queue_not_empty) != 0) checkForErrors();
      if(pthread_mutex_unlock(&finalLock) != 0) checkForErrors();
    }

  }while(!quit);


return NULL;
}


void* readFinalQueue(void* arg)
{
  bool quit = false;
  int counter = 0;
  do
    {
      if(pthread_mutex_lock(&finalLock) != 0) checkForErrors();
      while(finalQueue.empty())
	{
	  if(pthread_cond_wait(&final_queue_not_empty, &finalLock) != 0) checkForErrors();
	}

      if(finalQueue.front() == 0)
	{
	  counter++;
	}
      quit = (counter == N_THREADS);
      if(finalQueue.front() != 0) cout << "This number made it through all the filters: " << finalQueue.front()  << endl;
      finalQueue.pop();
	
      if(pthread_mutex_unlock(&finalLock) != 0) checkForErrors();
    }while(!quit);
  return NULL;
}

bool isOdd(long long num){
	//checks if num is odd
	return num % 2 == 1;
}

// A utility function that returns true if x is perfect square 
bool isPerfectSquare(long long x) 
{ 
    long long s = sqrt(x); 
    return (s*s == x); 
} 
  
// Returns true if n is a Fibinacci Number, else false 
// bool inFibonacci(long long n) 
// { 
//     // n is Fibinacci if one of 5*n*n + 4 or 5*n*n - 4 or both 
//     // is a perferct square 
//     return isPerfectSquare(5*n*n + 4) || 
//            isPerfectSquare(5*n*n - 4); 
// } 


bool inFibonacci(long long num){												    
  //cycles through fibonacci sequence checking to see if num is part of sequence at every step				    
	//	we could also calculate the fib sequence once (from 0 to 2^64) and store it in an array if this takes too long.	    
    bool inFib = false;													    
    long long fib1 = 0;													    
    long long fib2 = 1;													    
    long long fibn =0;													    
																    
    while (fibn <= num){													    
	fibn = fib1 + fib2;												    
																    
	if(fibn == num || fib1 == num || fib2 == num){									    
	    inFib = true;												    
	}														    
																    
	fib1 = fib2;													    
	fib2 = fibn;													    
    }															    
    return inFib;														    

}

bool collatzSteps(long long num)
{
  // Same as collatzSteps from codeAlong, just checks if number ever becomes 40 during the process
    while(num != 1 && num != 0 )
    {
	if(num == 40) return true;
	if(num % 2 == 0)
	{
	    num /= 2;
	}else{
	    num *= 3;
	    num++;
        }
    }
  return false;
}

