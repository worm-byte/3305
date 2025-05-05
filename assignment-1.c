/*
 *Rosaline Scully
 *Student ID: 250966670
 *January 20, 2025
 *
 *This program counts the number of prime numbers and the sum of them within a range.
 *It will split the counting into 4, and if parallel is selected, it will split the work
 *amount 4 children processes using modulo for even distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int isPrime(int number);
void countSumPrimes(int min, int max, int *count, long long int *sum);

int main(int argc, char *argv[]) {
    int mode = atoi(argv[1]); //0 is series, 1 or greater is parallel
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);

    int count;
    long long int sum;
    int totalNums = max - min + 1;
    int numbersPerProcess = totalNums / 4;
    int remainder = totalNums % 4;

    //if in series, proceed with this branch
    if (mode == 0) {
        printf("Process id: %d\n", getpid());
        int currMin = min;

        for (int i = 0; i < 4; i++) {
            int rangeSize = numbersPerProcess;
            if (i < remainder) {  // Distribute remainder evenly
                rangeSize++;
            }
            int currMax = currMin + rangeSize - 1;

            countSumPrimes(currMin, currMax, &count, &sum);

            printf("pid: %d,", getpid());
            printf("ppid: %d - ", getppid());
            printf("Count and sum of prime numbers between %d and %d are %d and %lld\n",
                   currMin, currMax, count, sum);

            currMin = currMax + 1;
        }
    }
    else {
        //if in parallel, proceed with this branch
        printf("Process id: %d\n", getpid());

        for (int i = 0; i < 4; i++) {
            pid_t pid = fork();
            if (pid == 0) { //if a child process, proceed within if statement
                int rangeSize = numbersPerProcess;
                if (i < remainder) {  // Distribute remainder evenly
                    rangeSize++;
                }

                // Calculate start and end points for this process
                int start = min;
                for (int j = 0; j < i; j++) {
                    start += numbersPerProcess + (j < remainder ? 1 : 0);
                }
                int end = start + rangeSize - 1;

                countSumPrimes(start, end, &count, &sum);

                printf("pid: %d,", getpid());
                printf("ppid: %d - ", getppid());
                printf("Count and sum of prime numbers between %d and %d are %d and %lld\n",
                       start, end, count, sum);

                exit(0); // exit child process
            }
        }

        //parent process must wait for all child processes to finish
        for (int i = 0; i < 4; i++) {
            wait(NULL);
        }
    }
}

//function that checks if a number is prime or not. returns 1 if prime.
int isPrime(int number) {
    if (number < 2) {
        return 0;
    }
    // Prime Number Check Program (For loop only)
    // Source: Programiz (https://www.programiz.com/c-programming/examples/prime-number)
    // Accessed on: Jan. 17, 2025
    for (int i = 2; i <= number / 2; ++i) {
        // if n is divisible by i, then n is not prime
        // return 0 if non-prime number
        if (number % i == 0) {
            return 0;
        }
    }
    return 1;
}

//function that will count the number of primes within a range and sum them
void countSumPrimes(int min, int max, int *count, long long int *sum) {
    *count = 0;
    *sum = 0;
    for (int i = min; i <= max; i++) {
        if (isPrime(i)) {
            (*count)++;
            (*sum) += i;
        }
    }
}
