#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_COMMAND_SIZE 12 // Based on the biggest command (len(unsubscribe) = 11)

/* Constants used for switch on commands */
#define REGISTER 1
#define UNREGISTER 2
#define LOGIN 3
#define LOGOUT 4
#define USER_EXIT 5
#define GROUPS_LIST 6
#define SUBSCRIBE 7
#define UNSUBSCRIBE 8
#define USER_GROUPS 9
#define SELECT 10
#define USERS_LIST 11
#define GROUP_POST 12
#define GROUP_RETRIEVE 13

/* Global variables */
int socketfd;

/* Auxiliary Functions declaration */
void validateInput(int argc, char * argv[]);
int parseUserCommand(char * command);

/* Main Body */
int main(int argc, char * argv[]) {
    char command[MAX_COMMAND_SIZE];
    int op;
    validateInput(argc, argv);
    while (scanf("%s", command) != -1) {
        op = parseUserCommand(command);
    }
    return EXIT_SUCCESS;
}

/* Auxiliary Functions implementation */
void validateInput(int argc, char * argv[]) {
    char c;
    if (argc != 1 && argc != 3 && argc != 5) {
        fprintf(stderr, "Invalid input. Please try again.\n");
        exit(EXIT_FAILURE);
    }
    switch(argc) {
        case 1:
            socketfd = socket(AF_UNIX, )

    }
    /* while ((c = getopt(argc, argv, ":n:p:")) != -1) {
        switch(c) {
            case 'n':
                if (strcmp(optarg, "-p") == 0) {
                    fprintf(stderr, "Missing argument. Please try again.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                break;
            case ':':
                fprintf(stderr, "Missing argument. Please try again.\n");
                exit(EXIT_FAILURE);
            case '?':
                printf("Unknown flag.\n");
                break;
            default:
                printf("Ayo.\n");
                break;
        }
    } */
}

int parseUserCommand(char * command) {
    if (strcmp(command, "reg") == 0) return REGISTER;
    else if (strcmp(command, "unregister") == 0) return UNREGISTER;
    else if (strcmp(command, "login") == 0) return LOGIN;
    else if (strcmp(command, "logout") == 0) return LOGOUT;
    else if (strcmp(command, "exit") == 0) return USER_EXIT;
    else if ((strcmp(command, "groups") == 0) || (strcmp(command, "gl") == 0)) return GROUPS_LIST;
    else if (strcmp(command, "subscribe") == 0) return SUBSCRIBE;
    else if (strcmp(command, "unsubscribe") == 0) return UNSUBSCRIBE;
    else if ((strcmp(command, "my_groups") == 0) || (strcmp(command, "mgl") == 0)) return USER_GROUPS;
    else if ((strcmp(command, "select") == 0) || (strcmp(command, "sag") == 0)) return SELECT;
    else if ((strcmp(command, "ulist") == 0) || (strcmp(command, "ul") == 0)) return USERS_LIST;
    else if (strcmp(command, "post") == 0) return GROUP_POST;
    else if (strcmp(command, "retrieve") == 0) return GROUP_RETRIEVE;
    else {
        fprintf(stderr, "Invalid user command. Please try again.\n");
        exit(EXIT_FAILURE);
    }
}