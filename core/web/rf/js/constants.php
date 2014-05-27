<?php 
header("Content-type: text/javascript");
define('public-access', true);

/**
 * Include Files
 */


include(getenv("DOCUMENT_ROOT").'/../rf_config-v2.inc');
include(SYSDIR.'/translation/'.LANGUAGE.'.inc');	/* German words */

$_CONSTANTS =  get_defined_constants();

foreach ($_CONSTANTS as $key => $value) {
	if (stristr($key,'rfstring_')) {
		if (is_bool($value)) {
			echo ("var ".$key." 		= ".($value?'true':'false').";\n");
		}
		else echo ("var ".$key." 		= '$value';\n");
	}
}

?>