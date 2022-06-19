//
// Created by yonadav on 6/16/22.
//
#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <inttypes.h>
#include <iso646.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

int startsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}
void removeFiles() {
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    DIR* dp = opendir(pwd);
    if (dp == NULL) {
        perror("opendir: Path does not exist or could not be read.");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dp))) {
        if(startsWith(entry->d_name, "to_client"))
            remove(entry->d_name);
    }

    closedir(dp);
    return;
}

void alarmHandler(int signum) {
    printf("%s\n", "The server was closed because no service request was received for the last 60 seconds");
    alarm(0);
    while(wait(NULL) != -1);
    removeFiles();
    exit(-1);
}

void handleClient(int num) {
    signal(SIGUSR1, handleClient);
    alarm(0);

    int pid_compute, status;
    /**********INSIDE THE FORK**********/
    if (pid_compute = fork() == 0) { /*Creat a procces for the compute*/
        int fd_to_srv = open("to_srv.txt", O_RDONLY); /*Open fd for to_srv.txt file*/
        if (fd_to_srv == -1) {
            perror("open");
            exit(-1);
        }
        char to_srv_buf[100]; /*Buffer for the conent of to_srv.txt*/
        int to_srv_read = read(fd_to_srv, to_srv_buf, 100);/*Read the content of to_srv*/
        if (to_srv_read == -1) {
            perror("read");
            exit(1);
        }
        close(fd_to_srv);

        /*if(pid_compute=fork()==0){ *//*Creat grandson*//*
            execl("/bin/rm","rm","to_srv.txt",NULL); *//*Delete the file as soon as possible*//*
        }
        waitpid(pid_compute,&status,NULL);*/
        remove("to_srv.txt");
        /**********SPLIT THE BUFFER**********/
        char *client_pid_c = strtok(to_srv_buf, "\n");
        char *P2_c = strtok(NULL, "\n");
        char *P3_c = strtok(NULL, "\n");
        char *P4_c = strtok(NULL, "\n");
        /**********CASTING**********/
        intmax_t xmax;
        char *tmp;
        pid_t client_pid_i;
        errno = 0;
        xmax = strtoimax(client_pid_c, &tmp, 10);
        if (errno != 0 or tmp == client_pid_c or *tmp != '\0' or xmax != (pid_t) xmax) {
            fprintf(stderr, "Bad PID!\n");
        } else {
            client_pid_i = (pid_t) xmax;
        }
        int temp;
        sscanf(P2_c, "%d", &temp);
        int P2_i = temp;
        sscanf(P3_c, "%d", &temp);
        int P3_i = temp;
        sscanf(P4_c, "%d", &temp);
        int P4_i = temp;
        /**********CALCULATE**********/
        int result;
        if (P4_i == 0 && P3_i==4) {
            //printf("CANNOT_DIVIDE_BY_0\n");
            //exit(1);
            char str[100] = "to_client_";
            strcat(str, client_pid_c);
            strcat(str, ".txt");
            int fd_to_client = open(str, O_RDWR | O_CREAT | O_APPEND, 0666); /*Open fd for to_client_xxxx file*/
            if (fd_to_client == -1) {
                perror("open");
                exit(1);
            }
            dup2(fd_to_client, 1);
            printf("%s\n", "CANNOT_DIVIDE_BY_ZERO");
            //}
            //waitpid(pid_compute, &status, 0);
            close(fd_to_client);
            kill(client_pid_i, SIGUSR1); /*Send a signal to the client we are done*/
            exit(0);
        }
        if (P3_i == 1) {
            result = P2_i + P4_i;
        }
        if (P3_i == 2) {
            result = P2_i - P4_i;
        }
        if (P3_i == 3) {
            result = P2_i * P4_i;
        }
        if (P3_i == 4) {
            result = P2_i / P4_i;
        }
        /**********OPEN to_client_xxxx FILE**********/
        char str[100] = "to_client_";
        strcat(str, client_pid_c);
        strcat(str, ".txt");
        int fd_to_client = open(str, O_RDWR | O_CREAT | O_APPEND, 0666); /*Open fd for to_client_xxxx file*/
        if (fd_to_client == -1) {
            perror("open");
            exit(1);
        }
        //if (pid_compute = fork() == 0) { /*Creat a procces*/
        dup2(fd_to_client, 1);
        printf("%d\n", result);
        //}
        //waitpid(pid_compute, &status, 0);
        close(fd_to_client);
        kill(client_pid_i, SIGUSR1); /*Send a signal to the client we are done*/
        exit(0);
    }
        /***********************************/
    else {
        signal(SIGCHLD, SIG_IGN); /*Prevent zombies*/
    }


}

int main(int argc, char *argv[]) {

    /**********DELETE to_srv IF IT EXSIST**********/
    if (access("to_srv.txt", F_OK) == 0) { /*If it exsist*/
        remove("to_srv.txt");

    }
    signal(SIGALRM, alarmHandler);

    while (1) { /*Waiting to signals from clients*/
        signal(SIGUSR1, handleClient); /*Signal registration*/
        alarm(60);
        //printf("%d\n", getpid());
        pause();
    }
    exit(0);
}