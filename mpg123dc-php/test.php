<?php

include ("mpg123dc.php");

$controler = new MPG123DC("192.168.1.3", 3322, "/mnt/mp3-home/johan/dev/mpg123d/mpg123dc/mpg123dc");

echo $controler->getStatus();

?>