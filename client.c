/*------------------------------
* client.c
* Description: HTTP client program
* CSC 361
* Instructor: Kui Wu
-------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "util.h"

/* define maximal string and reply length, this is just an example.*/
/* MAX_RES_LEN should be defined larger (e.g. 4096) in real testing. */
#define MAX_STR_LEN 120
#define MAX_RES_LEN 4096

/* --------- Main() routine ------------
 * three main task will be excuted:
 * accept the input URI and parse it into fragments for further operation
 * open socket connection with specified sockid ID
 * use the socket id to connect specified server
 * don't forget to handle errors
 */

void parse_URI(char *uri, char *hostname, int *port, char *identifier);
int open_connection(char *hostname, int port);
void perform_http(int sockid, char *identifier);

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
    else {
        printf("Open URI:  ");
        scanf("%s", uri);
    }

    parse_URI(uri, hostname, &port, identifier);
    sockid = open_connection(hostname, port);
    perform_http(sockid, identifier);
    exit(EXIT_SUCCESS);
}

/*------ Parse an "uri" into "hostname" and resource "identifier" --------*/

void parse_URI(char *uri, char *hostname, int *port, char *identifier)
{
    char *tmp = strtok(uri, ":/");

    if (strncmp(tmp, "http", 5) == 0)
    {
        tmp = strtok(NULL, ":/");
    }

    strncpy(hostname, tmp, MAX_STR_LEN-1);

    tmp = strtok(NULL, ":/");

    if ( atoi(tmp) > 0 )
    {
        *port = atoi(tmp);
        tmp = strtok(NULL, ":/");
    }

    if (tmp == NULL)
    {
        strncpy(identifier, "index.html", 11);
    }
    else
    {
        strncpy(identifier, tmp, MAX_STR_LEN-1);
    }
}

/*------------------------------------*
* connect to a HTTP server using hostname and port,
* and get the resource specified by identifier
*--------------------------------------*/
void perform_http(int sockid, char *identifier)
{
    /* connect to server and retrieve response */

    char get_request[MAX_STR_LEN];
    strncpy(get_request, "GET /", MAX_STR_LEN-1);
    strncat(get_request, identifier, MAX_STR_LEN-strlen(get_request) -1);
    strncat(get_request, " HTTP/1.0\r\n\r\n", MAX_STR_LEN-strlen(get_request) -1);
    writen(sockid, get_request, sizeof(get_request));

    char response[MAX_RES_LEN];
    readn(sockid, response, MAX_RES_LEN);
    printf("%s\n", response);
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

    if (sockfd == -1) {
        fprintf(stderr, "Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_ent = gethostbyname(hostname);
    memcpy(&server_addr.sin_addr, server_ent->h_addr, server_ent->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Socket Connection Failed\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}
