/* ROKFOR MAIN JS */

/* Globals */

var tree;							/* Tree Object */
var tree_arch;						/* Archive Tree Object */
var rfgui;							/* Rokfor GUI Object */
var rfcallbacks;					/* Rokfor CALLBACKS Object */
var tabs;							/* nuTabs Object */
var force_upload_stop = false;		/* Upload Meter status: IE Compatibility */

Locale.use('de-CH');

window.addEvent('domready', function() {
	rfgui = new rfGUI();
	rfcallbacks = new rfCALLBACKS();
});
