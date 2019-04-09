#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFSIZE		4096

char *sf = "/tmp/listen";

ssize_t sendall(int fd, void *buf, size_t r, struct sockaddr_un *addr)
{
  ssize_t w, t = 0;

  do {
    w = sendto(fd, buf+t, r-t, 0, (struct sockaddr *)addr, sizeof(struct sockaddr_un));
    if (w < 0) return w;
    t += w;
  } while (t < r);

  return t;
}

/**
 * This program should create a socket, then:
 * - Read lines of input from the user and send them to the listen program (via
 *   sendto(2)) using the socket file path, which is by default /tmp/listen, but
 *   may be modified by a path given on the command line.
 * - Print the message "Type Ctrl-D to quit:\n" before accepting input.
 * - The program should stop when the end of file on stdin is reached.
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

  printf("Type Ctrl-D to quit:\n");
  while ((r = read(STDIN_FILENO, buf, BUFSIZE)) > 0) {
    if (sendall(sock, buf, r, &addr) < 0) {
      perror("sendall");
      break;
    }
  }
  return 0;
}
