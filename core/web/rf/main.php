<?php

/**
 * Check Interface
 */

define("CLI", php_sapi_name()=="cli"?true:false);	
define("PID", posix_getpid());	
	
/**
 * Include Files
 */

include(((CLI===true)?$argv[1]:getenv("DOCUMENT_ROOT")).'/../rf_config-v2.inc');
include(SYSDIR.'/rokfor_MAIN.inc');					/* Main Lib */
include(SYSDIR.'/translation/'.LANGUAGE.'.inc');	/* German translation */

/**
 * Create Rokfor Instance
 */

$rfMAIN = new rokfor_MAIN();


/**
 * Command Line Interface: Starting the command line distiller
 */

if (CLI === true) {
	ini_set("max_execution_time", "0");
	ini_set("max_input_time", "0");
	ini_set("memory_limit", "-1");
	set_time_limit(0);
	declare(ticks = 1);
	switch ($argv[4]) {
		case 'pdf':
			$rfMAIN->distiller($argv[0], $argv[1], $argv[2],$argv[3],$argv[4],$argv[5],$argv[6], $argv[7]);	
			break;
		case 'video':
			$rfMAIN->ExecuteVideoConversion($argv[0], $argv[1], $argv[2],$argv[3]);	
			break;
	}

}

/**
 * Web Interface: Presenting the Rokfor Web Engine
 */

else {
	session_start();


	
	// 1. Session Check: Authentification

	$rfMAIN->checkSession();

	// 2. Post Check: Preparing _POST array

	$rfMAIN->postController();

	// 3. Get Check: Preparing _GET array

	$rfMAIN->getController();

	// 4. Create Output

	$rfMAIN->createBuffer();
}

// Print the Results

echo $rfMAIN->buffer;



?>