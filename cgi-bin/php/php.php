<?php
// Specify the full path to curl in case the path isn't set
$curlPath = '/usr/bin/curl'; // Adjust path if necessary
$var = "$curlPath -s http://localhost:9000/php/php.php";

// Execute the command and capture both stdout and stderr
$out = shell_exec($var);

// If there's no output, capture any errors
if ($out === null) {
    echo "Error executing command: " . $var;
} else {
    echo $out;
}
?>