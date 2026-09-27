AC_INIT([farrar], [0.1.0])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX

AC_CONFIG_HEADERS([config.h])

AC_ARG_WITH(
    [vec-arch],
    [AS_HELP_STRING([--with-vec-arch=ARCH],
        [select vector architecture: avx or rvv])],
    [VEC_ARCH="$withval"],
    [VEC_ARCH="avx"]
)

case "$VEC_ARCH" in
    avx)
        AC_MSG_NOTICE([Vector architecture: AVX])
        AM_CONDITIONAL([USE_AVX], [true])
        AM_CONDITIONAL([USE_RVV], [false])
        AC_DEFINE([USE_AVX], [1], [Use AVX backend])
        ;;

    rvv)
        AC_MSG_NOTICE([Vector architecture: RVV])
        AM_CONDITIONAL([USE_AVX], [false])
        AM_CONDITIONAL([USE_RVV], [true])
        AC_DEFINE([USE_RVV], [1], [Use RVV backend])
        ;;

    *)
        AC_MSG_ERROR([
            Invalid vector architecture '$VEC_ARCH'.
            Use --with-vec-arch=avx or --with-vec-arch=rvv.
        ])
        ;;
esac

AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT