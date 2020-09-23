//define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>


// Declare a struct for storing .out and .err filenames
struct CommandFiles{
    char command[20];
    char outFile[20];
    char errFile[20];
} ;



int main(int argc, char *argv[]){
    if ( argc != 2 ){
        printf( "Input file error!\n");
    }
    
    FILE * fp;
    char commands[20][20];
    char commandsCopy[20][20];
    
    fp = fopen(argv[1], "r" );
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // Note for num_commands: 0 corresponds to line 1 of command
    int num_commands = 0;
    // Read and store each line of command in array commands[][]
    while (fgets(commands[num_commands], sizeof(commands[num_commands]), fp)) {
        commands[num_commands][strlen(commands[num_commands]) - 1] = '\0';
        num_commands++;
    }
    fclose(fp);
    
    // Copy commands[][] into another array commandsCopy[][] for parsing purpose
    for(int i=0; i<num_commands; i++){
        strcpy(commandsCopy[i], commands[i]);
    }
    
    
    // For testing whether all lines of commands successfully store in commands[][]
//     for(int i = 0; i < num_commands; i++){
//         printf("%s\n", commands[i]);
//     }
    
    
    // Initialize data structure for storing parsed commands
    char *partCommands[num_commands][20];
    for(int i=0; i<num_commands; i++){
        for(int j=0; j<20; j++){
            partCommands[i][j] = NULL;
        }
    }
    
    // Parse each command
    for(int i=0; i<num_commands; i++){
        int count=0;
        char * token = strtok(commandsCopy[i], " ");
        partCommands[i][count++] = token;
        
        while(token != NULL){
            token = strtok(NULL, " ");
            partCommands[i][count++] = token;            
        }
    }
    
    
    
    // For testing whether partCommands[][] contains correct parsed commands
//     for(int i=0; i<num_commands; i++){
//         for(int j=0; 1; j++){
//             if (!partCommands[i][j])
//                 break;
//             printf("%s \n", partCommands[i][j]);
//         }
//         printf("\n");
//     }
    
    


    // commandPid[][] is for storing pid and ppid of a command
    int commandPid[num_commands][2];
    // commandPoints is for storing the starting and finishing time points of each command
    long int commandPoints[num_commands][2];
    // status: for checking exitcode
    // pid1: children pid collected for each executed command, will be stored for later use
    // pid2: for comparing to pid1 in order to find the index of a command
    int status, pid1, pid2;
    struct timespec start, finish;
    double duration;
    // array of struct, for storing filenames for corresponding command
    struct CommandFiles cfiles[20];
    
    int cct=0;
    // Store filename to its corresponding command, used to be compared by command content for future
    for(int i=0; i<num_commands; i++){
        int find = 0;
        for(int j=0; j<cct; j++){
            if(strcmp(commands[i], cfiles[j].command) == 0){
                find = 1;
                break;                
            }
        }
        if (!find) {
            sprintf(cfiles[cct].command, "%s", commands[i]);
            sprintf(cfiles[cct].outFile, "%d.out", cct+1);
            sprintf(cfiles[cct].errFile, "%d.err", cct+1);
            cct ++;
        }
    }
    
    
    // for testing whether cfiles[] contains correct corresponding filenames
//     for(int i=0; i<cct; i++){
//         printf("%s\n", cfiles[i].command);
//         printf("%s\n", cfiles[i].outFile);
//         printf("%s\n", cfiles[i].errFile);
//     }
    
    
    
    
#if 1

    // checking whether num_commands is correct
    //printf("----------num_commands----------\n%d\n", num_commands);

    
    // for loop to process each line of command
    for(int i=0; i<num_commands; i++){
        // ofile: filename of .out file
        // efile: filename of .err file
        char ofile[20];
        char efile[20];
        
        // find corresponding filenams for command[i]
        for(int j=0; j<cct; j++){
            if(strcmp(cfiles[j].command, commands[i]) == 0){
                sprintf(ofile, "%s", cfiles[j].outFile);
                sprintf(efile, "%s", cfiles[j].errFile);
                break;
            } 
        }
        
        
        // Record start time
        clock_gettime(CLOCK_MONOTONIC, &start);
        commandPoints[i][0] = start.tv_sec;
        
        pid1 = fork();
        
        if(pid1<0){
            fprintf(stderr,"Error forking");
            exit(1);
        }
        else if(pid1==0){//In child process
            // execute command
            execvp(partCommands[i][0], partCommands[i]);
            exit(0);
        }
        else{// In parent
            //Record pid of child
            commandPid[i][0] = pid1;
            commandPid[i][1] = (int)getpid();
            printf("-----------------\n%s\n", ofile);
            //Write to .out file
            fp = fopen(ofile, "a+");
            char ss[100];
            sprintf(ss, "Starting command %d: child %d pid of parent %d\n", i+1, pid1, (int)getpid());
            fputs(ss,fp);
            fclose(fp);
        }
    }
    
    
    // In parent, parent will wait for each process to complete and collect their status
    while((pid2 = wait(&status)) >= 0){
        
        if(pid2 > 0){
            // get finished time
            clock_gettime(CLOCK_MONOTONIC, &finish);
            
            int i;
            for(i=0; i<num_commands; i++){
                // based on the returned pid2, search commandPid[][] to find corresponding command index
                if(commandPid[i][0]==pid2){
                    commandPoints[i][1] = finish.tv_sec;
                    break;
                }
            }
            
            char ofile[20];
            char efile[20];
            // find corresponding filenames and store them in ofile and efile
            for(int j=0; j<cct; j++){
                if(strcmp(cfiles[j].command, commands[i]) == 0){
                    sprintf(ofile, "%s", cfiles[j].outFile);
                    sprintf(efile, "%s", cfiles[j].errFile);
                    break;
                } 
            }
                    
            // Write finished information in .out file
            char sout[100];
            fp = fopen(ofile, "a+");
            duration = (commandPoints[i][1] - commandPoints[i][0]);
            sprintf(sout, "Finished at %ld, runtime duration %f (pid: %d)\n", commandPoints[i][1], duration,  commandPid[i][0]);
            fputs(sout,fp);
            fclose(fp);
            
            // Write to .err file about exit status information
            char serr[100];
            if(status == 0){
                fp = fopen(efile, "a+");
                sprintf(serr,"Exited with exitcode = %d\n", status);
                fputs(serr, fp);
                fclose(fp);
            }
            else if(status == 2){
                fp = fopen(efile, "a+");
                fputs(commands[i], fp);
                sprintf(serr,"%s: Exec fails\n", commands[i]);
                fputs(serr, fp);
                fclose(fp);
            }
            else{
                fp = fopen(efile, "a+");
                sprintf(serr,"Killed with signal %d\n",status);
                fputs(serr, fp);
                fclose(fp);
            }
            // write to err file about duration time information or re-execute
            if(duration <= 2.0){
                fp = fopen(efile, "a+");
                sprintf(serr,"spawing too fast\n");
                fputs(serr, fp);
                fclose(fp);
            }
            else{// when duration is more than 2s, continuing execute the command
                // Record start time
                clock_gettime(CLOCK_MONOTONIC, &start);
                commandPoints[i][0] = start.tv_sec;
                
                int pid3 = fork();
                if(pid3<0){
                    fprintf(stderr,"Error forking");
                    exit(1);
                }
                else if(pid3==0){//In child process
                    // execute command
                    execvp(partCommands[i][0], partCommands[i]);
                    exit(0);
                }
                else{// in parent
                    //Write to .out file
                    fp = fopen(ofile, "a+");
                    char ss[100];
                    sprintf(ss, "Starting command %d: child %d pid of parent %d\n", i+1, pid3, (int)getpid());
                    fputs(ss,fp);
                    commandPid[i][0] = pid3;
                    commandPid[i][1] = (int)getpid();
                    fclose(fp);
                }
                

            }
            
            
        }
    }

#endif
    
    
    
    
    
    
    
    
    
    

   
    exit(EXIT_SUCCESS);
    
}



