/*------------------------------
* server.c
* Description: HTTP server program
* CSC 361
* Instructor: Kui Wu
* Developer: Rahat Mahbub
* Student ID: V00790465
-------------------------------*/
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#define _POSIX_C_SOURCE 200809L
#endif

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

/* Prototypes */
void cleanExit();
void perform_http(int newsockfd, char *dir);
void file_process(int newsockfd, char *dir, char *file);
void sendResponse(int newsockfd, char *msg);

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

    /* Tries to clean up, if Ctrl+C is pressed */
    signal(SIGINT, cleanExit);

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
       perror("ERROR opening socket");
       exit(1);
    }

    /* Initialize socket structure */
    memset(&serv_addr, 0, sizeof(serv_addr));

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
    char buffer[MAX_STR_LEN];
    memset(buffer, 0, MAX_STR_LEN);

    /* Read request from socket */
    int n = read(newsockfd,buffer, MAX_STR_LEN-1);

    if (n < 0) {
       perror("ERROR reading from socket");
       exit(1);
    }

    char *tmp = strtok(buffer, " ");

    /* Reply right away if the request is not GET */
    if (strncmp(tmp, "GET", 4) == 0)
    {
        tmp = strtok(NULL, " ");
        file_process(newsockfd, dir, tmp);
    }
    else
    {
        sendResponse(newsockfd, "HTTP/1.0 501 Not Implemented.\r\n\r\n");
    }
}

/*---------------------------------------------------------------------------*
 *  Send Given response to the client
 *---------------------------------------------------------------------------*/
void sendResponse(int newsockfd, char *msg)
{
    int n = writen(newsockfd, msg, strlen(msg));

    if (n < 0) {
       perror("ERROR writing to socket");
       exit(1);
    }
}

/*---------------------------------------------------------------------------*
 * If requested resource exists, process and send it.
 * Otherwise, send 404 Not Found and exit
 *---------------------------------------------------------------------------*/
void file_process(int newsockfd, char *dir, char *file)
{
    FILE *fp;
    char *buff = NULL;
    char path[MAX_STR_LEN];
    size_t len = 0;

    /* Add resource identifier and directory being served
     * to get path of requested file*/
    strncpy(path, dir, MAX_STR_LEN-1);
    strncat(path, file, MAX_STR_LEN-1);

    fp = fopen(path, "r");

    if (fp == NULL)
    {
        sendResponse(newsockfd, "HTTP/1.0 404 Not Found.\r\n\r\n");
        return;
    }

    sendResponse(newsockfd, "HTTP/1.0 200 OK\r\n\r\n");

    /* Send Resource, line by line */
    while ((getline(&buff, &len, fp)) != EOF) {
        sendResponse(newsockfd, buff);
    }

    free(buff);
    fclose(fp);
}
