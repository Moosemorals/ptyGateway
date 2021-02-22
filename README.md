SPDX-License-Identifier: MIT

A tool to provide a process with a pseudo TTY.

It needs to:

Create pty
  *  [posix_openpt]
  *  [grantpt]
  *  [unlockpt]

Fork

In child
  * Open slave pty
  * [dup] to setup stdin/stdout/stderr
       * MVP - stderr muxed with stdout
       * v2 - stderr over pipe?
  * [exec] target process

In parent
  * start [poll] loop
      * Read from stdin -> write to pty master
      * Read from pty master -> write to stdout

```
#define _GNU_SOURCE
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int open_pty(int *aleader, int *aminion) {
  int leader, minion;
  char *name;
  
  // Get leader side of pseudo terminal (pty)
  leader = getpt();
  if (leader < 0) {
    return 0;
  }
  
  // Set permissions and unlock pty
  if (grantpt(leader) < 0 || unlockpt(leader) < 0) {
    goto close_leader;
  }
  
  // Get the name of the minion
  name = ptsname(leader);
  if (name == NULL) {
    goto close_leader;
  }
  
  minion = open(name, O_RDWR);
  if (minion == -1) {
    goto close_leader;
  }
  
  *aleader = leader;
  *aminion = minion;
  return 1;
  
close_leader:
  close(leader);
  return 0;  
}


int main() {

  int leader, minion;
  if (!open_pty(&leader, &minion)) {
    perror("Can't open pesudo TTY");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child
    if (dup2(minion, STDIN_FILENO) == -1) {
      perror("Can't dup2 stdin to minion");
      exit(EXIT_FAILURE);
    }
    if (dup2(minion, STOUT_FILENO) == -1) {
      perror("Can't dup2 stdout to minion");
      exit(EXIT_FAILURE);
    }
    if (dup2(minion, STDERR_FILENO) == -1) {
      perror("Can't dup2 stderr to minion");
      exit(EXIT_FAILURE);
    }
    
    const char *const argv[] = {"bash", NULL};
    const char *const envp[] = { NULL };
    
    execve("/bin/bash", argv, envp);
    perror("Can't run bash");
    exit(EXIT_FAILURE);
           
  } else if (pid > 0) {
    // parent
    
    struct pollfd *pfds = calloc(2, sizeof(struct pollfd));
    if (pfds == NULL) {
      perror("Can't allocate memory for poll");
      exit(EXIT_FAILURE);    
    }
    
    pfds[0].fd = leader;
    pfds[0].events = POLLOUT;
    
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    
    while (1) {
      int ready;
      ready = poll(pfds, 1, -1);
      if (ready == -1) {
        perror("Poll failed");
        exit(EXIT_FAILURE);
      }
      
      if (pfds[0].revents & POLLOUT) {
        
      
      }
           
    }        
  } else {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }


}

```

[dup]: https://man7.org/linux/man-pages/man2/dup.2.html
[exec]: https://man7.org/linux/man-pages/man3/exec.3.html
[grantpt]: https://man7.org/linux/man-pages/man3/grantpt.3.html
[posix_openpt]: https://man7.org/linux/man-pages/man3/posix_openpt.3.html
[poll]: https://man7.org/linux/man-pages/man2/poll.2.html
[unlockpt]: https://man7.org/linux/man-pages/man3/grantpt.3.html
