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


[dup]: https://man7.org/linux/man-pages/man2/dup.2.html
[exec]: https://man7.org/linux/man-pages/man3/exec.3.html
[grantpt]: https://man7.org/linux/man-pages/man3/grantpt.3.html
[posix_openpt]: https://man7.org/linux/man-pages/man3/posix_openpt.3.html
[poll]: https://man7.org/linux/man-pages/man2/poll.2.html
[unlockpt]: https://man7.org/linux/man-pages/man3/grantpt.3.html
