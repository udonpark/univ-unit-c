#include <stdio.h>
#include <string.h>
#define MAX_PROCESS 100

/*
Yo Kogure
32134541
Start date: 7 Oct 2021
Last modified: 9 Oct 2021

I will implement based on FCFS(First Come, First Served) scheduling algorithms.
*/

// Assumption that all arrival times are in ascending order.
// There can be a duplicate arrival times in the source file.

typedef enum { // enum which represents the state processes are in
    READY, RUNNING, EXIT, TERMINATED
} process_state_t;

typedef struct pcb_t{ // struct containing information of the process
    char process_name[11];

    int entryTime;
    int serviceTime;
    int remainingTime;
    int deadlineTime;

    // added few variables to store, used to output to the file later on
    int waitTime;
    int turnaroundTime;
    int deadlineMet;
    
    process_state_t state;
} PCB_T;

// this is to store all processes that are in the text file.
// very similar structure to pcb_t, but this first converts the text in process[] format, which we can deal with easier
typedef struct process{
    char process_name[11];
    int entryTime;
    int serviceTime;
    int remainingTime;
    int deadlineTime;
} PROCESS;

/**
 * A function that reads all data in a the given file, and read one by line to store them
 * as an argument in a process list.
 * returns: null, as necessary actions are performed inside the function
 */
void read_all(PROCESS process[], int line, FILE* fp){
    fscanf(fp, "%s %d %d %d\n", process[line].process_name, &process[line].entryTime, &process[line].serviceTime, &process[line].deadlineTime);
    process[line].remainingTime = process[line].serviceTime; // reads line one by one in the format.
    // remainingTime and serviceTime are same initially, except that remainingTime is used to keep track as a counter
}


/**
 * A function that checks if any process in the is still running. Assumption that only one program is currently running.
 * returns: an integer, the index number if any program is running, and -1 otherwise
 */
int is_running(PCB_T pcb_t[], int line_max){
    // this method returns the index if any program is running, and -1 otherwise
    for (int i = 0; i < line_max; i++){
        // if (pcb_t[i] == NULL){
        //     return -1;
        // }
        if (pcb_t[i].state == RUNNING){
            return i; // if any is running, return that index
        }
    }
    return -1;
}

/**
 * A function that checks if any running process have lead to completion.
 * returns: integer , the index number if any program which is running has a remaining time of 0, and -1 if not found
 */
int is_complete(PCB_T pcb_t[], int line_max){
    for (int i = 0; i < line_max; i++){
        if (pcb_t[i].remainingTime == 0 && pcb_t[i].state == RUNNING){
            return i;  // find if any running process is complete
        }
    }
    return -1;
}

/**
 * A function that finds the total time required to complete the process
 * returns: the integer sum of all serviceTime of the taken processes stored in process[]
 */
int total_time(PROCESS process[], int line_max){
    int sum = 0;
    for (int i; i <= line_max; i++){
        sum += process[i].serviceTime; // sum of service time
        }
    sum += process[0].entryTime;
    return sum;
}

/**
 * A function that makes the First-Served ready process to running.
 * This is used to make the next-waiting process to RUNNING.
 * returns: integer index number if successfully changed, and -1 if failed to do so.
 */

int make_running(PCB_T pcb_t[], int line_max){
    for (int i = 0; i < line_max; i++){
        if (pcb_t[i].state == READY){
            pcb_t[i].state = RUNNING;
            return i;
        }
    }
    return -1;
}

/**
 * This function is used to 'tick' the processes that are in READY state.
 * When other process is running, this function is responsible to increase waitTime of all READY processes.
 * returns: none, as it does not require any boolean check in my implementation
 */
void tick_waiting(PCB_T pcb_t[], int line_max){
    for (int i = 0; i < line_max; i++){
        if (pcb_t[i].state == READY){
            pcb_t[i].waitTime++;
        }
    }
}

/**
 * This function is used to output the desired variables into an output_file.
 * When this is called, it opens required file, and perform a for-loop to iterate each lines stored in pcb_t,
 * calculates turnaroundTime, deadlineMet and prints out to a destination file with the specified format.
 * fopen() and fprintf() are used here to print out to a text file.
 */
