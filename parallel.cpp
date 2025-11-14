#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>     
#include <sys/wait.h>   
#include <sys/types.h>  
#include <stdlib.h>     
#include <cstdio> 

using namespace std;

const string BASE_URL_PREFIX = "https://api.open-meteo.com/v1/forecast?latitude=";
const string LON_URL_MIDFIX = "&longitude=";
const string BASE_URL_SUFFIX = "&current_weather=True";


string generate_url(const string& latitude, const string& longitude) {
    return BASE_URL_PREFIX + latitude + LON_URL_MIDFIX + longitude + BASE_URL_SUFFIX;
}


int main() {
    vector<pid_t> child_pids;
    int location_count = 0;

    // 1. Read locations from input.txt
    ifstream input_file("locations.txt");
    if (!input_file.is_open()) {
        cerr << "Error: Could not open locations.txt file. Make sure it's in the current directory." << endl;
        return 1;
    }

    string line;
    // loop through each line in the input file
    while (getline(input_file, line)) {
        stringstream ss(line);
        string latitude, longitude;
        
        if (!(ss >> latitude >> longitude)) {
            cerr << "Warning: Skipping malformed line in input file: " << line << endl;
            continue;
        }

        location_count++;

        // --- PARENT forks a child for each location ---
        pid_t pid = fork();

        // Check for fork() error
        if (pid < 0) {
            perror("fork");
            // terminate the parent process on critical error
            exit(-1);
        }

        // --- Child Process Logic (pid == 0) ---
        else if (pid == 0) {
            

            // Construct the output filename (e.g., file1.json)
            string output_filename = "file" + to_string(location_count) + ".json";
            
            // Construct the full URL
            string url = generate_url(latitude, longitude);

            // Print some process information for tracking (optional but useful)
            cout << "[Child " << location_count << "] PID: " << getpid() 
                 << ", Fetching (" << latitude << ", " << longitude << ") to " 
                 << output_filename << endl; 

            // execlp() replaces the current process image [cite: 32, 105]
            if (execlp("/usr/bin/curl", "curl", "-o", output_filename.c_str(), url.c_str(), (char *)NULL) == -1) {
                // If execlp fails, it returns -1[cite: 43], so we check and print error
                perror("execlp");
                // The child must exit after execlp failure [cite: 50]
                exit(-1);
            }
            // Code after execlp() is only executed if execlp fails
        } 

        // --- Parent Process Logic (pid > 0) ---
        else {
            // Store child PID for later waiting
            child_pids.push_back(pid);
        }
    } // End of location reading loop

    input_file.close();

    // 2. Parent waits for all children to terminate
    int status;
    size_t num_children = child_pids.size();
    cout << "\nParent process (PID: " << getpid() << ") has forked " 
         << num_children << " children and is now waiting for all to finish..." << endl;

    for (size_t i = 0; i < num_children; ++i) {
        pid_t terminated_pid = wait(&status); // Wait for ANY child

        // Check for wait() error
        if (terminated_pid < 0) {
            perror("wait");
            exit(-1);
        }
        
        cout << "[Parent] Child PID " << terminated_pid << " terminated." << endl;
    }

    // 3. Parent exits
    cout << "All children terminated. Parallel fetcher process exiting normally." << endl;
    return 0;
}