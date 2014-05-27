<?php
/**
 * Configuration for "min", the default application built with the Minify
 * library
 * 
 * @package Minify
 */


$min_enableBuilder = false;
$min_builderPassword = 'admin';
$min_errorLogger = false;
$min_allowDebugFlag =true;
$min_cachePath = '/tmp';

//$min_cachePath = preg_replace('/^\\d+;/', '', session_save_path());
/**
 * To use APC/Memcache/ZendPlatform for cache storage, require the class and
 * set $min_cachePath to an instance. Example below:
 */
//require dirname(__FILE__) . '/lib/Minify/Cache/APC.php';
//$min_cachePath = new Minify_Cache_APC();


/**
 * Leave an empty string to use PHP's $_SERVER['DOCUMENT_ROOT'].
 *
 * On some servers, this value may be misconfigured or missing. If so, set this 
 * to your full document root path with no trailing slash.
 * E.g. '/home/accountname/public_html' or 'c:\\xampp\\htdocs'
 *
 * If /min/ is directly inside your document root, just uncomment the 
 * second line. The third line might work on some Apache servers.
 */
$min_documentRoot = '';
//$min_documentRoot = substr(__FILE__, 0, -15);
//$min_documentRoot = $_SERVER['SUBDOMAIN_DOCUMENT_ROOT'];


$min_cacheFileLocking = true;
$min_serveOptions['bubbleCssImports'] = false;
$min_serveOptions['maxAge'] = 1800;



$min_serveOptions['minApp']['groupsOnly'] = true;
$min_symlinks = array(
	'/'.SITEPREFIX => DOCROOT.SITEPREFIX,
);
$min_uploaderHoursBehind = 0;
$min_libPath = dirname(__FILE__) . '/lib';
ini_set('zlib.output_compression', '0');
//$min_serveOptions['rewriteCssUris'] = false;

$min_serveOptions['preserveComments'] = false;