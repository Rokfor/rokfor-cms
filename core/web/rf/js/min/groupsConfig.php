<?php
/**
 * Groups configuration for default Minify implementation
 * @package Minify
 */

/** 
 * You may wish to use the Minify URI Builder app to suggest
 * changes. http://yourdomain/min/builder/
 *
 * See http://code.google.com/p/minify/wiki/CustomSource for other ideas
 **/

return array(
     'js' => array(
		'/'.SITEPREFIX.'/js/3rdparty/mootools-core.js',
		'/'.SITEPREFIX.'/js/3rdparty/mootools-more.js',
		'/'.SITEPREFIX.'/js/3rdparty/mootree.js',
		'/'.SITEPREFIX.'/js/3rdparty/nutabs.js',
		'/'.SITEPREFIX.'/js/3rdparty/string.utf8.js',
		'/'.SITEPREFIX.'/js/3rdparty/string.md5.js',
		'/'.SITEPREFIX.'/js/3rdparty/Firebug/firebug.js',
		'/'.SITEPREFIX.'/js/interface/rfGUI.js',
		'/'.SITEPREFIX.'/js/interface/rfCALLBACKS.js',
		'/'.SITEPREFIX.'/js/interface/rfOntology.js',
		'/'.SITEPREFIX.'/js/interface/rfTarget.js',
		'/'.SITEPREFIX.'/js/interface/rfCloud.js',
		'/'.SITEPREFIX.'/js/3rdparty/Datepicker/Picker.js',
		'/'.SITEPREFIX.'/js/3rdparty/Datepicker/Picker.Attach.js',
		'/'.SITEPREFIX.'/js/3rdparty/Datepicker/Picker.Date.js',
		'/'.SITEPREFIX.'/js/main.js'
	),
    'css' => array(
		'/'.SITEPREFIX.'/css/font/stylesheet.css',
		'/'.SITEPREFIX.'/css/main/reset.css',
		'/'.SITEPREFIX.'/css/main/master.css',
		'/'.SITEPREFIX.'/css/3rdparty/mootree.css',
		'/'.SITEPREFIX.'/css/interface/alerts.css',
		'/'.SITEPREFIX.'/css/interface/ontology.css',
		'/'.SITEPREFIX.'/css/interface/target.css',
		'/'.SITEPREFIX.'/css/interface/sliders.css',
		'/'.SITEPREFIX.'/css/interface/cloud.css',
		'/'.SITEPREFIX.'/css/3rdparty/datepicker_dashboard/datepicker_dashboard.css'
	),
);





