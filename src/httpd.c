#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>

#include <errno.h>

#include "mime.h"
#include "path.h"
#include "config.h"
#include "http_request.h"
#include "http_response.h"
#include "error_handler.h"

#include <signal.h>

void socket_loop ( );
int current_socket = 0;

/* Catch Signal Handler functio */
void signal_callback_handler (int signum) {
  fprintf(stderr, "Caught signal SIGPIPE %d\n", signum);
  close(current_socket);
}

char *config_root = ".", *sport = "8080";
uint16_t config_port = 8080;
uint16_t config_root_length;

void show_options (char *exe) {
  printf("USAGE: %s [-d dirname] [-p portnum]\n", exe);
  printf("  -d dirname    : directory to serve files from [default .]\n");
  printf("  -p port       : port to bind to [default 8080]\n");
}

int main (int argc, char **argv) {
  extern char *optarg;
  extern int optind;
  char c;

  while ((c = getopt(argc, argv, "d:p:h")) != -1) {
    switch (c) {
      case 'd':
        config_root = optarg;
        break;
      case 'p':
        sport = optarg;
        config_port = atoi(sport);
        break;
      case 'h':
        show_options(argv[0]);
        exit(0);
      }
  }

  if (strlen(config_root) >= MAX_ROOT_LENGTH) {
    fprintf(stderr, "ERROR: directory can only be %d characters long\n", MAX_ROOT_LENGTH);
    exit(1);
  }

  char *cret = realpath(config_root, NULL);

  config_root = cret;
  if (cret == NULL) {
    fprintf(stderr, "ERROR: unable to resolve directory\n");
    exit(1);
  }

  printf("webroot: %s, port: %d\n", cret, config_port);

  config_root_length = strlen(config_root);

  socket_loop();

  return 0;
}

void socket_loop ( ) {
  int sockfd, newsockfd, portno;
  unsigned int clilen;
  char buffer[MAX_READ_SIZE];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  signal(SIGPIPE, signal_callback_handler);

  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    exit(1);
  }

  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = config_port;

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
  listen(sockfd, MAX_READ_SIZE);
  clilen = sizeof(cli_addr);

  while (1) {
    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
      perror("ERROR on accept");
    }

    current_socket = newsockfd;

    /* If connection is established then start communicating */
    bzero(buffer, MAX_READ_SIZE);
    n = read(newsockfd, buffer, MAX_READ_SIZE);

    if (n < 0) {
      perror("ERROR reading from socket");
      close(newsockfd);
    }

    // parse the request, get the results back
    Request *request = parse_request((uint8_t *)buffer);
    Response *response = response_create(newsockfd);

    if (serve_file (request, response) == false) {
      serve_404(request, response);
    }

    close(newsockfd);
  }
}
