# libfaketime

### Modifies the system time for a single application or container through Linux syscall

Reference Projects：[wolfcw/libfaketime](https://github.com/wolfcw/libfaketime) and [batiati/dateoffset](https://github.com/batiati/dateoffset)

## 1. What is it

_libfaketime_ provides a very simple way to run a single application or container with a different date, without changing the system clock.

It's useful for testing and debugging code that are time-dependant, like databases, certificates, and all sort of business logic that involves dates.

## 2. Motivation

If you are looking for a portable and full featured solution, you might opt for the excellent [libfaketime](https://github.com/wolfcw/libfaketime).

Unfortunately the way [libfaketime](https://github.com/wolfcw/libfaketime) works [clash with some applications](https://github.com/wolfcw/libfaketime/issues/130) using [jemalloc](http://jemalloc.net/) as memory allocator, which apperas to be the case of [Microsoft SQL Server for Linux](https://hub.docker.com/_/microsoft-mssql-server) and many other applications.

This _libfaketime_ solves it by eliminating memory allocations caused by `dlsym` and directly calls Linux `clock_gettime` syscall.

I strongly suggest you to try [libfaketime](https://github.com/wolfcw/libfaketime) first before using this one.

## 3. Usage

Just like [libfaketime](https://github.com/wolfcw/libfaketime), the basic way of running any command/program with this _libfaketime_ is getting it loaded by system's linker using the environment variable `LD_PRELOAD`.

There are multiple ways to specify the faked time:

1. By setting the environment variable TIME_OFFSET.
2. By using the file .timeoffsetrc in your home directory.
3. By using the file /etc/timeoffsetrc for a system-wide default.

Examples:

1. Setting the environment variables

```bash
export LD_PRELOAD=/path/to/libfaketime.so
export TIME_OFFSET=-86400  # yesterday
# (now run any command you want) #
```

2. Or it can be done by specifying it on the command line itself:

```bash
LD_PRELOAD=/path/to/libfaketime.so \
TIME_OFFSET=-86400 \
# your_command_here #
```

3. using the file .timeoffsetrc

```bash
echo "TIME_OFFSET=-86400" > ~/.timeoffsetrc
export LD_PRELOAD=/path/to/libfaketime.so
# your_command_here #
```

4. using the file /etc/timeoffsetrc

```bash
echo "TIME_OFFSET=-86400" > /etc/timeoffsetrc
export LD_PRELOAD=/path/to/libfaketime.so
# your_command_here #
```

5. Dockerfile

```Dockerfile
ENV LD_PRELOAD=/path/to/libfaketime.so
ENV TIME_OFFSET=-86400
ENTRYPOINT ["/my-app"]
```

6. script

```bash
bash ./faketime.sh "2024-08-30 18:00:00"
LD_PRELOAD=./libfaketime.so.1 \
date # your command here
# Fri Aug 30 18:00:00 UTC 2024
```

## License

- This project is a free and unencumbered software released into the public domain. Plese visit [unlicense.org](https://unlicense.org/) for more details.
