/* Rosaline Scully
 * March 1, 2025
 * Student ID: 250966670
 *
 * This program tests if a sudoku solution is valid.
 * Please run with ./assignment-3 valid_sudoku.txt.
 * Or for testing invalid sudoku run ./assignment-3 invalid_sudoku.txt.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// global variables
int sudoku[9][9]; // sudoku grid
bool thread_results[27]; // results from each thread (true = valid, false = invalid)

// structure to pass thread parameters
typedef struct {
    int thread_id;
    int index; // row, column, or subgrid index
} thread_param;

// function to check if a row is valid (values 1-9 only seen once)
void* check_row(void* param) {
    thread_param* p = (thread_param*)param;
    int thread_id = p->thread_id;
    int row = p->index;

    // create a flag array to check if a digit appears more than once
    bool digit_seen[9] = {false};

    for (int col = 0; col < 9; col++) {
        int digit = sudoku[row][col] - 1;

        // if digit already seen, row is invalid
        if (digit_seen[digit]) {
            thread_results[thread_id - 1] = false;
            printf("Thread # %2d (row %d) is INVALID\n", thread_id, row + 1);
            free(param);
            return NULL;
        }

        digit_seen[digit] = true;
    }

    // ff we get here, row is valid
    thread_results[thread_id - 1] = true;
    printf("Thread # %2d (row %d) is valid\n", thread_id, row + 1);
    free(param);
    return NULL;
}

// function to check if a column is valid (values 1-9 only seen once)
void* check_column(void* param) {
    thread_param* p = (thread_param*)param;
    int thread_id = p->thread_id;
    int col = p->index;

    // create a flag array to check if a digit appears more than once
    bool digit_seen[9] = {false};

    for (int row = 0; row < 9; row++) {
        int digit = sudoku[row][col] - 1;

        // if digit already seen, column is invalid
        if (digit_seen[digit]) {
            thread_results[thread_id - 1] = false;
            printf("Thread # %2d (column %d) is INVALID\n", thread_id, col + 1);
            free(param);
            return NULL;
        }

        digit_seen[digit] = true;
    }

    // if we get here, column is valid
    thread_results[thread_id - 1] = true;
    printf("Thread # %2d (column %d) is valid\n", thread_id, col + 1);
    free(param);
    return NULL;
}

// function to check if a subgrid is valid
void* check_subgrid(void* param) {
    thread_param* p = (thread_param*)param;
    int thread_id = p->thread_id;
    int subgrid = p->index;

    // convert subgrid index to starting row and column
    int start_row = (subgrid / 3) * 3;
    int start_col = (subgrid % 3) * 3;

    // create a flag array to check if a digit appears more than once
    bool digit_seen[9] = {false};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int row = start_row + i;
            int col = start_col + j;
            int digit = sudoku[row][col] - 1;

            // if digit already seen, subgrid is invalid
            if (digit_seen[digit]) {
                thread_results[thread_id - 1] = false;
                printf("Thread # %2d (subgrid %d) is INVALID\n", thread_id, subgrid + 1);
                free(param);
                return NULL;
            }

            digit_seen[digit] = true;
        }
    }

    // if we get here, subgrid is valid
    thread_results[thread_id - 1] = true;
    printf("Thread # %2d (subgrid %d) is valid\n", thread_id, subgrid + 1);
    free(param);
    return NULL;
}

int main(int argc, char* argv[]) {
    // check if a filename was provided
    if (argc != 2) {
        printf("Usage: %s <sudoku_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // open the file
    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // read the Sudoku solution from the file
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (fscanf(file, "%d", &sudoku[row][col]) != 1) {
                printf("Error reading Sudoku grid from file\n");
                fclose(file);
                return EXIT_FAILURE;
            }
        }
    }

    // close the file
    fclose(file);

    // create threads to check rows, columns, and subgrids
    pthread_t threads[27];
    int thread_count = 0;

    // create 9 threads to check subgrids
    for (int i = 0; i < 9; i++) {
        thread_param* param = (thread_param*)malloc(sizeof(thread_param));
        param->thread_id = thread_count + 1;
        param->index = i;

        //create thread and check subgrid with the passed parameters
        pthread_create(&threads[thread_count], NULL, check_subgrid, param);

        thread_count++;
    }

    // create 9 threads to check rows
    for (int i = 0; i < 9; i++) {
        thread_param* param = (thread_param*)malloc(sizeof(thread_param));
        param->thread_id = thread_count + 1;
        param->index = i;

        //create thread and check row with the passed parameters
        pthread_create(&threads[thread_count], NULL, check_row, param);

        thread_count++;
    }

    // create 9 threads to check columns
    for (int i = 0; i < 9; i++) {
        thread_param* param = (thread_param*)malloc(sizeof(thread_param));
        param->thread_id = thread_count + 1;
        param->index = i;

        //create thread and check row with the passed parameters
        pthread_create(&threads[thread_count], NULL, check_column, param);

        thread_count++;
    }

    // join all threads
    for (int i = 0; i < 27; i++) {
        pthread_join(threads[i], NULL);
    }

    // check if all threads reported the grid as valid
    bool is_valid = true;
    for (int i = 0; i < 27; i++) {
        if (!thread_results[i]) {
            is_valid = false;
            break;
        }
    }

    // print final verdict
    if (is_valid) {
        printf("%s contains a valid solution\n", argv[1]);
    } else {
        printf("%s contains an INVALID solution\n", argv[1]);
    }

    return EXIT_SUCCESS;
}
