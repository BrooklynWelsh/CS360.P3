#include <iostream>

using namespace std;

int main(){
    long long number = 0;
    const long long MAX_SIZE = 259695496911122585;
    long long fib1 = 1;
    long long fib2 = 1;


    while(number < MAX_SIZE){

	    number = fib1 + fib2;

	    cout << number << endl;

	    fib1 = fib2;
	    fib2 = number;
        }
    cout << 0 << endl;
    }

