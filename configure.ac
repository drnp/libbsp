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

# Allocator
allocator="ptmalloc"
AC_ARG_WITH([allocator], 
    [AS_HELP_STRING([--with-allocator=NAME], [Specified memory allocator: jemalloc / tcmalloc / ptmalloc (default)])], 
    [
        if test "x$withval" != "xptmalloc"; then
            allocator=$withval
        fi
    ]
)

je_dir="(system)"
AC_ARG_WITH([jemalloc-dir], 
    [AS_HELP_STRING([--with-jemalloc-dir=DIR], [Specify installation directory of jemalloc])], 
    [je_dir=$withval]
)

tc_dir="(system)"
AC_ARG_WITH([tcmalloc-dir], 
    [AS_HELP_STRING([--with-tcmalloc-dir=DIR], [Specify installation directory of tcmalloc])], 
    [tc_dir=$withval]
)

saved_LIBS="$LIBS"
saved_LDFLAGS="$LDFLAGS"
saved_CPPFLAGS="$CPPFLAGS"
je_found=no
tc_found=no
if test "$allocator" = "jemalloc"; then
    AC_CACHE_CHECK([for libjemalloc installation], ac_cv_je_dir, 
    [
        for je in $je_dir "" /usr /usr/local; do
            LIBS="-ljemalloc $saved_LIBS"
            LDFLAGS="$saved_LDFLAGS"
            CPPFLAGS="$saved_CPPFLAGS"
            if test ! -z "$je" -a ! -d "$je"; then
                continue;
            fi
            if test ! -z "$je"; then
                if test -d "$je/lib64"; then
                    LDFLAGS="-L$je/lib64 $saved_LDFLAGS"
                elif test -d "$je/lib"; then
                    LDFLAGS="-L$je/lib $saved_LDFLAGS"
                else
                    LDFLAGS="-L$je $saved_LDFAGLS"
                fi
                if test -d "$je/include"; then
                    CPPFLAGS="-I$je/include $saved_CPPFLAGS"
                else
                    CPPFLAGS="-I$je $saved_CPPFLAGS"
                fi
            fi
            #Try link
            AC_TRY_LINK(
                [#include <jemalloc/jemalloc.h>], 
                [malloc(1);], 
                [je_linked=yes], 
                [je_linked=no]
            )
            if test $je_linked = yes; then
                if test ! -z $je; then
                    ac_cv_je_dir=$je
                else
                    ac_cv_je_dir="(system)"
                fi
                je_found=yes
                break
            fi
        done
        if test $je_found = no; then
            AC_MSG_ERROR([Cannot find jemalloc])
        fi
    ])
elif test "$allocator" = "tcmalloc"; then
    AC_CACHE_CHECK([for libtcmalloc installation], ac_cv_tc_dir, 
    [
        for tc in $tc_dir "" /usr /usr/local; do
            LIBS="-ltcmalloc $saved_LIBS"
            LDFLAGS="$saved_LDFLAGS"
            CPPFLAGS="$saved_CPPFLAGS"
            if test ! -z "$tc" -a ! -d "$tc"; then
                continue;
            fi
            if test ! -z "$tc"; then
                if test -d "$tc/lib64"; then
                    LDFLAGS="-L$tc/lib64 $saved_LDFLAGS"
                elif test -d "$tc/lib"; then
                    LDFLAGS="-L$tc/lib $saved_LDFLAGS"
                else
                    LDFLAGS="-L$tc $saved_LDFAGLS"
                fi
                if test -d "$tc/include"; then
                    CPPFLAGS="-I$tc/include $saved_CPPFLAGS"
                else
                    CPPFLAGS="-I$tc $saved_CPPFLAGS"
                fi
            fi
            #Try link
            AC_TRY_LINK(
                [#include <gperftools/tcmalloc.h>], 
                [malloc(1);], 
                [tc_linked=yes], 
                [tc_linked=no]
            )
            if test $tc_linked = yes; then
                if test ! -z $tc; then
                    ac_cv_tc_dir=$tc
                else
                    ac_cv_tc_dir="(system)"
                fi
                tc_found=yes
                break
            fi
        done
        if test $tc_found = no; then
            AC_MSG_ERROR([Cannot find tcmalloc])
        fi
    ])
fi

if test "$trybspspin" = "yes"; then
    AC_SUBST([ac_cv_enable_bsp_spinlock], [1])
else
    AC_SUBST([ac_cv_enable_bsp_spinlock], [0])
fi
if test "$je_found" = "yes"; then
    AC_SUBST([ac_cv_enable_jemalloc], [1])
else
    AC_SUBST([ac_cv_enable_jemalloc], [0])
fi
if test "$tc_found" = "yes"; then
    AC_SUBST([ac_cv_enable_tcmalloc], [1])
else
    AC_SUBST([ac_cv_enable_tcmalloc], [0])
fi
AC_CONFIG_FILES([src/bsp.h])

AC_OUTPUT([
Makefile
src/bsp.pc
src/Makefile])
