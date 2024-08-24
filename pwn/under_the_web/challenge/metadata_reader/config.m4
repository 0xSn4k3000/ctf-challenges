dnl config.m4 for extension metadata_reader

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([metadata_reader],
dnl   [for metadata_reader support],
dnl   [AS_HELP_STRING([--with-metadata_reader],
dnl     [Include metadata_reader support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([metadata_reader],
  [whether to enable metadata_reader support],
  [AS_HELP_STRING([--enable-metadata_reader],
    [Enable metadata_reader support])],
  [no])

if test "$PHP_METADATA_READER" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, METADATA_READER_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-metadata_reader -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/metadata_reader.h"  # you most likely want to change this
  dnl if test -r $PHP_METADATA_READER/$SEARCH_FOR; then # path given as parameter
  dnl   METADATA_READER_DIR=$PHP_METADATA_READER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for metadata_reader files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       METADATA_READER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$METADATA_READER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the metadata_reader distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-metadata_reader -> add include path
  dnl PHP_ADD_INCLUDE($METADATA_READER_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-metadata_reader -> check for lib and symbol presence
  dnl LIBNAME=METADATA_READER # you may want to change this
  dnl LIBSYMBOL=METADATA_READER # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_METADATA_READER_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your metadata_reader library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $METADATA_READER_DIR/$PHP_LIBDIR, METADATA_READER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_METADATA_READER_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your metadata_reader library.])
  dnl ],[
  dnl   -L$METADATA_READER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(METADATA_READER_SHARED_LIBADD)

  dnl In case of no dependencies
  AC_DEFINE(HAVE_METADATA_READER, 1, [ Have metadata_reader support ])

  PHP_NEW_EXTENSION(metadata_reader, metadata_reader.c, $ext_shared)
fi
