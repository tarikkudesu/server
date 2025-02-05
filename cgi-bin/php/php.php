<?php
$var = 'curl -s -x GET http://localhost:9000/php.php';

$out = shell_exec($var);

echo $out;
?>