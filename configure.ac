AC_PREREQ([2.63])
AC_INIT([wacs], [0.1], [andrei.i.ivanov@commandus.com])
AM_INIT_AUTOMAKE([1.11 foreign no-define subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([wacs.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_ENABLE_SHARED
AC_PROG_LIBTOOL

AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

# MacOS: 
#   brew install openssl@1.1
# install openssl into /usr/local/lib/opt/openssl@1.1
LDFLAGS="$LDFLAGS -L/usr/local/lib -L/usr/local/opt/openssl@1.1/lib"

# Checks for header files. Add /usr/local/include for OS X.
CFLAGS="$CFLAGS -I/usr/local/include"
AC_CHECK_HEADERS([openssl/ssl.h], [], AC_MSG_ERROR([Could not find openssl/ssl.h. Try $ ./configure CFLAGS='-Iyour-openssl-include-path]))
AC_CHECK_HEADERS([nanomsg/nn.h], [], AC_MSG_ERROR([Could not find nanomsg/nn.h. Try $ ./configure CFLAGS='-Iyour-nanomsg-include-path]))
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h string.h sys/time.h termios.h unistd.h])

# Checks for libraries. Add /usr/local/lib for OS X
AC_CHECK_LIB([crypto], [EVP_CIPHER_CTX_reset], [], AC_MSG_ERROR([Could not find openssl >= 1.1.0. Try $ ./configure LDFLAGS='-Lyour-openssl2-lib-path']))
AC_CHECK_LIB([nanomsg], [nn_connect], [], AC_MSG_ERROR([Could not find nanomsg library. Try $ ./configure LDFLAGS='-Lyour-nanomsg-lib-path']))
AC_CHECK_LIB([microhttpd], [MHD_start_daemon], [], AC_MSG_ERROR([Could not find microhttpd library. Try $ ./configure LDFLAGS='-Lyour-microhttpd-lib-path']))
AC_CHECK_LIB([netsnmp], [snmp_log], [], AC_MSG_ERROR([Could not find netsnmp library. Try $ ./configure LDFLAGS='-Lyour-netsnmp-lib-path']))
AC_CHECK_LIB([curl], curl_easy_init, [], AC_MSG_ERROR([Could not find libcurl. Try $ ./configure LDFLAGS='-Lyour-curl-lib-path']))

# Checks for typedefs, structures, and compiler characteristics.
# AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([memmove memset])

# postresgql header
AC_CHECK_HEADERS(libpq-fe.h)
if test "$ac_cv_header_libpq_fe_h" = "yes"; then
  POSTGRES=yes
else
  old="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS -I/usr/include/postgresql"
  AC_CHECK_HEADERS(/usr/include/postgresql/libpq-fe.h)
  if test "$ac_cv_header__usr_include_postgresql_libpq_fe_h" = "yes"; then
    POSTGRES=yes
  else
    CPPFLAGS="$CPPFLAGS -I/usr/include/pgsql"
    AC_CHECK_HEADERS(/usr/include/pgsql/libpq-fe.h)
    if test "$ac_cv_header__usr_include_pgsql_libpq_fe_h" = "yes"; then
      POSTGRES=yes
    else
      CPPFLAGS="$CPPFLAGS -I/usr/local/include/pgsql"
      AC_CHECK_HEADERS(/usr/local/include/pgsql/libpq-fe.h)
      if test "$ac_cv_header__usr_local_include_pgsql_libpq_fe_h" = "yes"; then
	POSTGRES=yes
      else
        CPPFLAGS="$CPPFLAGS -I/usr/pgsql-9.4/include"
        AC_CHECK_HEADERS(/usr/pgsql-9.4/include/libpq-fe.h)
        if test "$ac_cv_header__usr_pgsql_9_4_include_libpq_fe_h" = "yes"; then
          POSTGRES=yes
        else
          CPPFLAGS="$save_CPPFLAGS"
	  POSTGRES=
          echo "**************************************************************"
          echo "Unable to locate libpq (postgres) headers (are they installed)"
          echo "**************************************************************"
        fi
      fi
    fi
  fi
fi

# postgresql library
if test "$POSTGRES" = "yes"; then
  if test -d /usr/pgsql-9.4/lib ; then
    CPPFLAGS="$CPPFLAGS -L/usr/pgsql-9.4/lib"
    LDFLAGS="$LDFLAGS -L/usr/pgsql-9.4/lib"
  else
    if test -d /usr/lib/pgsql ; then
      CPPFLAGS="$CPPFLAGS -L/usr/lib/pgsql"
      LDFLAGS="$LDFLAGS -L/usr/lib/pgsql"
    else
      if test -d /usr/local/lib/pgsql ; then
        CPPFLAGS="$CPPFLAGS -L/usr/local/lib/pgsql"
        LDFLAGS="$LDFLAGS -L/usr/local/lib/pgsql"
      fi
    fi
  fi
  AC_CHECK_LIB(pq,PQfformat)
  if test "$ac_cv_lib_pq_PQfformat" != "yes"; then
      POSTGRES=
      AC_CHECK_LIB(pq,PQclear)
      echo "******************************************************"
      if test "$ac_cv_lib_pq_PQclear" != "yes"; then
	echo "Unable to locate postgres pq library (is it installed)"
      else
	echo "Located postgres pq library, but it is too old to use!"
      fi
      echo "Perhaps you can try 'configure --with-postgres=dir=path'"
      echo "to point to the postgres version you wish to use."
      echo "******************************************************"
  fi
fi

AC_OUTPUT
