## Summary

**pudc**: **P**ID **u** **d**iu **c**oses (which loosely translates to "PID 1
utters stuff").

Yet another blog site, but this time around cooked like this:

- The backend is written in a mixture of C and RISC-V assembly.
- The backend is actually PID1 (for real, not within a container).
- We poll and parse incoming HTTP requests ourselves.
- The frontend is a mere HTML page with [htmx](https://htmx.org/).

In the end, this repository is more of an exercise in order to dig deeper into
the Linux kernel, its interfaces, the RISC-V architecture, and all the dragons
in between.

## Build

*NOTE*: if you are running on a non-RISCV machine, then you will need to do
cross compilation. This is easily achieved by installing the proper package from
your Linux distribution (e.g. for openSUSE it's `cross-riscv64-<whatever>`) and
then setting the `CROSS_COMPILE` environment variable. This environment variable
is used pretty much like in the Linux Kernel and many other projects: it just
sets a prefix for `as`, `ld`, etc.

You can build everything with the default `make` target. After that, you can use
QEMU in order to test the given binary. For that you need QEMU with RISC-V
support (i.e. the `qemu-system-riscv64` command), and you need to pass the
`LINUX_SOURCE` option with the path to the Linux kernel source code. Another
requirement is the file system. This is supposed to be available and, if we are
going to use QEMU, then you need to pass an image with a file system in it. You
can generate this image like so:

```sh
$ qemu-img create -f raw disk.img 10G
$ sudo mkfs.{btrfs,ext4,whatever} -F disk.img
$ make qemu_prepare_disk
```

Note that the `qemu_prepare_disk` target will mount into `/mnt/pudc` this disk
and populate it with a minimal hierarchy and the executable produced by the
earlier `make` default target. If you want your image to be named in another
way, change the commands above and set the `MOUNT_DEST_IMAGE` environment
variable, as that will be used for the `qemu_prepare_disk` target.

Compiling the Linux Kernel is pretty easy. If you are cross compiling you must
make sure that the `ARCH` environment variable is set to `riscv` and that the
`CROSS_COMPILE` one is also set as the toolchain prefix (just like the
instructions for this project). With that, the default configuration from `make
defconfig` will already give you support for virtualized RISC-V environments and
all SoCs. Otherwise, just select with `menuconfig` whatever it is that you need.

With all of that, just run `make qemu`, which will start QEMU in `-nographic`
mode with the given Linux Kernel and the init program that was built from this
project. You can quit from QEMU in `-nographic` mode by pressing `Ctrl+A` and
then `X`.

Hence, a complete run down on how to build and run this project (by using QEMU):

``` sh
# Exporting variables.
$ export ARCH=riscv
$ export LINUX_SOURCE=<linux kernel location>
$ export CROSS_COMPILE=riscv64-suse-linux-  # on openSUSE. Change it for your case.

# Kernel
$ cd $LINUX_SOURCE
$ make defconfig
$ make -j $(nproc)
# -> your image is now at arch/riscv/boot/Image.

# pudc
$ cd <here>
## If you don't have an .img disk, then:
##   $ qemu-img create -f raw disk.img 10G
##   $ sudo mkfs.{btrfs,ext4,whatever} -F disk.img
$ make qemu
# -> this will build everything and launch QEMU in -nographic mode (output from the serial port).
```

Some other options from the given Makefile:

- Everything is set to be quiet. If you want to remove that, just set `V=1` when
  calling any make target.
- If you pass `DEBUG=1`, then further debug messages will be shown.
- The value being passed to GCC's `-march` is the one that matches my
  VisionFive2 board. If you want to tune this differently, simply set
  `ISA=<ISA-string>` on any make target.

## Architecture

Still in the works but something like:

- zsbl -> opensbi -> kernel -> init
- init:
  - poll and, for each incoming HTTP request (using io_uring):
    - clone with proper flags (with control for fork-bombs)
    - http parser
    - Database with either embedded sqlite or plain dirs/files.
    - reply back with htmx

### Alternatives

#### Rust

I actually considered building this with Rust, but since I figured I would be
doing a lot of interfacing with libraries from the Linux kernel, then I'd also
need to work on safe abstractions for all that C code. Since I didn't want to do
any of that for this project, I just sticked with C.

#### nolibc et. al.

I tried to work with `nolibc` as integrated from the Linux kernel, but then I
couldn't build the whole thing because of `-Werror` and the fact that `nolibc`
is riddled with a diverse set of warnings. Hence, maybe in the future I add a
way to support this, but for now it's good ol' `glibc`.

## License

Released under the [GPLv3+](http://www.gnu.org/licenses/gpl-3.0.txt), Copyright
(C) 2024-<i>Ω</i> Miquel Sabaté Solà.
