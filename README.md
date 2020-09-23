# OperatingSystem_ProcessManager

Assignment Requirements and Introduction
When a UNIX-like system starts up, it runs init. Nowadays this is a program called systemd on UNIX-like systems. On Mac the similar system manager is called launchd. It runs under PID of 1 and is an ancestor of all other processes. You can see the process with command "ps aux". If a process is left as an orphan (its parent dies), it gets reassigned as a child of PID 1. These service programs (init, systemd or launchd) are running in the background; on UNIX-like OSs these are commonly referred to as daemons and generally have names ending with the letter “d.” These programs ensure that things start up properly and stay running.  If a process crashes systemd (or launchd) can detect this and start it back up.

You will develop a proc_manager program that reads in the commands to execute from an input file one-by-one. You will read one command and its parameters per each line. Then the proc_manager program will start up the process and "babysit" it.

For this assignment you may use ideas from your C implementation from worksheet #9 (zyBooks 9.5) for this purpose, which was reading strings from stdin and parsing them. You may also use ideas from sigusr.c where we handled signals kill, usr1 and usr2.

You can also use some of your code ideas from avg_many in assignment #2 to create a tool that babysits several running processes. The exec system call (or one of its variants) is going to be at the heart of your code.

Some template code for searching a linked list storing commands (with associated data) is provided under folder code/a3.

You will run each command in a child process using exec (or one of its variants) and the parent process will wait for each exec to complete. For each child process there will be log messages written to output and error files, which are named after its index in the sequence of commands read in. For example, process #1 will have logs written to 1.out and 1.err. Upon start, the string "Starting command INDEX: child PID pid of parent PPID" will be logged to the corresponding output file 1.out. You can retrieve PID and PPID through the return value of fork() or getpid() or getppid().

You can use dup2() to make file handle 1 (stdout) go to a file X.out and handle 2 (stderr) go to a file X.err. Note: new executions of a command should append to the previous output and error files, such as 1.out, rather than overwrite them.

Timer for each process: you can include the timer.h library and use timer to record the start time of spawning each child process in the parent. Upon finish of an exec (either a successful finish or termination via a signal), the process should record the finish runtime, and it should write to the output file the string "Finished at FIN, runtime duration DUR" where FIN is the finish time and DUR is the duration of the execution.

Each time a process finishes, the message "Exited with exitcode = X" should be written to the error file of the process. X is the process exit code. If the process was killed with a signal, then "Killed with signal S" should be written to the error file. S is the signal number that killed the process. This information is gathered using the status parameter of the wait() system call. If the program cannot be started (exec fails), use perror("name of command") to get the error message and command name and write them to the command's error file. Processes that encounter an invalid command (exec fails) should have an exit code of 2. Remember the exit code of 0 indicates success. Exit codes other than 0 indicate some failure, including a termination via a kill signal.
