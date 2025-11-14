==================================================
README: CPSC-351 Assignment 1 - Process Management
==================================================

Team members:
1. Siddharth Vasu: siddharth.vasu@csu.fullerton.edu, Contribution: shell.cpp
2. Gilbert Cervantes: GilbertC23@csu.fullerton.edu, Contrinution: serial.cpp, locations.txt
3. Omar Rafiq, email: omarrafiq@csu.fullerton.edu, Contribution: parallel.cpp

We confirm everyone is familiar with all functionality in the assignment.


Programming Language used: C++


1.  To Compile All Files:
    g++ shell.cpp -o shell
    g++ serial.cpp -o serial
    g++ parallel.cpp -o parallel

    Extra credit:
    g++ multi-search.cpp -o multi-search

2.  To Run the Shell:
    ./shell

3.  To Run and Time the Fetchers: (Ensure 'locations.txt' is in the same directory)
    time ./serial
    time ./parallel


parallel.cpp Execution time: User: 0.07s  System: 0.04s Real: 0.791 
serial.cpp Execution time:


Questions: 
1. In the output of time, what is the difference between real, user, and sys times?
A: The real time represents the total elapsed time from the moment the program starts until it finishes, as measured by a wall clock. The user time is the amount of CPU time the program spent executing its own code in user mode. The system time is the amount of CPU time the OS kernel spent on behalf of the program.

2. Which is longer, user time or sys time? Use your knowledge to explain why.
A: The user time is longer than the sys time. This is because the majority of the instructions do not require kernel level privileges to execute.

3.When fetching all of the locations in the file, which fetcher finishes faster? Why do you
think that is?

A: TODO

4. Suppose one of the fetcher children crashes. Will this affect other children? Explain.
What is your conclusion regarding the benefits or disadvantages of isolating each fetch
task within a different process

If one fetcher child crashes, it will not affect the other children or the parent process. This is because the child processes are entirely isolated; each one operates in its own separate memory space, which prevents a failure in one from corrupting the state or execution of others. The conclusion is that isolating each fetch task within a different process provides the significant benefit of Fault Tolerance



