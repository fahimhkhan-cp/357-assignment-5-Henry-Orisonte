#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

void sigchld_handler(int s) {
    (void)s;
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_request(int nfd)
{
   FILE *network = fdopen(nfd, "r+");
   char *line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   while ((num = getline(&line, &size, network)) >= 0)
   {
      if(strcmp(line, "\r\n") == 0)
         break;
   }

   free(line);
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