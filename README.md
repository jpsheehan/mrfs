# The Meeting Room Filesystem (MRFS)

TODO: Flesh this out...

## Video

TODO

## Configuring

Edit the `src/.env.h` file and place your UC username and password in the relevant macros.
This is your username without the email address on it.

## Building

```shell
nix-shell -p gnumake gcc pkg-config fuse curl
make mrfs
```

## Running

To mount the folder `foo` at the room booking in room 215 at 6:30pm on the 21st of December 1902:

```shell
mkdir foo
./mrfs ./foo 1902 12 21 18 30 215
```

