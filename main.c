#define _GNU_SOURCE
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
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

void copy(int fdIn, int fdOut) {
  char buf[1024 * 16];
  
  while (1) {
    ssize_t r = read(fdIn, buf, sizeof(buf));
    if (r == -1) {
      perror("read");
      exit(EXIT_FAILURE);
    } else if (r == 0) {
      return;
    }
    
    ssize_t w = write(fdOut, buf, r);
    if (w == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    } else if (w != r) {
      // TODO: Handle this better
      fprintf(stderr, "Wrote %d bytes, should have written %d\n", w, r);
      exit(EXIT_FAILURE);    
    }   
  }
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
    pfds[0].events = POLLIN;
    
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    
    while (1) {
      int ready;
      ready = poll(pfds, 1, -1);
      if (ready == -1) {
        perror("Poll failed");
        exit(EXIT_FAILURE);
      }
      
      if (pfds[0].revents != 0) {
        if (pfds[0].revents & POLLIN) {
          copy(pfds[0], STDOUT_FILENO);
        } else {
          fprintf(stderr, "pty closed unexpectedly\n");
          exit(EXIT_FAILURE);
        }      
      }
      if (pfds[1].revents != 0) {
        if (pfds[1].revents & POLLIN) {
          copy(pfds[1], leader);
        } else {
          fprintf(stderr, "pty closed unexpectedly\n");
          exit(EXIT_FAILURE);
        }      
      }
    }      
  } else {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }
}
