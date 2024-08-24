--TEST--
test1() Basic test
--EXTENSIONS--
metadata_reader
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension metadata_reader is loaded and working!
NULL
