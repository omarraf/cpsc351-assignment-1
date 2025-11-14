```
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// Structure to store one city's coordinate pair
struct City {
    double latitude;
    double longitude;
};

int main() {
    // Try opening the input file with all coordinates
    ifstream locationFile("locations.txt");
    if (!locationFile.is_open()) {
        cerr << "Error: Could not open locations.txt — please make sure the file exists." << endl;
        return -1;
    }

    // Load all cities from the file
    vector<City> cities;
    double lat, lon;

    while (locationFile >> lat >> lon) {
        cities.push_back({lat, lon});
    }
    locationFile.close();

    if (cities.empty()) {
        cout << "No city coordinates found in locations.txt. Nothing to do." << endl;
        return 0;
    }

    cout << "Preparing to fetch weather data for " 
         << cities.size() << " cities..." << endl;

    // Process each city one-by-one
    for (size_t index = 0; index < cities.size(); ++index) {

        cout << "\n--- Starting city " << (index + 1) 
             << " of " << cities.size() << " ---" << endl;

        pid_t pid = fork();

        // Fork failed
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        // ----------------------
        //       CHILD PROCESS
        // ----------------------
        if (pid == 0) {
            cout << "Child process created." << endl;
            cout << "  PID: " << getpid() << endl;
            cout << "  Parent PID: " << getppid() << endl;
            cout << "  UID: " << getuid() 
                 << " | GID: " << getgid() << endl;

            // Output filename for this city
            string outputName = "city_weather_" + to_string(index + 1) + ".json";

            // Build the Open-Meteo API URL
            string url =
                "https://api.open-meteo.com/v1/forecast?"
                "latitude=" + to_string(cities[index].latitude) +
                "&longitude=" + to_string(cities[index].longitude) +
                "&current_weather=true";

            cout << "Fetching weather and saving to " << outputName << endl;

            // Replace this process with curl
            if (execlp("/usr/bin/curl", "curl",
                       "-o", outputName.c_str(),
                       url.c_str(),
                       NULL) < 0)
            {
                perror("execlp");
                _exit(-1);
            }
        }

        // ----------------------
        //       PARENT PROCESS
        // ----------------------
        else {
            int status;

            // Wait for this child to finish
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                return -1;
            }

            // Check how the child ended
            if (WIFEXITED(status)) {
                int exitCode = WEXITSTATUS(status);
                if (exitCode == 0) {
                    cout << "City " << (index + 1) 
                         << ": completed successfully." << endl;
                } else {
                    cout << "City " << (index + 1)
                         << ": child exited with code " << exitCode << endl;
                }
            } else if (WIFSIGNALED(status)) {
                cout << "City " << (index + 1)
                     << ": child was terminated by signal "
                     << WTERMSIG(status) << endl;
            }
        }
    }

    cout << "\nAll cities processed. Weather data downloaded successfully." << endl;
    return 0;
}