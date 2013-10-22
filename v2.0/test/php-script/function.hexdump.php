<?php
/**
 * View any string as a hexdump.
 *
 * This is most commonly used to view binary data from streams
 * or sockets while debugging, but can be used to view any string
 * with non-viewable characters.
 *
 * @version      1.2.2
 * @author       Aidan Lister <aidan@php.net>
 * @author       Peter Waller <iridum@php.net>
 * @param        string  $data        The string to be dumped
 * @param        bool    $htmloutput  Set to false for non-HTML output
 * @param        bool    $uppercase   Set to true for uppercase hex
 * @param        bool    $return      Set to true to return the dump
 */
function hexdump ($data, $htmloutput = true, $uppercase = false, $return = false)
{
    $hexi = $ascii = '';
    $dump = $htmloutput === true ?
                '<pre>' :
                '';

    // Upper or lower case hexidecimal
    if ($uppercase === false) {
        $x = 'x';
    } else {
        $x = 'X';
    }

    // Split the data into lines 16 chars long
    $lines = array();
    $i = 0;
    while ($i * 16 < strlen($data)) {
        $lines[] = substr($data, 16 * $i++, 16);
    }
    
    // Loop through each line
    $offset = 0;
    foreach ($lines as $line) {
        // Loop through each char in the line
        for ($i = 0; $i < strlen($line); $i++) {
            // Convert to hexidecimal
            $hexi .= sprintf("%02$x ", ord($line{$i}));

            // Replace non-viewable bytes with '.'
            if (ord($line{$i}) >= 32) {
                $ascii .= $htmloutput === true ?
                                htmlentities($line{$i}) :
                                $line{$i};
            } else {
                $ascii .= '.';
            }

            // Split into two columns
            if ($i == 7) {
                $hexi .= ' ';
                $ascii .= ' ';
            }
        }

        // Join the hexi / ascii output
        $dump .= sprintf("%04$x  %-49s  %s\n", $offset, $hexi, $ascii);
        // Line count
        $offset += 16;
        // Reset
        $hexi = $ascii = '';
    }
    
    // Strip last newline
    $dump = substr($dump, 0, -1);
    $dump .= $htmloutput === true ?
                '</pre>' :
                '';
    $dump .= "\n";

    // Output method
    if ($return === false) {
        echo $dump;
    } else {
        return $dump;
    }
}

?>