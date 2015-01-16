<?php

/**
 * @file
 * Provides functions for creating XFDF files.
 */

/**
 * create_xfdf
 *
 * Takes values passed via associative array and generates XFDF file format
 * with that data for the pdf address sullpiled.
 *
 * @param string $file The pdf file - url or file path accepted
 * @param array $info data to use in key/value pairs no more than 2 dimensions
 * @param string $enc default UTF-8, match server output: default_charset in php.ini
 * @return string The XFDF data for acrobat reader to use in the pdf form file
 */
function create_xfdf($file, $info, $enc = 'UTF-8') {
  $data = '<?xml version="1.0" encoding="' . $enc . '"?>' . "\n" .
    '<xfdf xmlns="http://ns.adobe.com/xfdf/" xml:space="preserve">' . "\n" .
    '<fields>' . "\n";
  foreach ($info as $name => $value) {
    $data .= '<field name="' . htmlspecialchars($name) . '"><value>' . htmlspecialchars($value) . '</value></field>' . "\n";
  }
  $data .= '</fields>' . "\n" .
    '<ids original="' . md5($file) . '" modified="' . REQUEST_TIME . '" />' . "\n" .
    '<f href="' . $file . '" />' . "\n" .
    '</xfdf>' . "\n";
  return $data;
}
