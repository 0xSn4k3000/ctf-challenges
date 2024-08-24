/* metadata_reader extension for PHP */

#ifndef PHP_METADATA_READER_H
# define PHP_METADATA_READER_H

extern zend_module_entry metadata_reader_module_entry;
# define phpext_metadata_reader_ptr &metadata_reader_module_entry

# define PHP_METADATA_READER_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_METADATA_READER)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_METADATA_READER_H */
