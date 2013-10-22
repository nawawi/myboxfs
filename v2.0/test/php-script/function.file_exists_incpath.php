<?php
/**
 * Check if a file exists in the include path
 *
 * @version      1.2.0
 * @author       Aidan Lister <aidan@php.net>
 * @param        string     $file       Name of the file to look for
 * @return       bool       TRUE if the file exists, FALSE if it does not
 */
function file_exists_incpath ($file)
{
	$paths = explode(PATH_SEPARATOR, get_include_path());

	foreach ($paths as $path) {
		// Formulate the absolute path
		$fullpath = $path . DIRECTORY_SEPARATOR . $file;

		// Check it
		if (file_exists($fullpath)) {
			return true;
		}
	}

	return false;
}

?>