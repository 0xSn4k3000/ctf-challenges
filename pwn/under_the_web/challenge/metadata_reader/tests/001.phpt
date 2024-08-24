--TEST--
Check if metadata_reader is loaded
--EXTENSIONS--
metadata_reader
--FILE--
<?php
echo 'The extension "metadata_reader" is available';
?>
--EXPECT--
The extension "metadata_reader" is available