void print_result(PCB_T pcb_t[], int line_max){
    FILE *op_file = fopen("results-task1.txt", "w");
    for (int i = 0; i < line_max; i++){
        pcb_t[i].turnaroundTime = pcb_t[i].waitTime + pcb_t[i].serviceTime;  // this is by the definition of turnaroundTime
        if (pcb_t[i].turnaroundTime <= pcb_t[i].deadlineTime){ // if deadline met, set it to 1
            pcb_t[i].deadlineMet = 1;
        }
        else{
            pcb_t[i].deadlineMet = 0;
        }
        // output accordingly in specified format
        fprintf(op_file,"%s %d %d %d", pcb_t[i].process_name, pcb_t[i].waitTime, pcb_t[i].turnaroundTime, pcb_t[i].deadlineMet); 
        if (i != line_max -1){
            fprintf(op_file, "\n");  // if not the last line, add a new line
        }
    }
}

/* I have assumed that text files are not corrupted, as shown in the specification sheet */

/**
 * This is a main function that performs all the required characteristics.
 * For Task 1, please refer to the specificatio sheet or the user manual submitted together with this file.
 * It will take in a file name as an argument (if necessary only. default leads to processes.txt), open it and store as
 * process[]. It will then perform the simulation one second by second, represented by i in the for-loop.
 * It will follow First Come First Served scheduling algorithm, and it is implemented by using the above functions.
 * 
 */
int main(int argc, char *argv[]){
    /* Referred to https://stackoverflow.com/questions/8625740/how-do-i-define-and-pass-in-a-file-name-to-fopen-from-command-line
to accept filename as an argument */
    FILE *fp_file;
    if (argc >= 2){
        fp_file = fopen(argv[1], "r");
    }
    else {
        fp_file = fopen("processes.txt", "r");
    }

    PCB_T pcb_t[MAX_PROCESS]; 
    PROCESS process[MAX_PROCESS];
    // int current_time = 0;
    int current_line = 0;
    int line_num = 0;


    // I must not store the information of a process which has not yet arrived.
    // first, obtain all information into the processes struct, then into pcb_t
    while (!feof(fp_file)){
        read_all(process, line_num, fp_file);
        line_num++;
    }


    fclose(fp_file);

    int max_line = line_num;
    int pcb_length = 0;

    int total_t = total_time(process, max_line);
    /* Note that this is on First Come, First Served basis */
    // i represents current time at each iteration, in every tick

    int has_arrived = 0;  // boolean to see if ANY FIRST process has already arrived in this program
    for (int i = 0; i < total_t; i++){
        for (int j = 0; j <= line_num; j++){ // 
            if (process[j].entryTime == i){  // if it is to be arrived, then add it to PCB block
                // char temp[11] = process[j].process_name;
                strcpy(pcb_t[j].process_name, process[j].process_name);
                has_arrived = 1;
                pcb_t[j].entryTime = process[j].entryTime;
                pcb_t[j].serviceTime = process[j].serviceTime;
                pcb_t[j].remainingTime = process[j].remainingTime;
                pcb_t[j].deadlineTime = process[j].deadlineTime;
                pcb_t[j].waitTime = 0;
                pcb_t[j].state = READY;
                printf("Time %d:    %s has entered the system.\n", i, pcb_t[j].process_name);
                pcb_length++;
            } // it adds whichever process that are ready into the pcb
        }
        if (!has_arrived){ // if first process is not found in time 0, it is a problem
            continue;
        }
        if (is_running(pcb_t, pcb_length) == -1){  // if no process is currently running
            int running = make_running(pcb_t, pcb_length);  // this will make the waiting process to running, in order!
            if (running == -1){ // if no more process to run, end the loop
                printf("All processes complete!\n");
                break;
            }
            printf("Time %d:    %s is in the running state.\n", i, pcb_t[running].process_name);
            continue;
        }

        else{
            int running = is_running(pcb_t, pcb_length);
            pcb_t[running].remainingTime--;
        }

        if (is_complete(pcb_t, pcb_length) != -1){
            int complete = is_complete(pcb_t, pcb_length);
            pcb_t[complete].state = EXIT;
            printf("Time %d:    %s has finished execution.\n", i, pcb_t[complete].process_name);
            pcb_t[complete].state = TERMINATED; // when it is TERMINATED, I assume that these processes are not stored anymore
        }

        // after termination, it looks for the next process in the queue
        if (is_running(pcb_t, pcb_length) == -1){  // if no process is currently running
            int running = make_running(pcb_t, pcb_length);  // this will make the waiting process to running, in order!
            if (running == -1){
                printf("All processes complete!\n");
                break;
            }
            printf("Time %d:    %s is in the running state.\n", i, pcb_t[running].process_name);
        }
        

        if (i != 0){ // if time = 0, do not tick as it has not waited any second
            tick_waiting(pcb_t, pcb_length);
        }
    }
    print_result(pcb_t, max_line);
}