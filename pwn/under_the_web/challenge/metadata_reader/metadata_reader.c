/* metadata_reader extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include <png.h>
#include "ext/standard/info.h"
#include "php_metadata_reader.h"
#include "metadata_reader_arginfo.h"

#define HEAP_CHUNK 56

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

struct MetaData {
	char *Artist;
	char *Title;
	char *Copyright;
	char *PngName;

	png_structp png_ptr;
    png_infop info_ptr;
	png_textp text_ptr;
};


PHP_FUNCTION(getImgMetadata)
{
	char *filename;
	size_t filename_len;
	FILE *fp;
    
	char Card[256] = "";

    ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(filename, filename_len)
	ZEND_PARSE_PARAMETERS_END();


	struct MetaData *md = (struct MetaData *)emalloc(HEAP_CHUNK);

	md->PngName = filename;

	fp = fopen(md->PngName, "rb");
	if (!fp) {
        php_error_docref(NULL, E_WARNING, "Unable to open file %s", md->PngName);
        return;
    }

	md->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!md->png_ptr) {
        fclose(fp);
        php_error_docref(NULL, E_WARNING, "Unable to create png read struct");
        return;
    }

    md->info_ptr = png_create_info_struct(md->png_ptr);
    if (!md->info_ptr) {
        png_destroy_read_struct(&md->png_ptr, NULL, NULL);
        fclose(fp);
        php_error_docref(NULL, E_WARNING, "Unable to create png info struct");
        return;
    }

    if (setjmp(png_jmpbuf(md->png_ptr))) {
        png_destroy_read_struct(&md->png_ptr, &md->info_ptr, NULL);
        fclose(fp);
        php_error_docref(NULL, E_WARNING, "Error during png creation");
        return;
    }

    png_init_io(md->png_ptr, fp);
    png_read_info(md->png_ptr, md->info_ptr);
   
    int num_text;

    if (png_get_text(md->png_ptr, md->info_ptr, &md->text_ptr, &num_text) > 0) {
        for (int i = 0; i < num_text; i++) {
            if (strcmp(md->text_ptr[i].key, "Title") == 0) {
				if(strlen(md->text_ptr[i].text) > 1){
					md->Title = emalloc(HEAP_CHUNK);

					if (md->Title){
						strcpy(md->Title, md->text_ptr[i].text);
					}
				}
                
            } else if (strcmp(md->text_ptr[i].key, "Artist") == 0) {
                if(strlen(md->text_ptr[i].text) > 1){
					md->Artist = emalloc(HEAP_CHUNK);

					if (md->Artist){
						strcpy(md->Artist, md->text_ptr[i].text);
					}
				}
            } else if (strcmp(md->text_ptr[i].key, "Copyright") == 0) {
                if(strlen(md->text_ptr[i].text) > 1){
					md->Copyright = emalloc(HEAP_CHUNK);

					if (md->Copyright){
						strcpy(md->Copyright, md->text_ptr[i].text);
					}
				}
            }
        }
    }


    png_destroy_read_struct(&md->png_ptr, &md->info_ptr, NULL);
	fclose(fp);
	size_t cardLen = 0;
	
	if (md->Title) {
		cardLen = strlen(Card);
		snprintf(Card+cardLen, 256-cardLen, "Title: %s\n", md->Title);
		efree(md->Title);
	} else {
		cardLen = strlen(Card);
		snprintf(Card+cardLen, 256-cardLen, "Title: UnKnown\n");
	}
	
	if (md->Artist) {
		cardLen = strlen(Card);
		snprintf(Card+cardLen, 256-cardLen, "Artist: %s\n", md->Artist);
		efree(md->Artist);
	} else {
		cardLen = strlen(Card);
		snprintf(Card+cardLen, 256-cardLen, "Artist: UnKnown\n");
	}

	if (md->Copyright) {
		cardLen = strlen(Card);
		snprintf(Card+cardLen, 256-cardLen, "Copyright: %s\n", md->Copyright);
		efree(md->Copyright);
	} else {
		cardLen = strlen(Card);
		snprintf(Card+cardLen, 256-cardLen, "Copyright: UnKnown\n");
	}

	efree(md);
	RETURN_STRING(Card);

}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(metadata_reader)
{
#if defined(ZTS) && defined(COMPILE_DL_METADATA_READER)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(metadata_reader)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "metadata_reader support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ metadata_reader_module_entry */
zend_module_entry metadata_reader_module_entry = {
	STANDARD_MODULE_HEADER,
	"metadata_reader",					/* Extension name */
	ext_functions,					/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(metadata_reader),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(metadata_reader),			/* PHP_MINFO - Module info */
	PHP_METADATA_READER_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_METADATA_READER
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(metadata_reader)
#endif
