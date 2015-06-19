# Introducing Nuxi CloudABI
Nuxi CloudABI is an
[application binary interface](http://en.wikipedia.org/wiki/Application_binary_interface)
for UNIX-like operating systems built around the concept of
[capability-based security](http://en.wikipedia.org/wiki/Capability-based_security).
It offers an environment in which processes can only interact with
locally created resources (pipes, socket pairs, shared memory,
subprocesses and threads), or global resources (files, sockets) when the
process possesses a token granting the process access to the resource (a
UNIX file descriptor). In a nutshell, it means that you can run
processes directly on top of a UNIX kernel while keeping complete
control over the actions the process is allowed to perform.

For example, consider the case where one wants to run a web service. In
CloudABI this can be modeled by starting a web server process with the
following file descriptors:

* A listening network socket used to receive incoming HTTP requests,
* One or more file descriptors to directories containing resources
  accessible through the web,
* One or more network sockets connected to backend services used by the
  web server (e.g., database servers),
* A file descriptor pointing to a log file.

During its lifetime this web server will never be capable of creating
new tokens out of thin air. It may accept incoming network connections
on the socket provided on startup, but not create an additional
listening socket. It may open files stored within the directories
provided on startup, but not access arbitrary paths on the system. This
makes CloudABI a lot more secure than the runtime offered by most other
UNIX-like systems. A security flaw in this web server will only allow an
attacker to compromise the process; not the entire system.

As CloudABI is mostly legacy-free and only needs to implement the
features that make sense in an environment that uses capability-based
security, it is very small in comparison to other UNIX ABIs. At the time
of writing, [CloudABI only has 58 system calls](src/common/syscalllist.h).
The number of types, datastructures and constants shared between
kernelspace and userspace is very low when compared to existing
operating systems. This makes it easy to add support for CloudABI
executables to existing operating systems. The idea being that software
only needs to be compiled once and can be run everywhere.

Nuxi CloudABI aims to become an ideal runtime environment for
cluster/cloud computing. As processes become more isolated and less
dependent on the host environment, the need for traditional system
administration and configuration management will decrease. A cluster can
be treated as a single abstract computation service, running processes
that are not tied to individual logical operating system instances.

The design of CloudABI is based on
[the Capsicum lightweight OS capability and sandbox framework](http://www.cl.cam.ac.uk/research/security/capsicum/)
developed at the
[University of Cambridge Computer Laboratory](http://www.cl.cam.ac.uk/).
Nuxi would like to thank its authors for their work.

# cloudlibc: Standard C library for CloudABI

cloudlibc is a standard C library built on top of CloudABI. It
implements almost all features of the C11 standard, but also a large
part of POSIX.1-2008. There are interfaces, however, that have been
omitted for one of the following reasons:

* **Safety first.** <br/>
  cloudlibc aims to make it easier for people to design and implement
  robust services. C applications may easily be prone to buffer
  overflows. Functions whose only purpose is to cause such bugs (e.g.,
  `gets()`, `strcpy()`) are not implemented. The same holds for
  functions that are inherently thread unsafe (`strtok()`), degrade
  security (`srand()`), introduce unneeded global state (`setlocale()`)
  or offer bad abstraction (signal handlers).

* **Pure capability-based security.** <br/>
  As access to global namespaces is prohibited, functions such as
  `open()`, `fopen()`, `stat()`, `mkdir()` and `wait()` have been
  omitted. For filesystem access it is possible to use the POSIX.1-2008
  `*at()` functions instead. In some other cases alternatives have been
  developed (e.g., `pdfork()` and `pdwait()`). Fear not: most of these
  interfaces are either available on systems such as FreeBSD or have
  been designed in such a way that they can easily be added to existing
  operating systems.

* **Computing in the cloud.** <br/>
  Though it is perfectly fine to run CloudABI processes from a
  command-line on your own workstation, the primary use case remains
  running networked services in the cloud or on a cluster. Certain
  concepts (e.g., UNIX user credentials, TTY handling, accounting) make
  little sense in such an environment.

The source tree is structured as follows:

* [src/include/](src/include): Public C/POSIX header files.
* [src/libc/](src/libc): C library source files.
* [src/common/](src/common): Internally used data structures, subroutines, etc.
* [src/crt/](src/crt): C program startup code.
* [contrib/](contrib): Contributed source code.

## Building and installing cloudlibc

After
[building and installing Clang and GNU Binutils](https://github.com/NuxiNL/cloudlibc/wiki/Building-the-toolchain),
cloudlibc may be built and installed by running the following command:

    ./build
    sudo ./install

Once the build and installation has completed successfully, it should be
possible to
[install additional runtime libraries](https://github.com/NuxiNL/cloudlibc/wiki/Building-runtime-libraries),
such as LLVM's compiler-rt, libc++ and libc++abi. These libraries are
required for full C and C++ language support.

## Operating system support

### FreeBSD

A copy of FreeBSD that supports CloudABI executables
[can be found on GitHub](https://github.com/NuxiNL/freebsd). It is
sufficient to compile a kernel that has the `COMPAT_CLOUDABI64` build
option enabled, or simply run `kldload cloudabi64` to load in the
CloudABI kernel module.

### NetBSD

A copy of NetBSD that supports CloudABI executables
[can also be found on GitHub](https://github.com/NuxiNL/netbsd). As on
FreeBSD, the kernel compilation option to enable CloudABI support is
called `COMPAT_CLOUDABI64`.

## Support

### Public support

The
[cloudabi-devel@googlegroups.com](mailto:cloudabi-devel@googlegroups.com)
mailing list is used to discuss the development of CloudABI. It is
possible to sign up for this mailing list and browse its archive on
[Google Groups](https://groups.google.com/forum/#!forum/cloudabi-devel).

An IRC channel called `#cloudabi` exists on EFnet.

Bugs and pull requests can be submitted through
[GitHub](https://github.com/NuxiNL/cloudlibc).

### Commercial support

CloudABI has been developed by [Nuxi](https://nuxi.nl/), an IT company
based in the lovely city of
['s-Hertogenbosch, the Netherlands](http://en.wikipedia.org/wiki/%27s-Hertogenbosch).
Are you interested in receiving commercial support on CloudABI or any of
its components? Be sure to get in touch with us at info@nuxi.nl.
