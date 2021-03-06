--TEST--
SPL: IteratorIterator and ArrayIterator/Object
--SKIPIF--
<?php if (!extension_loaded("spl")) print "skip"; ?>
--FILE--
<?php

class ArrayIteratorEx extends ArrayIterator
{
	function rewind()
	{
		echo __METHOD__ . "\n";
		return parent::rewind();
	}
}

$it = new ArrayIteratorEx(range(0,3));

foreach(new IteratorIterator($it) as $v)
{
	var_dump($v);
}

class ArrayObjectEx extends ArrayObject
{
	function getIterator()
	{
		echo __METHOD__ . "\n";
		return parent::getIterator();
	}
}

$it = new ArrayObjectEx(range(0,3));

foreach(new IteratorIterator($it) as $v)
{
	var_dump($v);
}

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
ArrayIteratorEx::rewind
int(0)
int(1)
int(2)
int(3)
ArrayObjectEx::getIterator
int(0)
int(1)
int(2)
int(3)
===DONE===
