
//This is a test for github
#include <iostream>
#include <pthread.h>
#include <string>
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

//bool quit = false;
int N_THREADS = 0;

//prototypes
void* readInput(void*);
void* filterOdds(void*);
void* filterFib(void*);
void* filterCollatz(void*);
void* readFinalQueue(void*);

bool isOdd(long long);
bool inFibonacci(long long);
bool collatzSteps(long long);

int main(int argc, char** argv){
  // NOTE : Numbers that pass odd filter but not fibonacci (i.e. 7) break the program...
  N_THREADS = stoi(argv[1]);
  
  pthread_t inputThread;
  pthread_create(&inputThread, NULL, readInput, NULL);
  
  pthread_t filterThread1[N_THREADS];
  for(int i = 0; i < N_THREADS; i++)
    {
      pthread_create(&filterThread1[i], NULL, filterOdds, NULL);
    }
  
  pthread_t filterThread2[N_THREADS];
  for(int i = 0; i < N_THREADS; i++)
    {
      pthread_create(&filterThread2[i], NULL, filterFib, NULL);
    }
  
  pthread_t filterThread3[N_THREADS];
  for(int i = 0; i < N_THREADS; i++)
    {
      pthread_create(&filterThread3[i], NULL, filterCollatz, NULL);
    }
	
  pthread_t printThread;
  pthread_create(&printThread, NULL, readFinalQueue, NULL);
  
  pthread_join(inputThread, NULL);
  for(int i = 0; i < N_THREADS; i++)
  {
    pthread_join(filterThread1[i], NULL);
    pthread_join(filterThread2[i], NULL);
    pthread_join(filterThread3[i], NULL);
  }
  pthread_join(printThread, NULL);
	
  return 0;
}

void* readInput(void* arg){
  long long input;

  do{
    cin >> input;
    //locking the first queue
    pthread_mutex_lock(&inputLock);
    inputQueue.push(input);
    pthread_mutex_unlock(&inputLock);
  }while(input != 0);//stops when 0 comes up

  for(int i = 0; i < N_THREADS && input == 0; i++)
  {
    inputQueue.push(input);
  }
return NULL;
}

void* filterOdds(void* arg){
  long long number;
  bool quit = false;
  do{
    pthread_mutex_lock(&inputLock);
    while(inputQueue.empty()){//spins while queue is empty
      pthread_mutex_unlock(&inputLock);
      pthread_mutex_lock(&inputLock);
    }
    number = inputQueue.front();
    inputQueue.pop();
    pthread_mutex_unlock(&inputLock);

    quit = (number == 0);
		
    if(isOdd(number) || number == 0){
      pthread_mutex_lock(&oddLock);
      oddQueue.push(number);//transfer num from first queue to second
      pthread_mutex_unlock(&oddLock);
      //cout << "Made it through odd filter :" << number << endl;
    }

  }while(!quit);

return NULL;
}

void* filterFib(void* arg){ //almost the exact same as filterOdds()
  long long number;
  bool quit = false;
  do{
    pthread_mutex_lock(&oddLock);
    while(oddQueue.empty()){
      pthread_mutex_unlock(&oddLock);
      pthread_mutex_lock(&oddLock);
    }
    number = oddQueue.front();
    oddQueue.pop();
    pthread_mutex_unlock(&oddLock);

    quit = (number == 0);
		
    if(inFibonacci(number) || number == 0){
      pthread_mutex_lock(&fibLock);
      fibQueue.push(number);
      pthread_mutex_unlock(&fibLock);
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
    pthread_mutex_lock(&fibLock);
    //cout << "collatz is about to start spinning " << endl;
    while(fibQueue.empty()){
      pthread_mutex_unlock(&fibLock);
      pthread_mutex_lock(&fibLock);
    }
    number = fibQueue.front();
    fibQueue.pop();
    pthread_mutex_unlock(&fibLock);
    //cout << "collatz made it past spin lock " << endl;
    quit = (number == 0);

    //cout << "collatz made it past quit statement" << endl;
    if(collatzSteps(number) || number == 0){
      pthread_mutex_lock(&finalLock);
      finalQueue.push(number);
      pthread_mutex_unlock(&finalLock);
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
      pthread_mutex_lock(&finalLock);
      while(finalQueue.empty())
	{
	  pthread_mutex_unlock(&finalLock);
	  pthread_mutex_lock(&finalLock);
	}

      if(finalQueue.front() == 0)
	{
	  counter++;
	}
      quit = (counter == N_THREADS);
      if(finalQueue.front() != 0) cout << "This number made it through all the filters: " << finalQueue.front()  << endl;
      finalQueue.pop();
	
      pthread_mutex_unlock(&finalLock);
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

