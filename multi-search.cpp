#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <unistd.h>     
#include <sys/wait.h>   
#include <sys/types.h>      
#include <signal.h>     
#include <stdlib.h>    
#include <cstdio>      
#include <errno.h>      

using namespace std;

// Function Prototypes
void child_search(const vector<string>& data, const string& key, size_t start_index, size_t end_index);
void parent_wait_and_check(vector<pid_t>& pids);


int main(int argc, char* argv[]) {
    // 1. Parse Command-Line Arguments
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <FILE NAME> <KEY> <NUMBER OF PROCESSES>" << endl;
        cerr << "Example: " << argv[0] << " strings.txt abcd 10" << endl;
        return 1;
    }
    const string filename = argv[1];
    const string key = argv[2];      
    const int num_processes = stoi(argv[3]);

    if (num_processes <= 0) {
        cerr << "Error: NUMBER OF PROCESSES must be a positive integer." << endl;
        return 1;
    }

    // 2. Load File Content (Parent Process)
    ifstream file(filename);
    if (!file.is_open()) {
        perror(("Error opening file: " + filename).c_str());
        return 1;
    }
    vector<string> data;
    string line;
    // Load a file of strings into an array (or a vector)
    while (getline(file, line)) {
        data.push_back(line); 
    }
    file.close();
    
    if (data.empty()) {
        cout << "File is empty. No string found." << endl; 
        return 0;
    }

    const size_t total_items = data.size();
    // Calculate chunk sizes: split the array into n sections
    const size_t chunk_size = total_items / num_processes;
    const size_t remainder = total_items % num_processes;
    vector<pid_t> child_pids;

    // 3. Fork Children and Assign Sections
    size_t current_start = 0;
    for (int i = 0; i < num_processes; ++i) {
        // Calculate the end index for the current section
        size_t current_end = current_start + chunk_size + (i < remainder ? 1 : 0);
        
        // Parent forks off n children
        pid_t pid = fork(); 

        if (pid < 0) {
            perror("fork");
            // Terminate any children already running before exiting
            for (pid_t p : child_pids) kill(p, SIGKILL);
            exit(-1);
        }

        if (pid == 0) {
            // Child: Search its assigned section of the shared data vector
            child_search(data, key, current_start, current_end);
            // Child exits inside child_search()
        } else {
            // Parent: Store PID for later waiting/killing
            child_pids.push_back(pid);
        }
        current_start = current_end; // Move start index for the next child
    }

    // 4. Parent Waits and Coordinates
    parent_wait_and_check(child_pids);

    return 0;
}


/**
 * @brief Executes the search logic within a child process.
 */
void child_search(const vector<string>& data, const string& key, size_t start_index, size_t end_index) {
    // Each child process shall search one of the n sections of the array
    for (size_t i = start_index; i < end_index; ++i) {
        if (data[i] == key) {
            cout << "[Child " << getpid() << "] Found key '" << key << "' at line " << i + 1 << endl;
            // Otherwise terminate with success code 0
            exit(0); 
        }
    }
    exit(1); 
}

/**
 * @brief Executes the wait and coordination logic within the parent process.
 * * Checks the exit status of terminated children and kills remaining children if a find occurs.
 */
void parent_wait_and_check(vector<pid_t>& pids) {
    int exit_status;
    int children_remaining = pids.size();
    bool key_found = false;

    // Parent continuously executes wait()
    while (children_remaining > 0) {
        // Wait for ANY child process to terminate
        pid_t terminated_pid = wait(&exit_status); 

        if (terminated_pid < 0) {
            if (errno == ECHILD) break; // No more children
            perror("wait"); 
            exit(-1);
        }

        children_remaining--;

        // Check the exit status of the child
        if (WIFEXITED(exit_status)) {
            if (WEXITSTATUS(exit_status) == 0) {
                cout << "[Parent] Child " << terminated_pid << " reported SUCCESS (Code 0)." << endl;
                key_found = true;
                break; 
            } else {
                cout << "[Parent] Child " << terminated_pid << " reported FAILURE (Code 1)." << endl;
            }
        }
    }

    if (key_found) {
        cout << "Key found! Terminating remaining children..." << endl;
        for (pid_t p : pids) {
            // Check if child is still running (waitpid with WNOHANG returns 0 if running)
            if (waitpid(p, NULL, WNOHANG) == 0) { 
                kill(p, SIGKILL);
                cout << "Killed remaining child PID: " << p << endl;
            }
        }
    } 
    else if (children_remaining == 0) {
        cout << "No string found" << endl; 
    }
}