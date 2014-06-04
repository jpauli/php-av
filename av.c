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
  | Author: Julien PAULI <jpauli@php.net>                                |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_av.h"

/* If you declare any globals in php_av.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(av)
*/
#define LIBAV_ERROR_BUF_LEN 255

#define AVFILE_RESOURCE_NAME "avfile_resource"
#define AVSTREAM_RESOURCE_NAME "avstream_resource"

#define AV_ERROR_TO_PHP(averror_num) do { \
	char error[LIBAV_ERROR_BUF_LEN]; \
	av_strerror(averror_num, error, LIBAV_ERROR_BUF_LEN); \
	php_error(E_WARNING, "libav error : %.*s", LIBAV_ERROR_BUF_LEN, error); \
	return; \
} while(0);

static int le_avfile_resource;
static int le_avstream_resource;

ZEND_BEGIN_ARG_INFO(arginfo_av_file_open, 0)
ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_av_file_close, 0)
ZEND_ARG_INFO(0, avfile_resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_av_stream_open, 0)
ZEND_ARG_INFO(0, avfile_resource)
ZEND_ARG_INFO(0, stream_no)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_av_stream_close, 0)
ZEND_ARG_INFO(0, avstream_resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_av_file_get_metadata, 0)
ZEND_ARG_INFO(0, avfile_resource)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_av_stream_get_metadata, 0)
ZEND_ARG_INFO(0, avstream_resource)
ZEND_END_ARG_INFO()

const zend_function_entry av_functions[] = {
	PHP_FE(av_file_open,	arginfo_av_file_open)
	PHP_FE(av_file_close,	arginfo_av_file_close)
	PHP_FE(av_file_get_metadata,	arginfo_av_file_get_metadata)
	PHP_FE(av_stream_open,	arginfo_av_stream_open)
	PHP_FE(av_stream_close,	arginfo_av_stream_close)
	PHP_FE(av_stream_get_metadata,	arginfo_av_stream_get_metadata)
	PHP_FE(av_libav_get_info,	NULL)
	PHP_FE_END
};
/* }}} */

/* {{{ av_module_entry
 */
zend_module_entry av_module_entry = {
	STANDARD_MODULE_HEADER,
	"av",
	av_functions,
	PHP_MINIT(av),
	PHP_MSHUTDOWN(av),
	PHP_RINIT(av),
	PHP_RSHUTDOWN(av),
	PHP_MINFO(av),
	PHP_AV_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_AV
ZEND_GET_MODULE(av)
#endif

static void avfile_resource_dtor(zend_rsrc_list_entry *rsrc)
{
	AVFormatContext *av_file = NULL;

	av_file = (AVFormatContext *)rsrc->ptr;

	avformat_free_context(av_file);
}

static void avstream_resource_dtor(zend_rsrc_list_entry *rsrc)
{

}

PHP_MINIT_FUNCTION(av)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/

	le_avfile_resource   = zend_register_list_destructors_ex(avfile_resource_dtor, NULL, AVFILE_RESOURCE_NAME, module_number);
	le_avstream_resource = zend_register_list_destructors_ex(avstream_resource_dtor, NULL, AVSTREAM_RESOURCE_NAME, module_number);

	av_register_all();

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(av)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}

PHP_RINIT_FUNCTION(av)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(av)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(av)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "av support", "enabled");
	php_info_print_table_header(2, "libav version", avformat_version());
	php_info_print_table_row(1, "av extension developped by Julien PAULI");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}

PHP_FUNCTION(av_file_open)
{
	char *filename = NULL;
	int filename_len;
	AVFormatContext *av_file = NULL;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
		return;
	}

	if (ret = avformat_open_input(&av_file, filename, NULL, NULL)) {
		AV_ERROR_TO_PHP(ret);
	}

	avformat_find_stream_info(av_file, NULL);

	ZEND_REGISTER_RESOURCE(return_value, (void *)av_file, le_avfile_resource);
}

PHP_FUNCTION(av_file_close)
{
	AVFormatContext *av_file = NULL;
	zval *res = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(av_file, void *, &res, -1, AVFILE_RESOURCE_NAME, le_avfile_resource);

	zend_list_delete(Z_RESVAL_P(res));
}


PHP_FUNCTION(av_file_get_metadata)
{
	AVFormatContext *av_file = NULL;
	zval *res = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(av_file, void *, &res, -1, AVFILE_RESOURCE_NAME, le_avfile_resource);

	array_init(return_value);
	add_assoc_long(return_value, "nb_streams", av_file->nb_streams);
	add_assoc_string(return_value, "filename", av_file->filename, 1);
	add_assoc_long(return_value, "duration", av_file->duration);
	add_assoc_long(return_value, "bit_rate", av_file->bit_rate);
}

PHP_FUNCTION(av_stream_open)
{

}

PHP_FUNCTION(av_stream_close)
{

}

PHP_FUNCTION(av_stream_get_metadata)
{

}

PHP_FUNCTION(av_libav_get_info)
{
	char *version = NULL;
	int version_len;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	version_len = spprintf(&version, 0, "%d.%d.%d", LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO);
	array_init(return_value);

	add_assoc_stringl(return_value, "version", version, version_len, 0);
	add_assoc_string(return_value, "configuration", (char *)avformat_configuration(), 1);
	add_assoc_string(return_value, "license", (char *)avformat_license(), 1);
}
