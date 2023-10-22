# The Meeting Room Filesystem (MRFS)

This is a project that I worked on during the final year of my Computer Engineering degree for COSC475 (Independent Course of Study; Advanced Operating Systems) at the [University of Canterbury](https://www.canterbury.ac.nz/).
It is a [Fuse filesystem](https://www.kernel.org/doc/html/next/filesystems/fuse.html) that uses the university's [library room booking system](https://library.canterbury.ac.nz/webapps/mrbs/week.php) to store its data.

The contents of each file is private however a sizeable amount of metadata is leaked due to the booking titles remaining viewable by all (even those without credentials).

## Presentation and Demonstration

_(Click the image to watch on YouTube)_

[![Watch the video](https://img.youtube.com/vi/-xAY_4wRgxg/hqdefault.jpg)](https://www.youtube.com/embed/-xAY_4wRgxg)

## Configuring

Edit the `src/.env.h` file and place your UC username and password in the relevant macros.
This is your username without the email address on it.
It's a bit ugly, but it works fine for now.

## Building

You need the following dependencies:

- Make
- GCC
- Pkg Config
- Fuse
- Curl

If you're using Nix you can use the following command to spin up a shell with the dependencies installed:

```shell
nix-shell -p gnumake gcc pkg-config fuse curl
```

Then, to build:

```shell
make mrfs
```

## Running

To mount the folder `foo` at the room booking in room 215 at 6:30pm on the 21st of December 1902:

```shell
mkdir foo
./mrfs ./foo 1902 12 21 18 30 215
```

You can learn the different room codes by hovering the mouse over the `+` (create new booking) button in the room booking UI and looking at the `room` query parameter.

## Unmounting

You can either press `Ctrl+C` to kill the program or run:

```shell
fusermount -u ./foo
```

## History

- 2023 October: Updated to use (the excellent!) [libcurl](https://curl.se/libcurl/) as the HTTP library since the university has begun to use HTTPS instead of HTTP.
- 2020 October: First release