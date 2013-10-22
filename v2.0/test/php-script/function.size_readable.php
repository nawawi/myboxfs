<?php
/**
 * Return human readable sizes
 *
 * @param       int    $size        Size
 * @param       int    $unit        The maximum unit
 * @param       int    $retstring   The return string format
 * @author      Aidan Lister <aidan@php.net>
 * @version     1.1.0
 */
function size_readable($size, $unit = null, $retstring = null)
{
    // Units
    $sizes = array('B', 'KB', 'MB', 'GB', 'TB');
    $ii = count($sizes) - 1;

    // Max unit
    $unit = array_search((string) $unit, $sizes);
    if ($unit === null || $unit === false) {
        $unit = $ii;
    }

    // Return string
    if ($retstring === null) {
        $retstring = '%01.2f %s';
    }

    // Loop
    $i = 0;
    while ($unit != $i && $size >= 1024 && $i < $ii) {
        $size /= 1024;
        $i++;
    }

    return sprintf($retstring, $size, $sizes[$i]);
}

?>