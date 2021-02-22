SPDX-License-Identifier: MIT

A tool to provide a process with a pseudo TTY.

It needs to:

Create pty
  *  [posix_openpt]
  *  [grantpt]
  *  [unlockpt]

Create pipe(s?) [see](https://stackoverflow.com/a/14171149/195833)

Fork

In child
  * Open slave pty
  * Setup stdin/stdout
  * (Still thinking about stderr)
  



[posix_openpt]: https://man7.org/linux/man-pages/man3/posix_openpt.3.html
[grantpt]: https://man7.org/linux/man-pages/man3/grantpt.3.html
[unockpt]: https://man7.org/linux/man-pages/man3/grantpt.3.html


