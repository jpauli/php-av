/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Julien PAULI <jpauli@php.net>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_AV_H
#define PHP_AV_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#define LIBAV_ERROR_BUF_LEN 255

#define AVFILE_RESOURCE_NAME    "avfile_resource"

#define AVSTREAM_RESOURCE_NAME "avstream_resource"

#define AV_LAST_ERROR_TO_PHP do { \
	if (php_av_last_error) { \
		php_error(E_WARNING, "libav error : %.*s", LIBAV_ERROR_BUF_LEN, php_av_last_error); \
	} \
} while(0);

#define AV_SET_LAST_ERROR(averror_num) av_strerror(averror_num, php_av_last_error, LIBAV_ERROR_BUF_LEN);


static int le_avfile_resource;
static int le_avstream_resource;
static char php_av_last_error[LIBAV_ERROR_BUF_LEN] = {0};


typedef struct {
	AVFormatContext *av_file;
	zend_bool is_user_opened;
	zend_uint user_opened_streams;
} php_av_file;

typedef struct {
	AVStream *stream;
	php_av_file *php_av_file;
} php_av_stream;


static php_av_file *php_av_file_open(char *);
static php_av_stream *php_av_stream_open_from_file(php_av_file *, uint);
static void php_av_stream_close(php_av_stream *);
static void php_av_file_close(php_av_file *);
static void _php_av_file_dtor(php_av_file *);
static void _resource_close(INTERNAL_FUNCTION_PARAMETERS);
static const char *_php_av_get_stream_type(php_av_stream *);

extern zend_module_entry av_module_entry;
#define phpext_av_ptr &av_module_entry

#define PHP_AV_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_AV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_AV_API __attribute__ ((visibility("default")))
#else
#	define PHP_AV_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(av);
PHP_MSHUTDOWN_FUNCTION(av);
PHP_RINIT_FUNCTION(av);
PHP_RSHUTDOWN_FUNCTION(av);
PHP_MINFO_FUNCTION(av);

PHP_FUNCTION(av_file_open);
//PHP_FUNCTION(av_open_fd);
PHP_FUNCTION(av_file_close);
PHP_FUNCTION(av_file_get_metadata);
PHP_FUNCTION(av_stream_open);
PHP_FUNCTION(av_stream_close);
PHP_FUNCTION(av_stream_get_metadata);
PHP_FUNCTION(av_libav_get_info);

#ifdef ZTS
#define AV_G(v) TSRMG(av_globals_id, zend_av_globals *, v)
#else
#define AV_G(v) (av_globals.v)
#endif

#endif	/* PHP_AV_H */
