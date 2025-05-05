/* Rosaline Scully
 * March 3, 2025
 * Student ID: 250966670
 * This program takes the input of a CSV file of different processes and their burst time.
 * It calculates the waitaround time and turnaround time for each process.
 * It averages all of these at the end of the program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROCESSES 100

// Structure to hold process information
typedef struct {
    int pid;
    float arrival_time;
    float burst_time;
    float burst_left;
    float wait_time;
    float turnaround_time;
    int completed;
    float completion_time;
} Process;

// Function to read the input file and populate the processes array
int read_input_file(const char *filename, Process processes[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    char line[100];
    int process_count = 0;
    int max_pid = -1;

    // Initialize processes array
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = i;
        processes[i].arrival_time = i; // Arrival time equals pid
        processes[i].burst_time = 0;
        processes[i].burst_left = 0;
        processes[i].wait_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].completed = 0;
        processes[i].completion_time = -1;
    }

    // Read process information from file
    while (fgets(line, sizeof(line), file) != NULL) {
        int pid;
        int burst;

        if (sscanf(line, "P%d,%d", &pid, &burst) == 2) {
            processes[pid].pid = pid;
            processes[pid].arrival_time = pid; // Arrival time equals pid
            processes[pid].burst_time = burst;
            processes[pid].burst_left = burst;

            if (pid > max_pid) max_pid = pid;
            process_count++;
        }
    }

    fclose(file);
    return max_pid + 1; // Return the number of processes read
}

// Function to find the next process to run for FCFS
int find_next_fcfs(Process processes[], int n, int current_time) {
    int next_process = -1;
    int min_arrival_time = __INT_MAX__;

    for (int i = 0; i < n; i++) {
        if (!processes[i].completed &&
            processes[i].arrival_time <= current_time &&
            processes[i].arrival_time < min_arrival_time) {
            min_arrival_time = processes[i].arrival_time;
            next_process = i;
        }
    }

    return next_process;
}

// Function to run the First Come First Served algorithm
void run_fcfs(Process processes[], int n) {
    printf("First Come First Served\n");

    int current_time = 0;
    int completed_processes = 0;
    int current_process = -1;

    while (completed_processes < n) {
        // Find the next process to run
        if (current_process == -1 || processes[current_process].completed) {
            current_process = find_next_fcfs(processes, n, current_time);
        }

        // No process is ready at this time
        if (current_process == -1) {
            current_time++;
            continue;
        }

        // Print the state at this tick
        printf("T%d : P%d - Burst left %.0f, Wait time %.0f, Turnaround time %.0f\n",
               current_time, current_process, processes[current_process].burst_left,
               processes[current_process].wait_time, processes[current_process].turnaround_time);

        // Update process state
        processes[current_process].burst_left--;

        // Check if the process has completed
        if (processes[current_process].burst_left == 0) {
            processes[current_process].completed = 1;
            processes[current_process].completion_time = current_time + 1;
            completed_processes++;
            current_process = -1; // Ready to select a new process
        }

        // Update wait and turnaround times for all processes
        for (int i = 0; i < n; i++) {
            if (!processes[i].completed && processes[i].arrival_time <= current_time) {
                if (i != current_process) {
                    // Process is waiting in the ready queue
                    processes[i].wait_time++;
                }
                // Update turnaround time for all processes that have arrived but not completed
                processes[i].turnaround_time++;
            }
        }

        current_time++;
    }

    // Print the final wait and turnaround times for each process
    for (int i = 0; i < n; i++) {
        // Calculate the final turnaround time as completion time - arrival time
        float final_turnaround;

        if (processes[i].completion_time > 0) {
            final_turnaround = processes[i].completion_time - processes[i].arrival_time;
        } else {
            // Fallback if completion time wasn't set
            final_turnaround = processes[i].burst_time + processes[i].wait_time;
        }

        printf("\nP%d\n\tWaiting time:\t\t%.0f\n\tTurnaround time:\t%.0f\n",
               i, processes[i].wait_time, final_turnaround);

        // Update the process struct with the correct final turnaround time
        processes[i].turnaround_time = final_turnaround;
    }

    // Calculate and print the average wait and turnaround times
    float avg_wait_time = 0.0;
    float avg_turnaround_time = 0.0;

    for (int i = 0; i < n; i++) {
        avg_wait_time += processes[i].wait_time;
        avg_turnaround_time += processes[i].turnaround_time;
    }

    avg_wait_time /= n;
    avg_turnaround_time /= n;

    printf("\nTotal average waiting time:\t%.1f\n", avg_wait_time);
    printf("Total average turnaround time:\t%.1f\n", avg_turnaround_time);
}

// Function to find the next process to run for Shortest Job First
int find_next_sjf(Process processes[], int n, int current_time, int current_process) {
    int next_process = -1;
    int min_burst_left = __INT_MAX__;

    for (int i = 0; i < n; i++) {
        if (!processes[i].completed && processes[i].arrival_time <= current_time) {
            if (i == current_process) {
                // If this is the current process, it has priority in case of a tie
                if (processes[i].burst_left <= min_burst_left) {
                    min_burst_left = processes[i].burst_left;
                    next_process = i;
                }
            } else {
                // For other processes, they must have strictly less burst time
                // to preempt the current process
                if (processes[i].burst_left < min_burst_left) {
                    min_burst_left = processes[i].burst_left;
                    next_process = i;
                }
            }
        }
    }

    return next_process;
}

// Function to run the Shortest Job First algorithm (preemptive)
void run_sjf(Process processes[], int n) {
    printf("Shortest Job First\n");

    float current_time = 0;
    int completed_processes = 0;
    int current_process = -1;

    while (completed_processes < n) {
        // Find the next process to run
        current_process = find_next_sjf(processes, n, current_time, current_process);

        // No process is ready at this time
        if (current_process == -1) {
            current_time++;
            continue;
        }

        // Print the state at this tick
        printf("T%.0f : P%d - Burst left %.0f, Wait time %.0f, Turnaround time %.0f\n",
               current_time, current_process, processes[current_process].burst_left,
               processes[current_process].wait_time, processes[current_process].turnaround_time);

        // Update process state
        processes[current_process].burst_left--;

        // Check if the process has completed
        if (processes[current_process].burst_left == 0) {
            processes[current_process].completed = 1;
            processes[current_process].completion_time = current_time + 1; // +1 because completion happens at the end of this tick
            completed_processes++;
        }

        // Update wait and turnaround times for all processes
        for (int i = 0; i < n; i++) {
            if (!processes[i].completed && processes[i].arrival_time <= current_time) {
                if (i != current_process) {
                    // Process is waiting in the ready queue
                    processes[i].wait_time++;
                }
                // Update turnaround time for all processes that have arrived but not completed
                processes[i].turnaround_time++;
            }
        }

        current_time++;
    }

    // Print the final wait and turnaround times for each process
    for (int i = 0; i < n; i++) {
        printf("\nP%d\n\tWaiting time:\t\t%.0f\n\tTurnaround time:\t%.0f\n",
               i, processes[i].wait_time, processes[i].turnaround_time);
    }

    // Calculate and print the average wait and turnaround times
    float avg_wait_time = 0.0;
    float avg_turnaround_time = 0.0;

    for (int i = 0; i < n; i++) {
        avg_wait_time += processes[i].wait_time;
        avg_turnaround_time += processes[i].turnaround_time;
    }

    avg_wait_time /= n;
    avg_turnaround_time /= n;

    printf("\nTotal average waiting time:\t%.1f\n", avg_wait_time);
    printf("Total average turnaround time:\t%.1f\n", avg_turnaround_time);
}

// Function to run the Round Robin algorithm
void run_rr(Process processes[], int n, int quantum, char *algorithm_type) {
    printf("Round Robin with Quantum %d\n", quantum);

    int current_time = 0;
    int completed_processes = 0;
    int current_process = -1;
    int time_in_quantum = 0;

    // Queue for ready processes
    int ready_queue[MAX_PROCESSES];
    int front = 0, rear = -1, queue_size = 0;

    // Flag to track if a process is in the queue
    int in_queue[MAX_PROCESSES] = {0};

    while (completed_processes < n) {
        // Check for newly arrived processes and add them to the queue
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time == current_time && !processes[i].completed && !in_queue[i]) {
                rear = (rear + 1) % MAX_PROCESSES;
                ready_queue[rear] = i;
                queue_size++;
                in_queue[i] = 1;
            }
        }

        // Check if we need to switch processes
        if (current_process == -1 ||
            processes[current_process].burst_left == 0 ||
            time_in_quantum >= quantum) {

            // If the current process has completed
            if (current_process != -1 && processes[current_process].burst_left == 0) {
                processes[current_process].completed = 1;
                processes[current_process].completion_time = current_time; // Record completion time
                completed_processes++;
            }
            // If the current process has burst left and has used its quantum, add it back to the queue
            else if (current_process != -1 && processes[current_process].burst_left > 0 && time_in_quantum >= quantum) {
                rear = (rear + 1) % MAX_PROCESSES;
                ready_queue[rear] = current_process;
                queue_size++;
                in_queue[current_process] = 1;
            }

            // Get a new process from the queue
            if (queue_size > 0) {
                current_process = ready_queue[front];
                front = (front + 1) % MAX_PROCESSES;
                queue_size--;
                in_queue[current_process] = 0;
                time_in_quantum = 0;
            } else {
                current_process = -1;
            }
        }

        // If no process is ready at this time, increment time and continue
        if (current_process == -1) {
            current_time++;
            continue;
        }

        // Print the state at this tick
        printf("T%d : P%d - Burst left %.0f, Wait time %.0f, Turnaround time %.0f\n",
               current_time, current_process, processes[current_process].burst_left,
               processes[current_process].wait_time, processes[current_process].turnaround_time);

        // Process the current process
        processes[current_process].burst_left--;
        time_in_quantum++;

        // Update wait and turnaround times for all processes
        for (int i = 0; i < n; i++) {
            if (!processes[i].completed && processes[i].arrival_time <= current_time) {
                if (i != current_process) {
                    // Process is waiting in the ready queue
                    processes[i].wait_time++;
                }
                // Update turnaround time for all processes that have arrived but not completed
                processes[i].turnaround_time++;
            }
        }

        current_time++;
    }

    // Print the final wait and turnaround times for each process
    for (int i = 0; i < n; i++) {
        printf("\nP%d\n\tWaiting time:\t\t%.0f\n\tTurnaround time:\t%.0f\n",
               i, processes[i].wait_time, processes[i].turnaround_time);
    }

    // Calculate and print the average wait and turnaround times
    float avg_wait_time = 0.0;
    float avg_turnaround_time = 0.0;

    for (int i = 0; i < n; i++) {
        avg_wait_time += processes[i].wait_time;
        avg_turnaround_time += processes[i].turnaround_time;
    }

    avg_wait_time /= n;
    avg_turnaround_time /= n;

    printf("\nTotal average waiting time:\t%.1f\n", avg_wait_time);
    printf("Total average turnaround time:\t%.1f\n", avg_turnaround_time);
}

// Update the main function to use the correct function signatures
int main(int argc, char *argv[]) {
    // Check if the correct number of arguments is provided
    if (argc < 3) {
        printf("Usage: %s [-f|-s|-r <quantum>] <input_file>\n", argv[0]);
        return 1;
    }

    // Parse the algorithm type
    char *algorithm = argv[1];
    char *input_file;
    int quantum = 0;

    if (strcmp(algorithm, "-f") == 0) {
        if (argc != 3) {
            printf("Usage: %s -f <input_file>\n", argv[0]);
            return 1;
        }
        input_file = argv[2];
    } else if (strcmp(algorithm, "-s") == 0) {
        if (argc != 3) {
            printf("Usage: %s -s <input_file>\n", argv[0]);
            return 1;
        }
        input_file = argv[2];
    } else if (strcmp(algorithm, "-r") == 0) {
        if (argc != 4) {
            printf("Usage: %s -r <quantum> <input_file>\n", argv[0]);
            return 1;
        }
        quantum = atoi(argv[2]);
        if (quantum <= 0) {
            printf("Quantum must be a positive integer\n");
            return 1;
        }
        input_file = argv[3];
    } else {
        printf("Unknown algorithm type: %s\n", algorithm);
        printf("Usage: %s [-f|-s|-r <quantum>] <input_file>\n", argv[0]);
        return 1;
    }

    // Read the input file
    Process processes[MAX_PROCESSES];
    int num_processes = read_input_file(input_file, processes);

    // Run the selected algorithm
    if (strcmp(algorithm, "-f") == 0) {
        run_fcfs(processes, num_processes);
    } else if (strcmp(algorithm, "-s") == 0) {
        run_sjf(processes, num_processes);
    } else if (strcmp(algorithm, "-r") == 0) {
        run_rr(processes, num_processes, quantum, algorithm);
    }

    return 0;
}
