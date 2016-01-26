/*------------------------------
* server.c
* Description: HTTP server program
* CSC 361
* Instructor: Kui Wu
-------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <signal.h>
#include "util.h"

#define MAX_STR_LEN 120         /* maximum string length */

void cleanExit();
void perform_http(int newsockfd, char *dir);
void file_process(char *dir, char *file, char *reply);

/*---------------------main() routine--------------------------*
 * tasks for main
 * generate socket and get socket id,
 * max number of connection is 3 (maximum length the queue of pending connections may grow to)
 * Accept request from client and generate new socket
 * Communicate with client and close new socket after done
 *---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char dir[MAX_STR_LEN];

    if (argc < 3) {
        perror("Incorrect Arguments");
        exit(1);
    }

    portno = atoi(argv[1]);
    strncpy(dir, argv[2], MAX_STR_LEN-1);

    signal(SIGINT, cleanExit);

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
       perror("ERROR opening socket");
       exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR on binding");
       exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd,3);
    clilen = sizeof(cli_addr);

    while (1)
    {
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *) &clilen);

        if (newsockfd < 0) {
           perror("ERROR on accept");
           exit(1);
        }

        perform_http(newsockfd, dir);
        close(newsockfd);
    }
    // exit(0);
}

/*---------------------------------------------------------------------------*
 *
 * cleans up opened sockets when killed by a signal.
 *
 *---------------------------------------------------------------------------*/

void cleanExit()
{
    exit(0);
}

/*---------------------------------------------------------------------------*
 *
 * Accepts a request from "sockid" and sends a response to "sockid".
 *
 *---------------------------------------------------------------------------*/

void perform_http(int newsockfd, char *dir)
{
    /* If connection is established then start communicating */
    char buffer[MAX_STR_LEN];
    char reply[MAX_STR_LEN];
    char file[MAX_STR_LEN];

    bzero(buffer,MAX_STR_LEN);
    int n = read(newsockfd,buffer, MAX_STR_LEN-1);

    if (n < 0) {
       perror("ERROR reading from socket");
       exit(1);
    }

    printf("Request: %s\n",buffer);

    char *tmp = strtok(buffer, " ");

    if (strncmp(tmp, "GET", 4) == 0)
    {
        tmp = strtok(NULL, " ");
        strncpy(file, tmp, MAX_STR_LEN);
        file_process(dir, file, reply);
    }
    else
    {
        strncpy(reply, "HTTP/1.0 501 Not Implemented.\r\n\r\n", 37);
    }

    /* Write a response to the client */
    n = writen(newsockfd, reply, strlen(reply));

    if (n < 0) {
       perror("ERROR writing to socket");
       exit(1);
    }
}

void file_process(char *dir, char *file, char *reply)
{
    FILE *fp;
    char buff[256];

    strncat(dir, file, MAX_STR_LEN-1);
    printf("%s\n", dir);
    fp = fopen(dir, "r");

    if (fp == NULL)
    {
        strncpy(reply, "HTTP/1.0 404 Not Found.\r\n\r\n", 27);
        return;
    }

    //TODO: Finish this
    fgets(buff, 256, fp);
    printf("1 : %s\n", buff);

    printf("processing files...\n");
    fclose(fp);
}
