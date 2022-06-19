#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define CHAIMON_ERROR "ERROR_FROM_EX4\n"

int fd_to_srv = 0;
int got_response = 0;
pid_t client_pid = 0;
char client_pid_string[50];
char response_pid_file_name[50];



void alarmHandler(int signum) {
    if (!got_response) {
        printf("%s", "Client closed because no response was received");
        close(fd_to_srv);

    }
    char fileName[100] = "to_client_";
    strcat(fileName, client_pid_string);
    strcat(fileName, ".txt");
    if (access(fileName, F_OK) == 0) { /*If it exsist*/
        remove(fileName);
    }
    exit(-1);
}

void responseHandler(int signum) {
    alarm(0);

    char fileName[100] = "to_client_";
    strcat(fileName, client_pid_string);
    strcat(fileName, ".txt");
    int responseFD = open(fileName, O_RDONLY);
    if (responseFD == -1) {
        printf("ERROR: There is no such pid\n");
        close(fd_to_srv);
        exit(-1);
    }
    char current = ' ';
    char answer[100];
    int i = 0;

    while (current != '\n') {
        read(responseFD, &current, 1);
        answer[i] = current;
        i++;
    }
    if(!isdigit(answer[i])){
        answer[i] = '\0';
    }
    printf("%s", answer);
    close(responseFD);
    remove(fileName);
    exit(0);

}


int main(int argc, char **argv) {
    /***********Validate Input*****/
    if (argc != 5) {
        printf("%s", CHAIMON_ERROR);
        exit(-1);
    }

    //printf("Hello, World!\n");
    char *serverPid = argv[1];
    char *firstParam = argv[2];
    char *operator = argv[3];
    char *secondParam = argv[4];

    /*************************************/
    int temp;
    sscanf(serverPid, "%d", &temp); /*Casting from string to int*/
    pid_t srv_pid = temp; /*Server pid*/
    client_pid = getpid(); /*Client pid*/


    /*******CHECK IF to_srv EXSIST********/
    int i;
    int success_flag = 0;
    srand(time(0));
    int wait_time;
    for (i = 0; i < 10; ++i) {
        if ((fd_to_srv = open("to_srv.txt", O_RDWR | O_CREAT | O_APPEND | O_EXCL, 0666)) == -1) { /*If it exsist*/
            sleep(((rand() % 5) + 1));
        } else { /*If it not exsist*/
            success_flag = 1;
            break;
        }
    }
    if (success_flag == 0) {
        //write(1,"ERROR: server is busy\n", 23);
        printf("Couldn't open file");
        close(fd_to_srv);
        exit(1);
    }
    /****OPEN to_srv.txt & WRITE TO IT****/
    sprintf(client_pid_string, "%d", client_pid);
    int pid, status;
    //dup2(fd_to_srv,1);
    //close(fd_to_srv);
    write(fd_to_srv, client_pid_string, strlen(client_pid_string));
    write(fd_to_srv, "\n", 1);
    write(fd_to_srv, firstParam, strlen(firstParam));
    write(fd_to_srv, "\n", 1);
    write(fd_to_srv, operator, strlen(operator));
    write(fd_to_srv, "\n", 1);
    write(fd_to_srv, secondParam, strlen(secondParam));

    close(fd_to_srv);
    signal(SIGALRM, alarmHandler);
    signal(SIGUSR1, responseHandler);
    if (kill(srv_pid, SIGUSR1) == -1) { /*Send a signal to to server we are done*/
        printf("ERROR: There is no such pid\n");
        close(fd_to_srv);
        exit(1);
    }
    alarm(30);

    while (1) {
        pause(); /*Waiting to answer from the server*/
    }


    return 0;
}
