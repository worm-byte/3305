/*
 *Rosaline Scully
 *February 8, 2025
 *Student ID: 250966670
 *
 *This program calculates a large number by passing smaller values back and forth
 *between a parent and child process and doing minor calculations on them.
 *You will be able to see what is being passed at each point.
 *
 *It allows values between 1000-9999 for calculations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#define N 4

int main(int argc, char *argv[]) {
    long X, Y, Z;

    if (argc != 3) {
        fprintf(stderr, "Error: Incorrect number of arguments. Please type in two numbers between 1000-9999.\n");
        exit(1);  // Changed to exit(1) for error conditions
    }

    int firstNum = atoi(argv[1]);
    int secondNum = atoi(argv[2]);

    // check if both conversions were successful and if the number is within 1000-9999
    if (firstNum == 0 && *argv[1] != '\0') {
        fprintf(stderr, "Error: Invalid first argument.\n");
        exit(1);
    }
    if (firstNum < 1000 || firstNum > 9999) {
        fprintf(stderr, "Error: First argument is out of range.\n");
        exit(1);
    }

    if (secondNum == 0 && *argv[2] != '\0') {
        fprintf(stderr, "Error: Invalid second argument.\n");
        exit(1);
    }
    if (secondNum < 1000 || secondNum > 9999) {
        fprintf(stderr, "Error: Second argument is out of range.\n");
        exit(1);
    }

    //partition the numbers
    int a1 = firstNum/100;
    int a2 = firstNum%100;
    int b1 = secondNum/100;
    int b2 = secondNum%100;

    int pipe1[2]; //pipe parent to child
    int pipe2[2]; //pipe child to parent

    // create both pipes
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        perror("pipe failed");
        exit(1);
    }

    printf("Your integers are %d %d\n", firstNum, secondNum);
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid > 0) { //parent
        printf("Parent (PID %d): created child (PID %d)\n", getpid(), pid);
        close(pipe1[0]); // close unused read end
        close(pipe2[1]); //close unused write end

        printf("\n###\n");
        printf("# Calculating X\n");
        printf("###\n");

        //send a1 and b1 to child for a calculation
        printf("Parent (PID %d): sending %d to child\n", getpid(), a1);
        write(pipe1[1], &a1, sizeof(a1));

        printf("Parent (PID %d): sending %d to child\n", getpid(), b1);
        write(pipe1[1], &b1, sizeof(b1));

        // read result from child
        long r;
        read(pipe2[0], &r, sizeof(r));
        printf("Parent (PID %d): received %ld from child\n", getpid(), r);

        //calculate X
        X = r * pow(10, N);

        printf("\n###\n");
        printf("# Calculating Y\n");
        printf("###\n");

        //send a2 and b1 to child for calculation
        printf("Parent (PID %d): sending %d to child\n", getpid(), a2);
        write(pipe1[1], &a2, sizeof(a2));

        printf("Parent (PID %d): sending %d to child\n", getpid(), b1);
        write(pipe1[1], &b1, sizeof(b1));

        //store B calculation
        long r1;
        read(pipe2[0], &r1, sizeof(r1));
        printf("Parent (PID %d): received %ld from child\n", getpid(), r1);

        //send a1 and b2 to child for calculation
        printf("Parent (PID %d): sending %d to child\n", getpid(), a1);
        write(pipe1[1], &a1, sizeof(a1));

        printf("Parent (PID %d): sending %d to child\n", getpid(), b2);
        write(pipe1[1], &b2, sizeof(b2));

        //store C calculation
        long r2;
        read(pipe2[0], &r2, sizeof(r2));
        printf("Parent (PID %d): received %ld from child\n", getpid(), r2);

        //calculate Y and store
        Y = (r1 + r2) * pow(10, N/2);

        printf("\n###\n");
        printf("# Calculating Z\n");
        printf("###\n");

        //send a2 and b2 to child for calculation
        printf("Parent (PID %d): sending %d to child\n", getpid(), a2);
        write(pipe1[1], &a2, sizeof(a2));

        printf("Parent (PID %d): sending %d to child\n", getpid(), b2);
        write(pipe1[1], &b2, sizeof(b2));

        //store D calculation
        read(pipe2[0], &Z, sizeof(Z));  // Fixed: Changed from pipe2[1] to pipe2[0]
        printf("Parent (PID %d): received %ld from child\n", getpid(), Z);


        // close remaining pipe ends
        close(pipe1[1]);
        close(pipe2[0]);

        // Calculate final sum before waiting for child
        long sum = X + Y + Z;
        printf("\n%d*%d == %ld + %ld + %ld == %ld\n", firstNum, secondNum, X, Y, Z, sum);

        wait(NULL);
    } else { //child
        close(pipe1[1]); // close unused write end
        close(pipe2[0]); // close unused read end

        //receive a1 and b1 from parent
        int received_a1, received_b1;
        read(pipe1[0], &received_a1, sizeof(received_a1));
        printf("Child (PID %d): received %d from parent\n", getpid(), received_a1);

        read(pipe1[0], &received_b1, sizeof(received_b1));
        printf("Child (PID %d): received %d from parent\n", getpid(), received_b1);

        // send a1*b1 back to parent
        int A = received_a1 * received_b1;
        printf("Child (PID %d): sending %d to parent\n", getpid(), A);
        write(pipe2[1], &A, sizeof(A));

        //receive a2 and b1 from parent
        int received_a2, received2_b1;
        read(pipe1[0], &received_a2, sizeof(received_a2));
        printf("Child (PID %d): received %d from parent\n", getpid(), received_a2);

        read(pipe1[0], &received2_b1, sizeof(received2_b1));
        printf("Child (PID %d): received %d from parent\n", getpid(), received2_b1);

        //send a2*b1 back to parent
        int B = received_a2 * received2_b1;
        printf("Child (PID %d): sending %d to parent\n", getpid(), B);
        write(pipe2[1], &B, sizeof(B));

        //receive a1 and b2 from parent
        int received2_a1, received_b2;
        read(pipe1[0], &received2_a1, sizeof(received2_a1));
        printf("Child (PID %d): received %d from parent\n", getpid(), received2_a1);

        read(pipe1[0], &received_b2, sizeof(received_b2));
        printf("Child (PID %d): received %d from parent\n", getpid(), received_b2);

        //send a1 * b2 to parent
        int C = received2_a1 * received_b2;
        printf("Child (PID %d): sending %d to parent\n", getpid(), C);
        write(pipe2[1], &C, sizeof(C));

        //receive a2 and b2 from parent
        int received2_a2, received2_b2;
        read(pipe1[0], &received2_a2, sizeof(received2_a2));
        printf("Child (PID %d): received %d from parent\n", getpid(), received2_a2);

        read(pipe1[0], &received2_b2, sizeof(received2_b2));
        printf("Child (PID %d): received %d from parent\n", getpid(), received2_b2);

        //send a2*b2 to parent
        int D = received2_a2 * received2_b2;
        printf("Child (PID %d): sending %d to parent\n", getpid(), D);
        write(pipe2[1], &D, sizeof(D));

        // Close remaining pipe ends
        close(pipe1[0]);
        close(pipe2[1]);

        exit(0);
    }

    return 0;
}
