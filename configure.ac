dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT(BSP.BlackTail, 0.0.1, bugs@bsgroup.org, bsp, http://p.bsgroup.org/)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 subdir-objects])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([disable])
AC_PREFIX_DEFAULT([/usr])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_HEADER_STDC
PKG_PROG_PKG_CONFIG

test "$prefix" = NONE && prefix=/usr

AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_TYPE_SIZE_T
LT_INIT

AC_SEARCH_LIBS(socket, socket, [], [AC_MSG_ERROR([Socket needed])])
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(malloc_usable_size, malloc)
AC_SEARCH_LIBS(pthread_spin_lock, pthread, [], [AC_MSG_ERROR([Pthread needed])])
AC_SEARCH_LIBS(log2, m, [], [AC_MSG_ERROR([GNU math library needed])])
AC_SEARCH_LIBS(gethugepagesizes, hugetlbfs)

AC_CHECK_FUNCS([ \
    dup2 \
    gettimeofday \
    sigignore \
    mmap \
    getaddrinfo \
    munmap \
    strerror \
    strnlen
], [], [AC_MSG_ERROR([Functions missed])])

AC_CHECK_FUNCS([ \
    getpagesizes \
    memcntl
])

AC_CHECK_HEADERS([ \
    ctype.h \
    errno.h \
    error.h \
    math.h \
    inttypes.h \
    fcntl.h \
    limits.h \
    netdb.h \
    pthread.h \
    sched.h \
    signal.h \
    stdarg.h \
    stdint.h \
    stdlib.h \
    stdio.h \
    string.h \
    time.h \
    unistd.h \
    sys/stat.h \
    sys/socket.h \
    sys/sysinfo.h \
    sys/syscall.h \
    sys/uio.h \
    sys/mman.h \
    sys/resource.h \
    sys/time.h \
    sys/types.h \
    sys/un.h \
    netinet/tcp.h \
    netinet/udp.h \
    arpa/inet.h
], [], [AC_MSG_ERROR([System header missed])])

AC_CHECK_HEADERS([ \
    malloc.h \
    sys/malloc.h \
    malloc/mallo.h
])

AC_MSG_CHECKING([Operating System])
AC_MSG_RESULT($host)
case $host in
*-linux*)
    # Linux
    AC_MSG_RESULT([Linux back-end)])
    AC_DEFINE(OS_LINUX, 1, [Linux implementations])
    AC_SUBST(OS_LINUX)
    AC_CHECK_HEADERS([ \
        sys/epoll.h \
        sys/eventfd.h \
        sys/timerfd.h \
        sys/signalfd.h
    ], [
        AC_DEFINE(EVENT_USE_EPOLL, 1, [Epoll supported])
    ], [
        AC_DEFINE(EVENT_USE_SELECT, 1, [Epoll unsupported])
    ])
    ;;
*-darwin*|*-*bsd*)
    # MacOS & BSD
    AC_MSG_RESULT([BSD back-end])
    AC_DEFINE(OS_BSD, 1, [BSD implementations])
    AC_SUBST(OS_BSD)
    AC_CHECK_HEADERS([ \
        sys/event.h
    ], [
        AC_DEFINE(EVENT_USE_KQUEUE, 1, [Kqueue supported])
    ], [
        AC_DEFINE(EVENT_USE_SELECT, 1, [Kqueue unsupported])
    ])
    ;;
*)
    # Other system
    AC_MSG_RESULT([Other general OS])
    AC_DEFINE(OS_OTHER, 1, [Other general OS])
    AC_SUBST(OS_OTHER)
    AC_CHECK_HEADERS([ \
        sys/select.h
    ])
    AC_DEFINE(EVENT_USE_SELECT, 1, [Other OS])
    ;;
esac

CPPFLAGS="-I$PWD -I$PWD/src"
CFLAGS="-O3 -g -Wall"

AC_DEFUN([AC_C_ENDIAN],
[AC_CACHE_CHECK(for endianness, ac_cv_c_endian,
[
    AC_RUN_IFELSE(
        [AC_LANG_PROGRAM([], [
            long val = 1;
            char *c = (char *) &val;
            exit(*c == 1);
        ])
    ],[
        ac_cv_c_endian=big
    ],[
        ac_cv_c_endian=little
    ])
])
if test $ac_cv_c_endian = big; then
    AC_DEFINE(ENDIAN_BIG, 1, [machine is big-endian])
fi
if test $ac_cv_c_endian = little; then
    AC_DEFINE(ENDIAN_LITTLE, 1, [machine is little-endian])
fi
])

AC_C_ENDIAN

# BSP Spinlock
trybspspin="no"
AC_ARG_ENABLE([bsp-spinlock], 
    [AS_HELP_STRING([--enable-bsp-spinlock], [Use BSP.Spinlock replace to pthread lock])], 
    [trybspspin=$enableval]
)
if test "$trybspspin" = "yes"; then
    AC_DEFINE([ENABLE_BSP_SPINLOCK], 1, [Enable BSP.Spinlock])
fi

AC_OUTPUT([
Makefile
src/bsp.pc
src/Makefile])
