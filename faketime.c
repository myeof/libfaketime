#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static time_t time_offset = 0;
static bool initialized = false;

/*
    real time functions

    static time_t       (*real_time)            (time_t *);
    static int          (*real_ftime)           (struct timeb *);
    static int          (*real_gettimeofday)    (struct timeval *, void *);
    static int          (*real_clock_gettime)   (clockid_t clk_id, struct timespec *tp);
    static int          (*real_timespec_get)    (struct timespec *ts, int base);
    static int          (*real_nanosleep)       (const struct timespec *req, struct timespec *rem);
 */

static time_t real_time(time_t *t)
{
    long ret;
    asm("syscall" : "=a"(ret) : "0"(__NR_time), "D"(t) : "memory");
    return ret;
}

static int real_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    long ret;
    asm("syscall" : "=a"(ret) : "0"(__NR_gettimeofday), "D"(tv), "S"(tz) : "memory");
    return ret;
}

static int real_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    long ret;
    asm("syscall" : "=a"(ret) : "0"(__NR_clock_gettime), "D"(clk_id), "S"(tp) : "memory");
    return ret;
}

/*
    get_time_offset function
 */

static time_t get_time_offset()
{
    if (initialized)
    {
        // printf("initialized: TIME_OFFSET=%ld\n", time_offset);
        return time_offset;
    }

    // get the value of the environment variable TIME_OFFSET
    const char *env = getenv("TIME_OFFSET");
    if (env != NULL)
    {
        if (sizeof(time_t) <= sizeof(int))
        {
            time_offset = atoi(env);
        }
        else
        {
            time_offset = atoll(env);
        }
        // printf("env TIME_OFFSET=%ld\n", time_offset);
        initialized = true;
        return time_offset;
    }
    // printf("TIME_OFFSET: env not found\n");

    // get the value of the file ${HOME}/.timeoffsetrc
    const char *home = getenv("HOME");
    if (home != NULL)
    {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s/.timeoffsetrc", home);
        int fd = syscall(SYS_open, buffer, O_RDONLY);
        if (fd != -1)
        {
            // printf("found: %s\n", buffer);
            int n = syscall(SYS_read, fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0';
                if (sizeof(time_t) <= sizeof(int))
                {
                    time_offset += atoi(buffer);
                }
                else
                {
                    time_offset += atoll(buffer);
                }
            }
            syscall(SYS_close, fd);
            // printf("TIME_OFFSET=%ld\n", time_offset);
            initialized = true;
            return time_offset;
        }
    }
    // printf("${HOME}/.timeoffsetrc: file not found\n");

    // get the value of the file /etc/timeoffsetrc
    int fd = syscall(SYS_open, "/etc/timeoffsetrc", O_RDONLY);
    if (fd != -1)
    {
        // printf("found /etc/timeoffsetrc\n");
        char buffer[1024];
        int n = syscall(SYS_read, fd, buffer, sizeof(buffer) - 1);
        if (n > 0)
        {
            buffer[n] = '\0';
            if (sizeof(time_t) <= sizeof(int))
            {
                time_offset += atoi(buffer);
            }
            else
            {
                time_offset += atoll(buffer);
            }
        }
        syscall(SYS_close, fd);
        // printf("TIME_OFFSET=%ld\n", time_offset);
        initialized = true;
        return time_offset;
    }
    // printf("/etc/timeoffsetrc: file not found\n");
    // printf("TIME_OFFSET=%ld\n", time_offset);
    initialized = true;
    return time_offset;
}

/*
   fake time functions

    time_t time(time_t *t);
    int ftime(struct timeb *tb);
    int gettimeofday(struct timeval *tv, struct timezone *tz);
    int clock_gettime(clockid_t clk_id, struct timespec *tp);
    int timespec_get(struct timespec *ts, int base);
 */

time_t time(time_t *t)
{
    time_t original_time = real_time(NULL);
    time_t modified_time = original_time + get_time_offset();
    if (t)
    {
        *t = modified_time;
    }
    return modified_time;
}

int ftime(struct timeb *tb)
{
    struct timespec tp;
    int result;

    result = clock_gettime(CLOCK_REALTIME, &tp);
    if (result == -1)
        return -1;

    tb->time = tp.tv_sec;
    tb->millitm = tp.tv_nsec / 1000000;
    tb->timezone = 0;
    tb->dstflag = 0;

    return result;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    int result = real_gettimeofday(tv, tz);
    if (result == 0)
    {
        tv->tv_sec += get_time_offset();
    }

    return result;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    int result = real_clock_gettime(clk_id, tp);

    if (result == 0)
    {
        tp->tv_sec += get_time_offset();
    }

    return result;
}

int timespec_get(struct timespec *ts, int base)
{
    if (base != TIME_UTC)
    {
        return 0;
    }

    return clock_gettime(CLOCK_REALTIME, ts);
}