AC_INIT([farrar], [0.1.0])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX

AC_CONFIG_HEADERS([config.h])

AC_ARG_WITH(
    [vec-arch],
    [AS_HELP_STRING([--with-vec-arch=ARCH],
        [select vector architecture: avx, rvv, or sve])],
    [VEC_ARCH="$withval"],
    [VEC_ARCH="avx"]
)

case "$VEC_ARCH" in
    avx)
        AC_MSG_NOTICE([Vector architecture: AVX])
        AM_CONDITIONAL([USE_AVX], [true])
        AM_CONDITIONAL([USE_RVV], [false])
        AM_CONDITIONAL([USE_SVE], [false])
        AC_DEFINE([USE_AVX], [1], [Use AVX backend])
        ;;

    rvv)
        AC_MSG_NOTICE([Vector architecture: RVV])
        AM_CONDITIONAL([USE_AVX], [false])
        AM_CONDITIONAL([USE_RVV], [true])
        AM_CONDITIONAL([USE_SVE], [false])
        AC_DEFINE([USE_RVV], [1], [Use RVV backend])
        ;;

    sve)
        AC_MSG_NOTICE([Vector architecture: SVE])
        AM_CONDITIONAL([USE_AVX], [false])
        AM_CONDITIONAL([USE_RVV], [false])
        AM_CONDITIONAL([USE_SVE], [true])
        AC_DEFINE([USE_SVE], [1], [Use Arm SVE backend])
        ;;

    *)
        AC_MSG_ERROR([
            Invalid vector architecture '$VEC_ARCH'.
            Use --with-vec-arch=avx, --with-vec-arch=rvv, or --with-vec-arch=sve.
        ])
        ;;
esac

AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT