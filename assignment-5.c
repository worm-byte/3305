/* Rosaline Scully
 * Student ID: 250966670
 * March 30th, 2025
 *
 * This program takes in a file through command line. The file contains starting account
 * information and a list of transactions. The program then uses multiple threads to
 * do the transactions, but uses mutual exclusion to ensure data-consistency.  
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 256
#define NUM_ACCOUNTS 4
#define NUM_ATMS 4

// Global variables
int account_balances[NUM_ACCOUNTS]; // Array to store account balances
pthread_mutex_t account_locks[NUM_ACCOUNTS]; // One mutex per account

// Structure to pass data to threads
typedef struct {
    int atm_id;
    char* filename;
} thread_data_t;

// Function to process transactions for a specific ATM
void* process_transactions(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int atm_id = data->atm_id;
    char* filename = data->filename;

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        pthread_exit(NULL);
    }

    char line[MAX_LINE_LENGTH];

    // Skip header line
    fgets(line, MAX_LINE_LENGTH, file);

    // Skip account balance lines (first 4 lines after header)
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        fgets(line, MAX_LINE_LENGTH, file);
    }

    // Process transaction lines
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        int transaction_atm, account, amount;

        // Parse the line
        sscanf(line, "%d,%d,%d", &transaction_atm, &account, &amount);

        // Check if this transaction belongs to this ATM
        if (transaction_atm == atm_id) {
            // Account numbers are 1-based, adjust to 0-based for array indexing
            int account_idx = account - 1;

            // Try to acquire the lock for this account
            pthread_mutex_lock(&account_locks[account_idx]);

            // Check if it's a withdrawal and if there are sufficient funds
            if (amount < 0 && account_balances[account_idx] + amount < 0) {
                printf("Thread %d: Withdraw $%d from Account %d - INSUFFICIENT FUNDS\n",
                       atm_id, -amount, account);
            } else {
                // Perform the transaction
                account_balances[account_idx] += amount;

                if (amount > 0) {
                    printf("Thread %d: Deposit $%d into Account %d\n", atm_id, amount, account);
                } else {
                    printf("Thread %d: Withdraw $%d from Account %d\n", atm_id, -amount, account);
                }
            }

            // Release the lock
            pthread_mutex_unlock(&account_locks[account_idx]);
        }
    }

    fclose(file);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    char line[MAX_LINE_LENGTH];

    // Skip header line
    fgets(line, MAX_LINE_LENGTH, file);

    // Read initial account balances
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        char atm[10];
        int account, balance;

        fgets(line, MAX_LINE_LENGTH, file);
        sscanf(line, "%[^,],%d,%d", atm, &account, &balance);

        // Account numbers are 1-based, adjust to 0-based for array indexing
        account_balances[account - 1] = balance;
    }

    // Close the file as we'll reopen it in each thread
    fclose(file);

    // Print initial balances
    printf("Balance \n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d \n", i + 1, account_balances[i]);
    }
    printf("\n");

    // Initialize mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_init(&account_locks[i], NULL);
    }

    // Create threads
    pthread_t threads[NUM_ATMS];
    thread_data_t thread_data[NUM_ATMS];

    for (int i = 0; i < NUM_ATMS; i++) {
        thread_data[i].atm_id = i + 1; // ATM IDs are 1-based
        thread_data[i].filename = filename;

        if (pthread_create(&threads[i], NULL, process_transactions, &thread_data[i]) != 0) {
            printf("Error creating thread %d\n", i + 1);
            return 1;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_ATMS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final balances
    printf("\nBalance \n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d \n", i + 1, account_balances[i]);
    }

    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&account_locks[i]);
    }

    return 0;
}
