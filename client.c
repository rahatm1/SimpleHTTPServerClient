/*------------------------------
* client.c
* Description: HTTP client program
* CSC 361
* Instructor: Kui Wu
* Developer: Rahat Mahbub
* Student ID: V00790465
-------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "util.h"

#ifndef h_addr
#define h_addr h_addr_list[0] /* for backward compatibility */
#endif

/* maximal string and reply length */
#define MAX_STR_LEN 120
#define MAX_RES_LEN 4096

/* Prototypes */
void parse_URI(char *uri, char *hostname, int *port, char *identifier);
int open_connection(char *hostname, int port);
void perform_http(int sockid, char *hostname, char *identifier);

/* --------- Main() routine ------------
 * three main task will be excuted:
 * accept the input URI and parse it into fragments for further operation
 * open socket connection with specified sockid ID
 * use the socket id to connect specified server
 * don't forget to handle errors
 */

int main(int argc, char *argv[])
{
    char uri[MAX_STR_LEN];
    char hostname[MAX_STR_LEN];
    char identifier[MAX_STR_LEN];
    int port = 80;
    int sockid;

    if (argc >= 2)
    {
        strncpy(uri, argv[1], MAX_STR_LEN-1);
    }
    else
    {
        printf("Open URI:  ");
        scanf("%s", uri);
    }

    parse_URI(uri, hostname, &port, identifier);
    sockid = open_connection(hostname, port);
    perform_http(sockid, hostname, identifier);
    exit(EXIT_SUCCESS);
}

/*------ Parse an "uri" into "hostname" and resource "identifier" --------*/

void parse_URI(char *uri, char *hostname, int *port, char *identifier)
{
    char *tmp = strtok(uri, ":/");

    /* Ignore http tag */
    if (strncmp(tmp, "http", 5) == 0)
    {
        tmp = strtok(NULL, ":/");
    }

    strncpy(hostname, tmp, MAX_STR_LEN-1);

    tmp = strtok(NULL, ":/");

    /* Get Port, if exists */
    if ( (tmp != NULL) && (atoi(tmp) > 0))
    {
        *port = atoi(tmp);
        tmp = strtok(NULL, " ");
    }

    /* If resource identifier doesn't exist, add index.html as default */
    if (tmp == NULL)
    {
        strncpy(identifier, "index.html", 11);
    }
    else
    {
        strncpy(identifier, tmp, MAX_STR_LEN-1);
        tmp = strtok(NULL, ":/");

        while (tmp!=NULL) {
            strncat(identifier, "/", MAX_STR_LEN-strlen(identifier)-1);
            strncat(identifier, tmp, MAX_STR_LEN-strlen(identifier)-1);
            tmp = strtok(NULL, ":/");
        }

    }
}

/*------------------------------------*
* connect to a HTTP server using hostname and port,
* and get the resource specified by identifier
*--------------------------------------*/
void perform_http(int sockid, char *hostname, char *identifier)
{
    char get_request[MAX_STR_LEN];
    char *response = malloc(MAX_RES_LEN);

    if (response == NULL) {
        perror("Not Enough memory");
        exit(1);
    }

    memset(get_request, 0, MAX_STR_LEN);
    memset(response, 0, MAX_STR_LEN);

    /* Create a GET request */
    strncpy(get_request, "GET /", MAX_STR_LEN-1);
    strncat(get_request, identifier, MAX_STR_LEN-strlen(get_request) -1);
    strncat(get_request, " HTTP/1.0\r\n\r\n", MAX_STR_LEN-strlen(get_request) -1);

    printf("---Request begin---\n");
    printf("GET /%s HTTP/1.0\n", identifier);
    printf("Host: %s\n\n", hostname);

    /* Send GET Request to the server */
    writen(sockid, get_request, sizeof(get_request));

    printf("---Request end---\n");
    printf("HTTP request sent, awaiting response...\n\n");

    /* Receive and print response */
    printf("---Response Header---\n");
    readn(sockid, response, MAX_RES_LEN);
    char *tmp = strstr(response, "\r\n\r\n");
    printf("%.*s\n\n---Response Body--- %s\n", (int) (strlen(response)-strlen(tmp)), response, tmp);

    while (readn(sockid, response, MAX_RES_LEN) > 0)
    {
        printf("%s\n", response);
    }

    free(response);
    close(sockid);
}

/*---------------------------------------------------------------------------*
 *
 * open_conn() routine. It connects to a remote server on a specified port.
 *
 *---------------------------------------------------------------------------*/

int open_connection(char *hostname, int port)
{

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server_ent;
    /* generate socket
    * connect socket to the host address
    */
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == -1)
    {
        fprintf(stderr, "Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize server_addr structure */
    memset(&server_addr, 0, sizeof(server_addr));

    server_ent = gethostbyname(hostname);
    memcpy(&server_addr.sin_addr, server_ent->h_addr, server_ent->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "Socket Connection Failed\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}
