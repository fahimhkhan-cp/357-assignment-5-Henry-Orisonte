#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

void sigchld_handler(int s) {
    (void)s;
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void send_error(FILE *network, char *status, char *title) {
    char body[256];

    snprintf(body, sizeof(body), "%s\r\n", title);
    
    fprintf(network, "HTTP/1.0 %s\r\n", status);
    fprintf(network, "Content-Type: text/html\r\n");
    fprintf(network, "Content-Length: %zu\r\n", strlen(body));
    fprintf(network, "\r\n");
    fprintf(network, "%s", body);
    fflush(network);
}


void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r+");
   char *line = NULL;
   size_t size;

   char method[16], path[256], version[16];

   getline(&line, &size, network);
   sscanf(line, "%s %s %s", method, path, version);

   while(getline(&line, &size, network) > 0)
   {
      if (strcmp(line, "\r\n") == 0)
         break;
   }

   free(line);

   char *filepath = path + 1;

   if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        send_error(network, "501 Not Implemented", "501 Not Implemented");
        fclose(network);
        return;
    }
   struct stat st;
   
   if (stat(filepath, &st) == -1) {
      send_error(network, "404 Not Found", "404 Not Found");
      fclose(network);
      return;
   }

   if(access(filepath, R_OK) == -1) {
      send_error(network, "403 Forbidden", "403 Forbidden");
      fclose(network);
      return;
   }

   fprintf(network, "HTTP/1.0 200 OK\r\n");
   fprintf(network, "Content-Type: text/html\r\n");
   fprintf(network, "Content-Length: %lld\r\n", st.st_size);
   fprintf(network, "\r\n");

   if(strcmp(method, "GET") == 0)
   {
    FILE *file = fopen(filepath, "r");
    if(file == NULL) {
        send_error(network, "500 Internal Server Error", "500 Internal Server Error");
        fclose(network);
        return;
    }
    char buffer[4096];
    size_t bytesRead;

    while((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        fwrite(buffer, 1, bytesRead, network);
    }   
    fclose(file);
   }
   fflush(network);
   fclose(network);
}

void run_service(int fd)
{
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd == -1)
         continue;
      
         printf("Connection established\n");

         pid_t pid = fork();
         if (pid == -1)
         {
            perror("fork");
            close(nfd);
            continue;
         }
         else if (pid == 0)
         {
            close(fd);
            handle_request(nfd);
            exit(0);
         }
         else
         {
            close(nfd);
            printf("Forked child process with PID: %d\n", pid);
         }
   }
}

int main (int argc, char *argv[]) {
    int portnumber;

    if (argc != 2){
        fprintf(stderr, "Usage: %s <portnumber>\n", argv[0]);
        exit(1);
    }

    
    portnumber = atoi(argv[1]);
    if (portnumber < 1024 || portnumber > 65535) {
        fprintf(stderr, "Error: Port number must be between 1024 and 65535.\n");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    int fd = create_service(portnumber);
    if (fd == -1) {
        perror("create_service");
        exit(1);
    }

    printf("Listening on port %d...\n", portnumber);
    run_service(fd);
    close(fd);
    return 0;
}
