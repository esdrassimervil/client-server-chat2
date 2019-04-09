#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>

#define BUFSIZE		4096

char *sf = "/tmp/listen";

/**
 * This function should fork(), and execlp() the clear Unix command in the child
 * and wait for the child process to complete in the parent.
 */
void clear(void)
{
  pid_t pid = fork();
  if (pid == 0) {
    execlp("clear", "clear", NULL);
    perror("exec clear");
    exit(0);
  }
  wait(NULL);
}

/**
 * This should be a signal handler to clean up the socket file and exit when
 * at a minimum Ctl-C is pressed.
 */
void cleanup(int sig)
{
  if (access(sf, F_OK) == 0)
    unlink(sf);
  exit(0);
}

/**
 * Make a program that works just like listen.c, done in class, except that
 * it uses a Unix Domain socket to listen on. The default socket path should be
 * /tmp/listen, but may be altered by providing a path given by the command line.
 * After setting up the socket it should clear the screen and print:
 * Login to <hostname> and say at <socket_path>
 * 
 * It should then read messages from the socket continuously and print them
 * to the stdout.
 */
int main(int argc, char *argv[])
{
  struct sockaddr_un addr;
  char buf[BUFSIZE];
  int r;

  if (argc > 1) sf = argv[1];

  /* Create the socket, initially unconnected: */
  int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }

  /* Create the address (filesystem socket file.) */
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, sf);
  
  /* Remove a stale socket file if present: */
  unlink(sf);

  /* Bind the address to the socket: */
  if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
    perror("bind");
    exit(1);
  }

  clear();
  printf("Login to %s and say at %s\n", getenv("HOSTNAME"), sf);

  while ((r = read(sock, buf, BUFSIZE)) > 0) {
    write(STDOUT_FILENO, buf, r);
  }

  return 0;
}
