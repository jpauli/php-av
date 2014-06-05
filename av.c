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
	php_av_file *av_file = NULL;

	av_file = (php_av_file *)rsrc->ptr;

	php_av_file_close(av_file);
}

static void avstream_resource_dtor(zend_rsrc_list_entry *rsrc)
{
	php_av_stream *stream = NULL;

	stream = (php_av_stream *)rsrc->ptr;

	php_av_stream_close(stream);
}

static void _resource_close(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *res = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		return;
	}

	zend_list_delete(Z_RESVAL_P(res));
}

static php_av_file *php_av_file_open(char *filename)
{
	AVFormatContext *av_ctx = NULL;
	php_av_file *file       = NULL;
	int ret = 0;

	if (ret = avformat_open_input(&av_ctx, filename, NULL, NULL)) {
		AV_SET_LAST_ERROR(ret);
		return NULL;
	}

	avformat_find_stream_info(av_ctx, NULL);

	file = ecalloc(1, sizeof(*file));
	file->av_file = av_ctx;
	file->is_user_opened = 1;

	return file;
}

static php_av_stream *php_av_stream_open_from_file(php_av_file *file, uint stream_no)
{
	php_av_stream *php_av_stream = NULL;

	if (stream_no > file->av_file->nb_streams) {
		return NULL;
	}

	php_av_stream = ecalloc(1, sizeof(*php_av_stream));
	php_av_stream->php_av_file = file;
	php_av_stream->stream      = file->av_file->streams[stream_no];
	file->user_opened_streams++;

	return php_av_stream;
}

static void php_av_stream_close(php_av_stream *av_stream)
{
	av_stream->php_av_file->user_opened_streams--;
	_php_av_file_dtor(av_stream->php_av_file);
	efree(av_stream);
}

static void php_av_file_close(php_av_file *av_file)
{
	av_file->is_user_opened = 0;
	_php_av_file_dtor(av_file);
}

static void _php_av_file_dtor(php_av_file *php_av_file)
{
	if (!php_av_file->is_user_opened && php_av_file->user_opened_streams == 0) {
		avformat_free_context(php_av_file->av_file);
		efree(php_av_file);
	}
}

static const char *_php_av_get_stream_type(php_av_stream *pas)
{
	switch (pas->stream->codec->codec_type) {
	case AVMEDIA_TYPE_AUDIO:
		return "audio";
	case AVMEDIA_TYPE_VIDEO:
		return "video";
	case AVMEDIA_TYPE_SUBTITLE:
		return "subtitle";
	}

	return "unknown";
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
	char *filename       = NULL;
	int filename_len;
	php_av_file *av_file = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &filename, &filename_len) == FAILURE) {
		return;
	}

	av_file = php_av_file_open(filename);
	if (!av_file) {
		AV_LAST_ERROR_TO_PHP
		return;
	}

	ZEND_REGISTER_RESOURCE(return_value, (void *)av_file, le_avfile_resource);
}

PHP_FUNCTION(av_file_close)
{
	_resource_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(av_file_get_metadata)
{
	php_av_file *paf = NULL;
	zval *res        = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(paf, php_av_file *, &res, -1, AVFILE_RESOURCE_NAME, le_avfile_resource);

	array_init(return_value);
	add_assoc_long(return_value, "nb_streams", paf->av_file->nb_streams);
	add_assoc_string(return_value, "filename", paf->av_file->filename, 1);
	add_assoc_long(return_value, "duration", paf->av_file->duration);
	add_assoc_long(return_value, "bit_rate", paf->av_file->bit_rate);
}

PHP_FUNCTION(av_stream_open)
{
	php_av_file *paf   = NULL;
	zval *res          = NULL;
	php_av_stream *pas = NULL;
	long stream_no     = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &res, &stream_no) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(paf, php_av_file *, &res, -1, AVFILE_RESOURCE_NAME, le_avfile_resource);

	pas = php_av_stream_open_from_file(paf, stream_no);

	if (!pas) {
		php_error(E_WARNING, "Unknown stream number %d", stream_no);
		return;
	}

	ZEND_REGISTER_RESOURCE(return_value, (void *)pas, le_avstream_resource);
}

PHP_FUNCTION(av_stream_close)
{
	_resource_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(av_stream_get_metadata)
{
	php_av_stream *pas = NULL;
	zval *res          = NULL;
	AVCodec *codec     = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(pas, php_av_stream *, &res, -1, AVSTREAM_RESOURCE_NAME, le_avstream_resource);

	codec = avcodec_find_decoder(pas->stream->codec->codec_id);

	array_init(return_value);
	add_assoc_string(return_value, "type", (char *)_php_av_get_stream_type(pas), 1);
	add_assoc_string(return_value, "codec", (char *)codec->long_name, 1);
	add_assoc_long(return_value, "duration", pas->stream->duration);
	if (codec->type == AVMEDIA_TYPE_VIDEO) {
		add_assoc_long(return_value, "width", pas->stream->codec->width);
		add_assoc_long(return_value, "height", pas->stream->codec->height);
	}
	if (codec->type == AVMEDIA_TYPE_AUDIO) {
		add_assoc_long(return_value, "audio_channels", pas->stream->codec->channels);
		add_assoc_long(return_value, "sample_rate", pas->stream->codec->sample_rate);
		add_assoc_long(return_value, "bit_rate", pas->stream->codec->bit_rate);
	}
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
