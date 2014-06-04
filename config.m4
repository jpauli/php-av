dnl $Id$
dnl config.m4 for extension av

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

 PHP_ARG_WITH(av, for av support,
dnl Make sure that the comment is aligned:
 [  --with-av[=DIR]          Include av support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(av, whether to enable av support,
dnl Make sure that the comment is aligned:
dnl [  --enable-av           Enable av support])

if test "$PHP_AV" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-av -> check with-path
  SEARCH_PATH="/usr/local/include /usr/include"     # you might want to change this
  SEARCH_FOR="/libavformat/avformat.h"  # you most likely want to change this
  if test -r $PHP_AV/$SEARCH_FOR; then # path given as parameter
     AV_DIR=$PHP_AV
   else # search default path list
     AC_MSG_CHECKING([for av files in default path])
     for i in $SEARCH_PATH ; do
       if test -r $i/$SEARCH_FOR; then
         AV_DIR=$i
         AC_MSG_RESULT(found in $i)
       fi
     done
   fi
  dnl
   if test -z "$AV_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([Please reinstall the libav library])
   fi

  dnl # --with-av -> add include path
  PHP_ADD_INCLUDE($AV_DIR/include)

  dnl # --with-av -> check for lib and symbol presence
  LIBNAME=avformat # you may want to change this
  LIBSYMBOL=av_register_all # you most likely want to change this 

   PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
   [
     PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AV_DIR/lib, AV_SHARED_LIBADD)
     AC_DEFINE(HAVE_AVLIB,1,[ ])
   ],[
     AC_MSG_ERROR([wrong av lib version or lib not found])
   ],[
     -L$AV_DIR/lib -lm
   ])
  
  PHP_SUBST(AV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(av, av.c, $ext_shared)
fi
