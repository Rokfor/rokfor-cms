<?php

/**
 * rokfor_MAIN class
 * Main Functions of the Rokfor Interface
 *
 *
 * @package Rokfor
 * @author Urs Hofer
 **/

define("VERSION", "v2.0 beta");
define("VERSION_NR", 2);

include(SYSDIR.'/rokfor_DB.inc');				/* Rokfor Database API */
include(SYSDIR.'/rokfor_TEMPLATES.inc');		/* HTML Templates */
include(SYSDIR."/lib/ffmpeg.inc");

/* Some functions for backwards compatibility */

if(!function_exists('parse_ini_string')){
    function parse_ini_string($str, $ProcessSections=false){
        $lines  = explode("\n", $str);
        $return = Array();
        $inSect = false;
        foreach($lines as $line){
            $line = trim($line);
            if(!$line || $line[0] == "#" || $line[0] == ";")
                continue;
            if($line[0] == "[" && $endIdx = strpos($line, "]")){
                $inSect = substr($line, 1, $endIdx-1);
                continue;
            }
            if(!strpos($line, '=')) // (We don't use "=== false" because value 0 is not valid as well)
                continue;
            
            $tmp = explode("=", $line, 2);
            if($ProcessSections && $inSect)
                $return[$inSect][trim($tmp[0])] = ltrim($tmp[1]);
            else
                $return[trim($tmp[0])] = ltrim($tmp[1]);
        }
        return $return;
    }
}






class rokfor_MAIN
{
	/**
	 * Rokfor Database Class
	 * @var object
	 **/	
	var $rfDB = null;

	/**
	 * Rokfor Template Class
	 * @var object
	 **/	
	var $rfTPL = null;

	/**
	 * Authenticated or not
	 * @var boolean
	 **/	
	var $auth;

	/**
	 * Transport Mode: Ajax/Export
	 * @var string
	 **/
	var $ajax;
	var $api;	

	/**
	 * _GET Parameter: Action
	 * @var string
	 */
	var $action;	

	/**
	 * _GET Parameter: Parameter
	 * @var string
	 */	
	var $parameter;

	/**
	 * _GET Parameter: Values
	 * @var array
	 */	
	var $values;

	/**
	 * _POST Parameter: Main Form Submission
	 * @var array
	 */	
	var $form;
	
	/**
	 * output buffer. contains all html output
	 * @var array
	 */	
	var $buffer;
		
	/**
	 * Constructor
	 *
	 * @author Urs Hofer
	 */
	function rokfor_MAIN () 
	{
		$this->buffer = "";
		$this->rfDB = new rokfor_DB();
		$this->rfTPL = new rokfor_TEMPLATES($this->rfDB,$this->buffer, $this);	
		$this->auth = 0;
		$this->ajax = false;
		$this->api = false;		
		$this->action = false;
		$this->parameter = false;
		$this->values = array();
		
		/**
		 * strips slashes depending on magic_quotes
		 */
		if (get_magic_quotes_gpc()) {
			$_POST['store'] = str_replace('\\', '\\\\', $_POST['store']);
			$_POST['store'] = stripslashes($_POST['store']);
		}
/*
		    $process = array(&$_GET, &$_POST, &$_COOKIE, &$_REQUEST);
		    while (list($key, $val) = each($process)) {
		        foreach ($val as $k => $v) {
		            unset($process[$key][$k]);
		            if (is_array($v)) {
		                $process[$key][stripslashes($k)] = $v;
		                $process[] = &$process[$key][stripslashes($k)];
		            } else {
		                $process[$key][stripslashes($k)] = stripslashes($v);
		            }
		        }
		    }
		    unset($process);
		}
*/

			
	}
	
	function _stripslashes($_str) {
		if (get_magic_quotes_gpc()) {return stripslashes($_str);}
		else return $_str;
	}
	
	/**
	 * returns a browser array
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function getBrowser() 
	{ 
		$u_agent = $_SERVER['HTTP_USER_AGENT']; 
		$ub = 'Unknown';
		$platform = 'Unknown';
		if (preg_match('/linux/i', $u_agent)) $platform = 'linux';
		elseif (preg_match('/macintosh|mac os x/i', $u_agent)) $platform = 'mac';
		elseif (preg_match('/windows|win32/i', $u_agent)) $platform = 'windows';
		if(preg_match('/MSIE/i',$u_agent) && !preg_match('/Opera/i',$u_agent)) $ub = "MSIE"; 
		elseif(preg_match('/Firefox/i',$u_agent)) $ub = "Firefox"; 
		elseif(preg_match('/Chrome/i',$u_agent)) $ub = "Chrome"; 
		elseif(preg_match('/Safari/i',$u_agent)) $ub = "Safari"; 
		elseif(preg_match('/Opera/i',$u_agent)) $ub = "Opera"; 
		elseif(preg_match('/Netscape/i',$u_agent)) $ub = "Netscape"; 
		return array(
		'name'      => $ub,
		'platform'  => $platform,
		);
	}
	
	
	/**
	 * does some mysql maintenance after login
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	private function _mysqlMaintenanceFunction() {
		
		// Check for contributions - chapter consistency
		
		$sql = ("SELECT _templatenames._inchapter as _inchapter,_contributions.id FROM _contributions 
					LEFT JOIN _templatenames ON _templatenames.id = _contributions._fortemplate
					WHERE _contributions.__split__ IS null || _contributions.__split__ = 'Null' || _contributions.__split__ = 'NULL'");
		$ch_c = $this->rfDB->query($sql);
		while ($ch_cc = $this->rfDB->fetch_object($ch_c)) {
			$chId = $this->rfDB->_splitTextListData($ch_cc->_inchapter);
			$sql = ("UPDATE _contributions SET __split__ = '".$chId[0]."' WHERE id = ".$ch_cc->id." LIMIT 1");
			$this->rfDB->query($sql);
			echo "done";
		}		
		

		
		// Updates: Only do them if there is no version mark in the log database
		
		$sql = ("SELECT _user FROM _log 
		WHERE _user LIKE 'AlteredDB<%' 
		ORDER BY _date DESC, id DESC LIMIT 1");
		$logId = $this->rfDB->fetch_object($this->rfDB->query($sql));		
		if ($logId->_user && end(explode('<::::::>',$logId->_user))==VERSION) return;
		
		$sql = ('

		ALTER TABLE  `_data` CHANGE  `_forcontribution`  `_forcontribution` INT( 32 ) NULL DEFAULT NULL ,
		CHANGE  `_fortemplatefield`  `_fortemplatefield` INT( 32 ) NULL DEFAULT NULL		//
		
		ALTER TABLE  `_contributions` CHANGE  `_fortemplate`  `_fortemplate` INT( 32 ) NULL DEFAULT NULL ,
		CHANGE  `_forissue`  `_forissue` INT( 32 ) NULL DEFAULT NULL //
		
		ALTER TABLE  `_templates` CHANGE  `_fortemplate`  `_fortemplate` INT( 32 ) NULL DEFAULT NULL //
		
		ALTER TABLE  `_formats` CHANGE  `_forbook`  `_forbook` INT( 32 ) NULL DEFAULT NULL //
		
		ALTER TABLE  `_issues` CHANGE  `_forbook`  `_forbook` INT( 32 ) NULL DEFAULT NULL //		
		
		DROP TRIGGER IF EXISTS `Delete Data after Contributions Delete`;//
		DROP TRIGGER IF EXISTS `Delete Contributions after Issues Delete`;//
		DROP TRIGGER IF EXISTS `Delete Template Fields after Template Delete`;//
		DROP TRIGGER IF EXISTS `Delete Data after Template Delete`;//
		DROP TRIGGER IF EXISTS `Delete Chapter or Issue after Book Delete`;//

		CREATE TRIGGER `Delete Data after Contributions Delete` AFTER DELETE ON `_contributions`
		FOR EACH ROW BEGIN
		DELETE FROM _data WHERE _forcontribution = OLD.id;
		END//

		CREATE TRIGGER `Delete Contributions after Issues Delete` AFTER DELETE ON `_issues`
		FOR EACH ROW BEGIN
		DELETE FROM _contributions WHERE _forissue = OLD.id;
		END//

		CREATE TRIGGER `Delete Template Fields after Template Delete` AFTER DELETE ON `_templatenames`
		FOR EACH ROW BEGIN
		DELETE FROM _templates WHERE _fortemplate = OLD.id;
		END//

		CREATE TRIGGER `Delete Data after Template Delete` AFTER DELETE ON `_templates`
		FOR EACH ROW BEGIN
		DELETE FROM _data WHERE _fortemplatefield = OLD.id;
		END//

		CREATE TRIGGER `Delete Chapter or Issue after Book Delete` AFTER DELETE ON `_books`
		FOR EACH ROW BEGIN
		DELETE FROM _formats WHERE _forbook = OLD.id;
		DELETE FROM _issues WHERE _forbook = OLD.id;
		END//

		ALTER TABLE  `users` CHANGE  `id`  `id` INT( 32 ) NOT NULL AUTO_INCREMENT//		

		ALTER TABLE  `users` ADD PRIMARY KEY(`id`)		
		');
		foreach (explode('//', $sql) as $value) {
			$this->rfDB->query($value);
		}
		$this->_log("AlteredDB",VERSION);
	}
	
	/**
	 * escaping a string for use as a xml parameter
	 *
	 * @param string $_string 
	 * @return string
	 * @author Urs Hofer
	 */
	private function _xmlescape($_string)
	{
		return htmlspecialchars(html_entity_decode(strip_tags($_string)), ENT_QUOTES);
	}
	
	/**
	 * _pcre_error_decode
	 * decode regular expression errors and returns a fully qualified string
	 *
	 * @return string
	 * @author Urs Hofer
	 */
	function _pcre_error_decode() {
	    switch (preg_last_error()) {
	        case PREG_NO_ERROR:
	            return "pcre_error: PREG_NO_ERROR!\n";
	            break;
	        case PREG_INTERNAL_ERROR:
	            return "pcre_error: PREG_INTERNAL_ERROR!\n";
	            break;
	        case PREG_BACKTRACK_LIMIT_ERROR:
	            return "pcre_error: PREG_BACKTRACK_LIMIT_ERROR!\n";
	            break;
	        case PREG_RECURSION_LIMIT_ERROR:
	            return "pcre_error: PREG_RECURSION_LIMIT_ERROR!\n";
	            break;
	        case PREG_BAD_UTF8_ERROR:
	            return "pcre_error: PREG_BAD_UTF8_ERROR!\n";
	            break;
	        case PREG_BAD_UTF8_OFFSET_ERROR:
	            return "pcre_error: PREG_BAD_UTF8_OFFSET_ERROR!\n";
	            break;
	    }
	}
	
	/**
	 * checkSession: 1 (ok), 0 (not ok), -1 (expired)
	 *
	 * @return int
	 * @author Urs Hofer
	 **/
	function checkSession() 
	{
		if (array_key_exists('control', $_GET) && in_array($_GET['control'], array('Login','Logout','Expired'))) {
			if ($this->auth = $this->rfDB->newSession($_POST['uname'],$_POST['pass']))
			{
				$_SESSION['sessionid'] = $this->rfDB->getSessionId();
				$this->_mysqlMaintenanceFunction(); // Do once after Login some mySQL Maintenance checks.
				$this->updateSessionVars(false);
				$this->rfTPL->redirect('/');
			}
			else 
			{
				$this->rfDB->cleanSessionId($_SESSION['sessionid']);								
				unset($_SESSION['sessionid']);
				$this->auth = 0;
				if ($_GET['control']=='Login') $this->authmsg = PleaseLogin;
				if ($_GET['control']=='Logout') $this->authmsg = LoggedOut;
				if ($_GET['control']=='Expired') $this->authmsg = SessionExpired;								
			}
			return;
		}
		
		// If we reached this point, this is the normal check
		$this->auth = $this->rfDB->checkSession($_SESSION['sessionid'],($_GET['action']=='OtherUsers')?(true):(false));
		// If the user is admin or root disable access right checks
		if ($this->rfDB->getUserGroup()=="root" || $this->rfDB->getUserGroup()=="admin") $this->rfDB->setUserRestriction(false);
		else $this->rfDB->setUserRestriction(true);		
	}
	
	/**
	 * shows all users logged in currently
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function showOtherUsers() 
	{
		$sql = "SELECT users.username, session.starttime, session.currenttime FROM session
				LEFT JOIN users ON session.userid = users.id WHERE users.id <> ".$this->rfDB->getUserId();
		$_q = $this->rfDB->query($sql);
		while ($user = $this->rfDB->fetch_object($_q)) {
			$this->buffer .= $user->username." [".Since." ".(round((time()-$user->currenttime)/60))." ".Minutes."] ";
		}
	}
	
	
	
	/**
	 * postController: do some action with the post data
	 *
	 * @return void
	 * @author Urs Hofer
	 **/
	function postController() 
	{
		// Set after Search Result
		if (array_key_exists('searchstring', $_POST) && $_POST['searchstring']) {
			$this->rfTPL->searchstring = json_decode($this->_stripslashes($_POST['searchstring']));
		}
		
		// Custom Controls can be used to send individual POST Data per Field. They will be transmitted into the template class
		if (array_key_exists('customControl', $_POST) && is_array($_POST['customControl'])) {
			$this->rfTPL->customcontrol = $_POST['customControl'];
		}
		
		
		// Store Mainform Post Data in Class Array
		$this->form = array 
		(
			"text" => array_key_exists('text', $_POST)?$_POST['text']:array(),
			"caption" => array_key_exists('caption', $_POST)?$_POST['caption']:array(),
			"files" => is_array($_FILES)?$_FILES:array(),
			"existing_files" =>  array_key_exists('existing_files', $_POST)?$_POST['existing_files']:array(),
			"table" => array_key_exists('table', $_POST)?$_POST['table']:array(),
			"number" => array_key_exists('number', $_POST)?$_POST['number']:array(),											
		);
		

		
	}
	
	/**
	 * getController: Translate Get-Parameters to Class Variables. Follows this syntax:
	 * Ajax|Regular/Action/Parameter/Values[]
	 *
	 * @return void
	 * @author Urs Hofer
	 **/
	function getController () 
	{
		/* Store Values in Class Array */	
		if (array_key_exists('values', $_GET) && is_array($_GET['values'])) $this->values = $_GET['values'];
		
		/* Switch Ajax Mode On/Off */
		if (array_key_exists('mode', $_GET) && $_GET['mode']=='Ajax') $this->ajax = true;
		if (array_key_exists('mode', $_GET) && $_GET['mode']=='Api') $this->api = true;

		/* Store Action in Class Vars */
		if (array_key_exists('action', $_GET) && $_GET['action']) $this->action = $_GET['action'];

		/* Store Parameter in Class Vars */
		if (array_key_exists('parameter', $_GET) && $_GET['parameter']) $this->parameter = $_GET['parameter'];	
		
		/* Modify Parameter: Convert "Latest" to latest added Contribution	 */
		if ($this->parameter=="Latest") {
			$this->parameter = $this->getLatestContrib();
		}		

		/* Modify Parameter: Convert "Lastmodified" to latest modified Contribution	 */
		if ($this->parameter=="Lastmodified") {
			$this->parameter = $this->getLastmodifiedContrib();
		}		

		/* Modify Parameter: Convert "Next/Previous" to latest added Contribution	 */
		
/*		if ($this->parameter=="Next"||$this->parameter=="Previous") {
			$this->parameter = $this->getNeighbourContrib($this->parameter,$this->values[0]);
		}
*/
	}
	
	/**
	 * returns the Order stored in the logfile for an edition.
	 * if no config value is found, false is returned.
	 * false will trigger the default order (by id asc)
	 *
	 * @param string $editionId 
	 * @return void
	 * @author Urs Hofer
	 */
	function getOrder($bookId) {
		$sql = ("SELECT * FROM _log 
		WHERE _log._user LIKE 'Order:".$bookId."<%' 
		AND (_log.__user__ = '".$this->rfDB->getUserName()."' || _log.__user__ = '".$this->rfDB->getUserId()."')
		ORDER BY _log._date DESC, _log.id DESC LIMIT 1");
		$logId = @$this->rfDB->fetch_object(@$this->rfDB->query($sql));
		if ($logId->_user) preg_match('/:>(.*)/',$logId->_user,$matches);
		if ($matches[1]<>""&&$logId->_user) return $matches[1];		
		else false;
	}
	
	/**
	 * stores the order of an edition in the logfile
	 *
	 * @param int $editionId 
	 * @param string $field (id, _newdate, _moddate, _name)
	 * @param string $order (asc, desc)
	 * @return void
	 * @author Urs Hofer
	 */
	function storeOrder($bookId,$field, $order) {
		$field = trim($field);
		$order = trim($order);
		if (!in_array($field,array('id','_newdate','_moddate','_name','__sort__')) || !in_array($order,array('ASC','DESC'))) return false;
		$this->_log("Order:$bookId","$field $order");
		$this->buffer .= "ok";
	}

	/**
	 * checking if there are children in the tree with only one containing element
	 *
	 * @param string $_args 
	 * @return void
	 * @author Urs Hofer
	 */
	function _checkChildren($_args, $_mode) {
		$_hiddenlevel = ' hiddenlevel="'; $_item = '';
		if ($_args[0]) {
			if ($_editions = $this->rfDB->getEditions($_args[0],false,($_mode=='Current')?('open'):('closed'))) {
				if (count($_editions)==1) {
					$_args[1] = $_editions[0]->id;
					$_hiddenlevel .= '3,';
					$_item .= ' editionid="'.$_editions[0]->id.'" ';
				}
			}
		}
		if ($_args[1]) {
			if ($_parts=$this->rfDB->getParts($_args[0])) {
				if (count($_parts)==1) {
					$_args[2] = $_parts[0]->id;
					$_hiddenlevel .= '4,';
					$_item .= ' chapterid="'.$_parts[0]->id.'" ';
				}
			}
		}

		$_hiddenlevel .= '"'.$_item;
		return $_hiddenlevel;		
		
	}
	
	/**
	 * treeXML: Putting XML Code for mootree together, store active project state (open book/issue/chapter/contrib)
	 *
	 * @param string $_mode ("Current" "Archive")
	 * @param int $_level (1:Book 2:Issue 3:Chapter 4: Contributions)
	 * @param int $_choice (1:Book 2:Issue 3:Chapter 4: Contributions)	
	 * @return string
	 * @author Urs Hofer
	 */
	function treeXML($_mode,$_args = null) {

		$retval = "<?xml version=\"1.0\"?>\n<nodes>\n";

		if ($_args[0]) {
			$this->rfDB->setProjectActive($_args[0]);
			if ($_editions = $this->rfDB->getEditions(false,false,($_mode=='Current')?('open'):('closed'))) {
				if (count($_editions)==1) {
					$_args[1] = $_editions[0]->id;
				}
			}
		}
		if ($_args[1]) {
			$this->rfDB->setEditionActive($_args[1]);
			if ($_parts=$this->rfDB->getParts()) {
				if (count($_parts)==1) {
					$_args[2] = $_parts[0]->id;
				}
			}
		}
		if ($_args[2]) {
			$this->rfDB->setPartActive($_args[2]);
		}

		
		switch (count($_args)) {
			case 0:
				/* Load Projects */
				if ($_projects = $this->rfDB->getProjects()) foreach ($_projects as $_value) {
					$_hiddenlevel = $this->_checkChildren(array($_value->id),$_mode);
					$retval .= "\t<node $_hiddenlevel level=\"".(count($_args)+2)."\" icon=\"./img/3rdparty/mt/archives.png\" projectid=\"".$_value->id."\" text=\"$_level ".$this->_xmlescape($_value->_name)."\" load=\"/rf/Ajax/Tree/".$_mode."/".$_value->id."\" />\n";
				}
				break;
			case 1:
				/* Load Issues */
				if ($_editions = $this->rfDB->getEditions(false,false,($_mode=='Current')?('open'):('closed'))) foreach ($_editions as $_value) {
					$_hiddenlevel = $this->_checkChildren(array($_args[0], $_value->id),$_mode);
					$retval .= "\t<node $_hiddenlevel level=\"".(count($_args)+2)."\" icon=\"./img/3rdparty/mt/document-library.png\" projectid=\"".$_args[0]."\" editionid=\"".$_value->id."\" text=\"".$this->_xmlescape($_value->_name)."\" load=\"/rf/Ajax/Tree/".$_mode."/".$_args[0]."/".$_value->id."\" />\n";
				}				
				break;
			case 2:
				/* Load Chapters */
				if ($_parts=$this->rfDB->getParts()) foreach ($_parts as $_value) {
					$retval .= "\t<node level=\"".(count($_args)+2)."\" icon=\"./img/3rdparty/mt/issue.png\" projectid=\"".$_args[0]."\" editionid=\"".$_args[1]."\" chapterid=\"".$_value->id."\" text=\"".$this->_xmlescape($_value->_name)."\" load=\"/rf/Ajax/Tree/".$_mode."/".$_args[0]."/".$_args[1]."/".$_value->id."\" />\n";
				}
				break;
			case 3:
				$_entries = array();		
		
				// State according to user
				$_state = "Open";	// User: Only Open Contribs
				if ($this->rfDB->getUserGroup() == 'admin' || (defined("SHOWCLOSEDCONTRIB") && SHOWCLOSEDCONTRIB===true)) $_state = 'Open" || _contributions._status = "Close';	// Admin: Open and Closed
				if ($this->rfDB->getUserGroup() == 'root' || ((defined("SHOWCLOSEDCONTRIB") && SHOWCLOSEDCONTRIB===true) && (defined("SHOWDELETEDCONTRIB") && SHOWDELETEDCONTRIB===true))) $_state = false;	// Root: all (incl. deleted)
				
				// Order according to configuration
				$_order = $this->getOrder($this->rfDB->activeProject->id);
				
				if ($_elements = $this->rfDB->getElements(false, $_state, false, false, $_order)) {
					foreach ($_elements as $_value) {
						$_unsorted .= $_entries[$this->rfDB->getUserNameById($_value->__user__)][] = "\t\t<node level=\"".(count($_args)+2)."\" icon=\"./img/3rdparty/mt/project.png\" text=\"".$this->_xmlescape($_value->_name)."\" state=\"".$_value->_status."\" cssclass=\"".((array_key_exists('addcontribution', $_SESSION) && $_SESSION['addcontribution']==$_value->id)?('mooTree_recentlyadded" preselected="true'):($_value->_status))."\" contribid=\"".$_value->id."\" />\n";
					}				
					if (!defined("HIDEUSER") || HIDEUSER===false) foreach ($_entries as $_key => $_value) {
						$retval .= "\t<node icon=\"./img/3rdparty/mt/user.png\" text=\"User: ".$_key."\" open=\"true\">\n";
						$retval .= implode("",$_value);
						$retval .= "\t</node>\n";					
					}
					else $retval .= $_unsorted;
				}
				break;			

			default:
				# code...
				break;
		}
		// Unset Addcontribution: Mark only once after a new contribution is added the entry in the xml tree.
		unset ($_SESSION['addcontribution']);
		$retval .= ('</nodes>');
		return $retval;
	}
	
	/**
	 * get the contribution next or previous in the xml list
	 * obsolete!
	 * @param string $action (Next / Previous)
	 * @return void
	 * @author Urs Hofer
	 */
/*	function getNeighbourContrib($action,$currentId) {
		$_state = "Open";	// User: Only Open Contribs
		if ($this->rfDB->getUserGroup() == 'root' || $this->rfDB->getUserGroup() == 'admin') $_state .= '" || _contributions._status = "Close';

		$_order = $this->getOrder($_SESSION['bookid']);	
		if (stristr($_order,'_moddate')) {
			if ($this->action=="Form") $this->rfTPL->_domready('
				rfgui.rfAlert(this,rfstring_SORTERROR);
			');
			return	$currentId;
		}
		$_next = $_before = $currentId;
		if ($_elements = $this->rfDB->getElements($_SESSION['chapterid'], $_state, $_SESSION['issueid'], false, $_order)) {
			foreach ($_elements as $_key=>$_value) {
				if ($currentId==$_value->id) {
					if ($_elements[$_key+1]->id) $_next = $_elements[$_key+1]->id;
					break;
				}
				$_before = $_value->id;
			}				
		}
//		echo ("$_before < $currentId > $_next ($_order)");
		return $_SESSION['addcontribution']=($action=="Next"?$_next:$_before);
	}
*/	
	/**
	 * Updating the session Variables, if no parameter is passed, unset the current contrib
	 *
	 * @param object $_currentContrib 
	 * @return void
	 * @author Urs Hofer
	 */
	function updateSessionVars($_currentContrib=false)
	{
		if (!$_currentContrib) {
			unset($_SESSION['contribid']);
			return;
		}
		$_SESSION['contribid'] 	= $_currentContrib->id;
		$_SESSION['issueid'] 	= $_currentContrib->_forissue;		

		$_SESSION['templateid'] = $_currentContrib->_fortemplate;	
		$_SESSION['templatename'] = $this->rfDB->getTemplateNamebyId($_currentContrib->id)->_name;

		$_SESSION['issuename'] 	= $this->rfDB->getEditionNamebyId($_currentContrib->id)->_name;
		
		$_SESSION['chapterid'] 	= $_currentContrib->__split__;
		$_SESSION['chaptername'] = $this->rfDB->getPartNamebyId($_currentContrib->id)->_name;

		$_SESSION['bookname'] = $this->rfDB->getProjectNamebyPartId($_currentContrib->__split__)->_name;
		$_SESSION['bookid']	  = $this->rfDB->getProjectNamebyPartId($_currentContrib->__split__)->id;
	}
	
	/**
	 * logs actions into the log-db.
	 *
	 * @param string $key 
	 * @param string $value 
	 * @param string $user 
	 * @return void
	 * @author Urs Hofer
	 */
	function _log($key,$value) {
		$sql = ("
			INSERT INTO `_log` (`id`, `_ip`, `_agent`, `_user`, `_date`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
			('', '".GETENV("REMOTE_ADDR")."', '".GETENV("HTTP_USER_AGENT")."', '".mysql_real_escape_string($key.'<::::::>'.$value)."', '".time()."', '".$this->rfDB->getUserId()."', NULL, NULL, -1, 1);
		");
		$this->rfDB->query($sql);
		$logId = $this->rfDB->insert_id();
		$sql = ("
			UPDATE  `_log` SET  `__sort__` =  '$logId' WHERE  `_log`.`id` =$logId LIMIT 1 ;
		");
		$this->rfDB->query($sql);		
	}
	
	
	/**
	 * echoing a javascript alert
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function _alert($s)
	{
		echo '<script>alert(\''.str_replace("'", "\'", $s).'\');</script>';
	}
	

	/**
	 * _callFieldPostProcessor calls the post processor function for a field stored in the database
	 * it dies if an error occured in the executed code
	 * returns false if there's no code to execute for a field
	 * 
	 * Postprocessor have to return the same data structure as they get passed.
	 * - For Images: a string (filename)
	 * - For Integers: a int
	 * - For Text-Fields: a string or an array
	 * 
	 * 
	 *
	 * @param string $dataId: Field ID
	 * @return mixed $data (processed or unprocessed if there's no callback code stored)
	 * @author Urs Hofer
	 */
	function _callFieldPostProcessor($dataId, $data) {
		$_field_info_object = $this->rfDB->getFieldData($dataId);

		/* Compatibility to old callbacks: */
		$_field_info["_fortemplatefield"] 	= $_field_info_object->d_fortemplatefield;
		$_field_info["_fieldname"] 			= $_field_info_object->t_fieldname;
		$_field_info["_fieldtype"] 			= $_field_info_object->t_fieldtype;
		$_field_info["_colNames"] 			= $_field_info_object->t_colnames;
		$_field_info["_contribId"] 			= $_field_info_object->c_id;
		
		/* Prepare Data for Tables &  */
		if (stristr($data, '<::::::>') && stristr($data, '<;;;;;;>'))
			$data_processd = $this->rfDB->_splitTableData($data);
		else if  (stristr($data, $_glue='<;;;;;;>'))
			$data_processd = $this->rfDB->_splitWordListData($data);
		else if  (stristr($data, $_glue='<::::::>'))
			$data_processd = $this->rfDB->_splitTextListData($data);
		else 
			$data_processd = $data;
		
		// Postprocessing callbacks stored in the database...
		$_fnct_data = array();
		$name_neu = false;						
		$sql_cb = "SELECT * FROM _fieldpostprocessor WHERE (_forfield LIKE '%:>".$_field_info_object->d_fortemplatefield."<:%' OR _forfield LIKE '".$_field_info_object->d_fortemplatefield."<:%') LIMIT 1";
		$_fnct_data = $this->rfDB->fetch_object($this->rfDB->query($sql_cb));



		if ($_fnct_data->_code<>"") {
			if (!function_exists('_postprocessing_callback'.$dataId)) {
				$_fnct = '<?php function _postprocessing_callback'.$dataId.'($db,$data,$_field_info, $rfDB, $_field_info_as_object)
				{
				'.$this->_stripslashes($_fnct_data->_code).'
				return($data);
				} ?>';
				$temp_db = tempnam("/tmp", "rokfor_post_");
				file_put_contents($temp_db, $_fnct);
				require($temp_db);
				unlink($temp_db);
			}
			$data_processd = call_user_func('_postprocessing_callback'.$dataId, $this->rfDB->db, $data_processd,$_field_info, $this->rfDB, $_field_info_object);
			if ($data_processd !== false) {
				if (is_array($data_processd)) {
					if (is_array($data_processd[0])) return $this->rfDB->_glueTableData($data_processd);
					else return (implode($_glue,$data_processd));
				}
				else {
					return ($data_processd);
				}
			}
			else {
				die(CallbackExecutionError);
			}
		}
		return $data;
	}	
	

	/**
	 * br2nl - stripes out <br[/]> tags
	 *
	 * @param string $string 
	 * @return void
	 * @author Urs Hofer
	 */
	function _br2nl($text)
	{
	    /* Remove regular line breaks and tabs. */
	    $text = str_replace(array("\r", "\r\n", "\n", "\t"),"",$text);
	    /* Convert XHTML and conventional linebreak <br>, <br/> etc. tags to carriage return */
		$text = preg_replace('/<br[^>]*>/ ', "\r", $text);
	    /* Remove Opening Paragraphs. */
		$text = preg_replace('/<p[^>]*>/', "", $text);
	    /* Replace closing paragraphs with newlines. */
		$text = preg_replace('/<\/p>/', "\n", $text);
		// Replace last </p> - since it is not actually a new line...
		if (substr($text, -1)=="\n") $text = substr($text, 0, -1);
		/* Return the result. */
	    return $text;
	}

	/**
	 * cleans a string from html data and specific codes which should not be part of the string in the database
	 *
	 * @param string $fielddata 
	 * @return string
	 * @author Urs Hofer
	 */
	function _clean_from_rteditor($fielddata, $trim = false) {
		// Return immediatly if suppressed by config:
		if (defined('STORE_HTML') && STORE_HTML) {
			$fielddata = preg_replace('/<span[^>]*>/m', '', $fielddata);
			$fielddata = str_replace('</span>', '', $fielddata);			
			return $fielddata;
		}
		
		// Convert BR to NL
		$fielddata = $this->_br2nl($fielddata);
		// Preserve the holy delimiters
		$fielddata = str_replace(array('<::::::>','<;;;;;;>','<>'), array('?-::::::-?','?-;;;;;;-?','?--?'), $fielddata);
		//Preserve all custom signs
		$fielddata = str_replace(array('<fn--', '--fn>', '<!--', '--!>'), array('?fn--','--fn?','?!--','--!?'), $fielddata);
		// Clean out mistakes
		$fielddata = str_replace(array('&lt;fn--', '--fn&gt;', '&lt;!--', '--!&gt;'), array('?fn--','--fn?','?!--','--!?'), $fielddata);
		// Convert Page Break Div to <!--pagebreak!-->
		$fielddata = preg_replace('/<div(.*?)<\/div>/m', '?!--pagebreak--!?', $fielddata);


		// Delete all Tags but:
		$allowable = '<u><i><b><blockquote><ul><li><ol><a><img><hr><iframe><h5><h6>';
		$fielddata = strip_tags($fielddata, $allowable);
		// Clear Non Blocking Spaces
		$fielddata = str_replace('&nbsp;', ' ', $fielddata);  
		// Save without HTML Entities
		$fielddata = html_entity_decode($fielddata, ENT_COMPAT, 'UTF-8');
		// Restore the holy delimiters
		$fielddata = str_replace(array('?-::::::-?','?-;;;;;;-?','?--?'), array('<::::::>','<;;;;;;>','<>'), $fielddata);
		//restore all custom signs
		$fielddata = str_replace(array('?fn--','--fn?','?!--','--!?'),array('<fn--', '--fn>', '<!--', '--!>'), $fielddata);
		
		if ($trim)
			return trim($fielddata);
		else
			return $fielddata;

	}
	

	/**
	 * executing batch function stored in the database. batchpost is an associative array
	 *
	 * @param array $validation 
	 * @param array $batchpost 
	 * @return void
	 * @author Urs Hofer
	 */
	function batchProcessorPost() {
		$ret = true;  
		$sql = 'SELECT * FROM _batch WHERE id =  '.$_POST["function_id"].' LIMIT 1';
		$all = $this->rfDB->query($sql);			
		$this->buffer = '<b>'.RunBatch.' '.$_POST["function_id"].'</b>';
		$fncts = $this->rfDB->fetch_object($all);
		$temp_db = tempnam("/tmp", "rokfor_dbconf_");
		$fnc_head = '<?php function batch_postprocessing'.$_POST["function_id"].'($_db,$_rokforDB,$issueid,$bookid,$batchid,$batchpost, $contribs, $templates, $_caller) {';
		$fnc_foot = '} ?>';			
		file_put_contents($temp_db, $fnc_head.$this->_stripslashes($fncts->_postcode).$fnc_foot);
		require($temp_db);
		unlink($temp_db);
		$_contribs = $this->rfDB->getElements('_contributions.__split__', false, $_POST["issue_id"]);
		$_templates = $this->rfDB->getTemplates($_POST["book_id"],"all");
		$olderror=error_reporting(E_ERROR);
		$_callback = call_user_func("batch_postprocessing".$_POST["function_id"], $this->rfDB->db,$this->rfDB,$_POST["issue_id"],$_POST["book_id"],$_POST["function_id"],$_POST, $_contribs, $_templates, $this);
		error_reporting($olderror);
		if ($_callback===false) {
			$_err = error_get_last();
			$_callback = BatchPostError.": ".$_err['message'].' on line '.$_err['line'];       
			$ret = false;  
	    }
		$this->buffer .= '<div class="batchcallback">'.$_callback.'</div>';		
		return $ret;			
	}

	/**
	 * executing search replace. analyzing the post array:
	 * 
	 * 	Array ( 
	 *		[search] => Ich 
	 *		[replace] => Ich(?)Bin 
	 * 		[searchrange] => issue 
	 *		[issues] => Array ( [0] => 2 ) 
	 *		[fieldlist] => Array ( [0] => 2 ) 
	 *		[replaceaction] => 0 
	 *	) 
	 * 
	 * 
	 * @return void
	 * @author Urs Hofer
	 */
	function searchreplaceResults() {
		// Preliminiary Checks
		if ((stristr($_POST['search'],'<::::::>')) 
			|| (stristr($_POST['search'],'<;;;;;;>')) 
			|| (stristr($_POST['replace'],'<::::::>')) 
			|| (stristr($_POST['replace'],'<;;;;;;>'))
			|| (trim(str_replace(array('*','?'), '', $_POST['search']))=='' && $_POST['regex']==='true')
			|| (!$_POST['search'])
			|| (($_POST['searchrange']=='issue') && !is_array($_POST['issues']))
			) return NotValidRegexp;
		
		
		// Preparing contributions

		if ($_POST['searchrange']=='issue') {
			foreach ($_POST['issues'] as $_issueid) {
				foreach ($this->rfDB->getElements('_contributions.__split__', 'Open', $_issueid) as $_contribs) {
					$_contribtosearch[] = $_contribs->id;
					$_bookname[$_contribs->id] = $this->rfDB->getProjectNamebyEditionId($_issueid)->_name;
					$_issuename[$_contribs->id] = $this->rfDB->getEditionInfo($_issueid)->_name;
				}
			}
		}
		else if ($_POST['searchrange']=='all') {
			foreach ($this->rfDB->getProjects(false) as $_book) {
				$_eds = $this->rfDB->getEditions($_book->id);
				if ($_eds) foreach ($_eds as $_issue) {
					foreach ($this->rfDB->getElements('_contributions.__split__', 'Open', $_issue->id) as $_contribs) {
						$_contribtosearch[] = $_contribs->id;
						$_bookname[$_contribs->id] = $_book->_name;
						$_issuename[$_contribs->id] = $_issue->_name;
					}
				}
			}
		}		
		else $_contribtosearch = array((int)$_POST['searchrange']);

		// Collecting data fields
		$_allfields = false;
		$_fieldstosearch = array();		
		if ($_POST['fieldlist'][0]==0) $_allfields = true;
		else $_fieldstosearch = $_POST['fieldlist'];

		// Prepare Regexp
		if ($_POST['regex']==='true') {
			$_escape = array('*','?');
			$_escape_save = array('˜%%%˜','˜%˜%˜');
			$_POST['search'] = str_replace($_escape, $_escape_save, $_POST['search']);
		}
		$_regexp = '/'.(preg_quote($_POST['search'], '/')).'/';
		if ($_POST['regex']==='true') {
			$_regexp = str_replace($_escape_save, array('.*?','.'),$_regexp);
		}


		// Case Sensitivity
		if ($_POST['case']!=='true') $_regexp .= 'i';

		// Cycle thru the fields. Do a replace
		$_found = array();
		$o_e = error_reporting(E_ERROR);
		foreach ($_contribtosearch as $_contribs) {
			$_found[$_contribs] = 0;
			foreach ($this->rfDB->getFieldsByElement($_contribs) as $_field) {
				if (($_allfields || in_array($_field->t_id, $_fieldstosearch))) { 

					# code...$_datatext = str_replace($searchstring, '<span comment="rokfor_temp" style="background-color: #ffff00;">'.$searchstring.'</span comment="rokfor_temp">', $_datatext);
					$_c = 0;
					
					if ($_field->_fieldtype == "Bild") $_r = preg_replace($_regexp, $_POST['replace'], $_field->d_binary, -1, $_c);	// Substitute
					else $_r = preg_replace($_regexp, $_POST['replace'], $_field->d_text, -1, $_c);	// Substitute
					if ($_r===NULL) {
						$this->buffer .= NotValidRegexp."<br/>Error Code: ".($this->_pcre_error_decode())."<br/>".CheckSyntax.": <b>$_regexp</b><br/>";
						return;
					}
					if ($_POST['replaceaction']==1) $this->rfDB->setFieldData($_r,$_field->id,$_field->_fieldtype == "Bild"?"_databinary":"_datatext");									// Store
					$_found[$_contribs] += $_c;											// Add counts

				}
			}
		}

		error_reporting($o_e);
		
		// Post action: create list
		arsort($_found);
		$_list = array(); $noresults = true;
		foreach ($_found as $_found => $_count) if ($_count) $_list[$_bookname[$_found]][$_issuename[$_found]][$_found] =  $_count;
		foreach ($_list as $_bookname => $_bookcontent) {
			$noresults = false;
			foreach ($_bookcontent as $_issuename => $_issuecontent) {
				foreach ($_issuecontent as $_contribid => $_count) {
				if ($_POST['searchrange']=='issue' || $_POST['searchrange']=='all') {
					if ($_bookname<>$_lastbook) {
						if ($_lastbook) $this->buffer .= '</div>';		
						$this->buffer .= '<a class="bookresult button green wide">'.($_lastbook=$_bookname).'</a><div class="toggle">';		
					}
					if ($_issuename<>$_lastissue) {
						$this->buffer .= '<p class="issue">'.($_lastissue=$_issuename).'</p>';		
						$this->buffer .= '<p><span style="float:right">'.($_POST['replaceaction']==1?Substitutes:Hits).'</span>'.Resultate.'</p>';		
					}					
				}
				else {
					$this->buffer .= '<p><span style="float:right">'.($_POST['replaceaction']==1?Substitutes:Hits).'</span>'.Resultate.'</p>';
				}
				$this->buffer .= '<p><span style="float:right">'.$_count.'</span><a class="searchresultlinks" id="searchlink'.$_contribid.'" href="/'.$_contribid.'">'.($this->rfDB->getElementsNamebyId($_contribid)->_name).'</a></p>';		
				}
			}
		}					
		if ($_POST['searchrange']=='issue'||$_POST['searchrange']=='all') $this->buffer .= '</div>';	
		if ($noresults) $this->buffer .= '<p>'.NoResults.'</p>';

		$this->rfTPL->_domready("
		$$('.bookresult').addEvent('click', function(e){
			rfcallbacks.toggleSytemtab(this);
		});
		
		$$('.searchresultlinks').addEvent('click', function(e){
			var postdata = 'searchstring=".($_POST['replaceaction']==1?json_encode(preg_quote('/'.$_POST['replace'].'/', '/')):json_encode($_regexp))."';
			e.preventDefault();
			e.stop();
			if (rfgui.intro) {
				rfcallbacks.hideIntro();
				rfgui._ajaxtransport('/Context'+this.get('href'),$('context'));						
			}
			rfgui._ajaxtransport('/Form'+this.get('href'),$('mainform'),postdata);
		});
		");
		
		if ($_POST['searchrange']<>'issue') {
			$this->rfTPL->_domready("			
				$('searchlink".$_contribid."').fireEvent('click');			
			");
		}
		
		
		// Final Action: searchrange <> issue - reload contribution
		// either: marked or replaced.
		
	}
	
	/**
	 * Stores the content of the main form in the database
	 * The output is stored in a hidden iframe (multipart forms can only be submitted, not send by xhr)
	 *
	 * @param object $_currentContrib 
	 * @return void
	 * @author Urs Hofer
	 */
	function storeData($_currentContrib)
	{
		$this->_log("Store",$_currentContrib->id);		
		$sql = ("UPDATE `_contributions` SET  `_moddate` =  '".time()."' WHERE  `_contributions`.`id` =".$_currentContrib->id." LIMIT 1;");				
		$this->rfDB->query($sql);		
	
		/* Store Table Data */
		if (count($this->form["table"])>0) {
			foreach ($this->form["table"] as $_id=>$_data) {
				// Call Postprocessors stored in Database
				$_data = $this->_callFieldPostProcessor($_id,$_data);
				// Save and update 
				$this->rfDB->setFieldData($_data,$_id,'_datatext');
			}
		}
		
		/* Store Integer Data */
		if (count($this->form["number"])>0) {
			foreach ($this->form["number"] as $_id=>$_data) {
				if (is_array($_data)) $tstamp = mktime ($_data[4], $_data[5], 0, $_data[2], $_data[1], $_data[3]);
				else $tstamp = $_data;
				// Call Postprocessor
				$tstamp = $this->_callFieldPostProcessor($_id,intval($tstamp));
				$this->rfDB->setFieldData($tstamp,$_id,'_datainteger');
			}
		}		
		
		/* Store Binary Data */
		$img_exception = false;
		foreach ($this->form['caption'] as $_id=>$_captions) {
			/* Load current dataset */
			$_field_info = $this->rfDB->getFieldData($_id);
			$_current_data = $this->rfDB->_splitTableData($_field_info->d_binary);
			$_store_data = array();
						
			foreach ($_captions as $_imgId=>$_caption) {
				$_filename = "";
				// Keep existing File
				if ($this->form['existing_files'][$_id][$_imgId]!="added"&&$this->form['files']['data']['error'][$_id][$_imgId] != UPLOAD_ERR_OK) {
					$_filename = $_current_data[$this->form['existing_files'][$_id][$_imgId]][1];
					$_parts = explode(".", $_filename);
					$_test = array_shift($_parts);
					foreach ($_parts as $part) {
						$_test .= ".".$part;
						if (is_file(REPOSITORYDIR."/".$_test)) {
							$_filename = $_test;
							break;
						}
					}
					$_current_data[$this->form['existing_files'][$_id][$_imgId]][1] = "";
				}
				// Take New File
				else if ($this->form['files']['data']['error'][$_id][$_imgId] == UPLOAD_ERR_OK) {


					# code 0: Store File or overwrite old file at this position
					if (!is_dir(REPOSITORYDIR.'/thumbs')) {
						if (!mkdir (REPOSITORYDIR.'/thumbs'))  die ("Could not create thumbnail directory.");
					}
					$_tmpname = $this->form['files']["data"]["tmp_name"][$_id][$_imgId];
					$_filename = preg_replace('/[^\w\.]/','', $this->form['files']["data"]["name"][$_id][$_imgId]);

					if (defined("OVERWRITE_UPLOADS_WITH_SAMENAME") || OVERWRITE_UPLOADS_WITH_SAMENAME===true) {
						$_filename = $_currentContrib->id."_".$_filename;
						// If File exists, delete it from the current array, otherwise it will be deleted at the end.
						if (file_exists(REPOSITORYDIR.'/'.$_filename)) {
							$_current_data[$this->form['existing_files'][$_id][$_imgId]][1] = "";							
						}
					}
					else {
						if (file_exists(REPOSITORYDIR.'/'.$_filename)) $_filename = time().$_filename;	// If there's a file like this, add timestamp to the name
					}
			        if (!move_uploaded_file($_tmpname, REPOSITORYDIR."/$_filename")) die ("Could not move uploaded file.");
				}
				/* filename has to be set, either loaded or already existing... */
				if ($_filename) {
					$this->buffer .= (REPOSITORYDIR."/$_filename");							
					# call callbacks
					$_ret = false;
					$_ret = $this->_callFieldPostProcessor($_id,REPOSITORYDIR."/$_filename");
					if ($_ret && $_ret <> REPOSITORYDIR."/$_filename") {
						$_filename = str_replace(REPOSITORYDIR."/", "", $_ret);
					}
					if (!$_mime = trim(shell_exec('file -bi '.escapeshellarg(REPOSITORYDIR."/$_filename")))) {
						$_finfo = finfo_open(FILEINFO_MIME_TYPE);
						$_mime = finfo_file($_finfo, REPOSITORYDIR."/$_filename");
						finfo_close($_finfo);
					}
					if (stristr($_mime, 'image')||stristr($_mime, 'pdf')) {
						# create scaled down version (if set in field settings or alternatively
						# in the globals - otherwise just store a jpg copy in the original resolution
						try {
							$imagick = new Imagick(REPOSITORYDIR."/$_filename"."[0]");
						} catch (Exception $e) {
							$this->_alert('Imagick Error: ' . $e->getMessage() . "\n");
							$img_exception = true;							
							continue;
						}							
						$imagick->setBackgroundColor('white'); 
                        $imagick->setImageCompression(Imagick::COMPRESSION_JPEG);
                        $imagick->setImageCompressionQuality(defined("JPEG_COMPRESSION")?JPEG_COMPRESSION:95);
//						$imagick->profileImage ("*", NULL);
						$imagick->setImageFormat("jpg");		
						if ($imagick->getImageColorspace() == Imagick::COLORSPACE_CMYK) {
							if (stristr($_mime, 'pdf')) {
								$imagick->setImageColorspace(Imagick::COLORSPACE_RGB);
								$imagick->negateImage(false);
							}										
							else {
							    $profiles = $imagick->getImageProfiles('*', false);
							    // we're only interested if ICC profile(s) exist
							    $has_icc_profile = (array_search('icc', $profiles) !== false);
							    // if it doesnt have a CMYK ICC profile, we add one
							    if ($has_icc_profile === false) {
							        $icc_cmyk = file_get_contents(SYSDIR.'/lib/USWebUncoated.icc');
							        $imagick->profileImage('icc', $icc_cmyk);
							        unset($icc_cmyk);
							    }
							    // then we add an RGB profile
							    $icc_rgb = file_get_contents(SYSDIR.'/lib/sRGB_v4_ICC_preference.icc');
							    $imagick->profileImage('icc', $icc_rgb);
							    unset($icc_rgb);
							}
						}
						$imagick->stripImage();


			
						/* Prepare some values */
						$_wA = $_hA = array();
					
						if (stristr($_field_info->t_imagewidth, ";") ||	stristr($_field_info->t_imageheight, ";")) {
							$_wA = explode(";", $_field_info->t_imagewidth);
							$_hA = explode(";", $_field_info->t_imageheight);										
						}
						else if ($_field_info->t_imagewidth>0 || $_field_info->t_imageheight>0) {
							$_wA[0] = $_field_info->t_imagewidth; 
							$_hA[0] = $_field_info->t_imageheight;										
						}
						else if (defined("PREVIEW_WIDTH")&&defined("PREVIEW_HEIGHT")) {
							$_wA[0] = PREVIEW_WIDTH; 
							$_hA[0] = PREVIEW_HEIGHT;																				
						}

						$_written = false;
						foreach ((count($_wA)>=count($_hA))?($_wA):($_hA) as $_key => $_value) {
							$__w = 0; $__h = 0;									
							$__w = $_wA[$_key]?$_wA[$_key]:0; 
							$__h = $_hA[$_key]?$_hA[$_key]:0; 

							$_process = $imagick->clone(); 
						

							if ($__w>0 || $__h>0) {
								/* Beide Seiten definiert Nur die grössere als Referenz nehmen */
								if ($__w>0 && $__h>0) {
									$_is_w = $_process->getImageWidth()/$_process->getImageHeight()>$__w/$__h?true:false;
									$_process->thumbnailImage ($_is_w?$__w:0, $_is_w?0:$__h);
								}
								/* eine ist null */
								else $_process->thumbnailImage ($__w, $__h);
							}
							$_process->writeImage (REPOSITORYDIR."/".$_filename."-preview".(($_key>0)?($_key):('')).".jpg");
							$_written = true;
						}							
						/* Create small Rokfor Thumbnail anyway and a jpg copy of the input file if no clone is created so far */
						if (!$_written) {
							try {
								$imagick->writeImage (REPOSITORYDIR."/".$_filename."-preview.jpg");								
							} catch (Exception $e) {
								$this->_alert('Imagick Error: ' . $e->getMessage() . "\n");
								$img_exception = true;								
								continue;							
							}
						}
						$imagick->thumbnailImage (150, 150, true);
						try {
							$imagick->writeImage (REPOSITORYDIR."/thumbs/".$_filename."-thmb.jpg");							
						} catch (Exception $e) {
							$this->_alert('Imagick Error: ' . $e->getMessage() . "\n");
							$img_exception = true;							
							continue;							
						}
					}
					if ($_filename) $_store_data[] = array($_caption, $_filename);
				}
			}
			if (!$img_exception)
				$this->rfDB->setFieldData($this->rfDB->_glueTableData($_store_data),$_id,'_databinary');			

			/* Delete unused Files */
			foreach ($_current_data as $_todel) {
				if (is_file(REPOSITORYDIR.'/'.$_todel[1])) unlink(REPOSITORYDIR.'/'.$_todel[1]); /* Original */
				if (is_file(REPOSITORYDIR.'/'.$_todel[1]."-preview.jpg")) {
					foreach (glob(REPOSITORYDIR.'/'.$_todel[1]."-preview*") as $filename) {
					   unlink($filename); /* Jpg Previews */
					}
				}
				if (is_file(REPOSITORYDIR.'/thumbs/'.$_todel[1]."-thmb.jpg")) unlink(REPOSITORYDIR.'/thumbs/'.$_todel[1]."-thmb.jpg");	/* Jpg Preview */								
			}
		}
		
		/* Store Text Data */
		if (count($this->form["text"])>0) {
			foreach ($this->form["text"] as $_id=>$_data) {
				$_field_info = $this->rfDB->getFieldData($_id);

				/* Multi Line */
				if (is_array($_data)) {
					if (in_array("-1", $_data) && $_field_info->t_fieldtype=="TypologySelect") $_data = "-1";	// If in a select box -1 is selected, all other values are automatically deselected...
					else $_data = join($_data, "<::::::>");
				}
				else {
					/* Clean HTML Markup if RTF Editor (Text Field only)*/
					$_rtfeditor_disabled = false;
					if (stristr($_field_info->t_colnames, 'rtfeditor=false') || $_field_info->t_fieldtype<>"Text") $_rtfeditor_disabled = true;
					if ((!$_rtfeditor_disabled&&defined("EDITOR"))) {
						$_data = $this->_clean_from_rteditor($_data);						
					}
				}
				// Call Postprocessors stored in Database
				$_data = $this->_callFieldPostProcessor($_id,$_data);
				// Save and update 
				$this->rfDB->setFieldData($_data,$_id,'_datatext');
			}
		}
		$this->buffer .= "<body onload=\"parent.window.force_upload_stop = true;\">stored</body>";
		
	}
	
	/**
	 * Prints the upload percentage as json value for further processing via javascript
	 *
	 * @param string $_progressKey 
	 * @return void
	 * @author Urs Hofer
	 */
	function getProgress($_progressKey) {
	  	$status = apc_fetch('upload_'.$_progressKey);	
		$this->buffer .= json_encode(round(($status['total']>0)?(300/$status['total']*$status['current']):(0)));
	}

	/**
	 * deletes a subitem of a binary field
	 *
	 * @param int $fieldId 
	 * @param int $imgId 
	 * @return void
	 * @author Urs Hofer
	 */
	function deleteFromBinary($fieldId,$imgId=false) {
		$_field_info = $this->rfDB->getFieldData($fieldId);
 		if ($imgId===false) {
			$udb_update = "";
			$deletedArray = explode("<;;;;;;>", $_field_info->d_binary);
		}
		else {
			$udb_oldnames = explode("<;;;;;;>", $_field_info->d_binary);
			$deletedArray = array_splice($udb_oldnames, $imgId, 1);
			$udb_update = implode ("<;;;;;;>", $udb_oldnames);
		}

		foreach ($deletedArray as $toDelete) {
			$_filename = end(explode("<::::::>",$toDelete));
			if (is_file(REPOSITORYDIR.'/'.$_filename)) unlink(REPOSITORYDIR.'/'.$_filename); /* Original */
			if (is_file(REPOSITORYDIR.'/'.$_filename."-preview.jpg")) {
				foreach (glob(REPOSITORYDIR.'/'.$_filename."-preview*") as $filename) {
				   unlink($filename); /* Jpg Previews */
				}
			}
			if (is_file(REPOSITORYDIR.'/thumbs/'.$_filename."-thmb.jpg")) unlink(REPOSITORYDIR.'/thumbs/'.$_filename."-thmb.jpg");	/* Jpg Preview */								
		}


		$this->rfDB->setFieldData($udb_update,$fieldId,'_databinary');
	}
	
	/**
	 * undocumented function
	 *
	 * @param int $_issueid
	 * @param int $_chapterid
	 * @param string $_contribname 
	 * @param int $_templateid 
	 * @return void
	 * @author Urs Hofer
	 */
	function newContribution($_issueid, $_chapterid, $_contribname, $_templateid) {

		if (!$_contribname || !$_templateid || !$_issueid || !$_chapterid) return false;

		$initTime= time();
		// Insert into Contributions
		$sql = ("
			INSERT INTO `_contributions` (`id`, `_fortemplate`, `_forissue`, `_name`, `_status`, `_newdate`, `_moddate`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
			('', '$_templateid', '$_issueid', '".mysql_real_escape_string($_contribname)."', 'Open', $initTime, $initTime, '".$this->rfDB->getUserId()."', NULL, '$_chapterid', -1, 1);
		");
		$this->rfDB->query($sql);
		$newId = $this->rfDB->insert_id();
		$sql = ("
			UPDATE  `_contributions` SET  `__sort__` =  '$newId' WHERE  `_contributions`.`id` =$newId LIMIT 1 ;
		");
		$this->rfDB->query($sql);
		
		// Prepare Cell Fields in Datatable...
		$_fields = $this->rfDB->getTemplateFields($_templateid);
		
//		print_r($_fields);

		$parentNode = 0;
		foreach ($_fields as $fields) {

			// 2. Insert Fields in the Datatable
			$sql_I = ("			
				INSERT INTO `_data` (`id`, `_forcontribution`, `_fortemplatefield`, `_datatext`, `_databinary`, `_datainteger`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
				('', '$newId', '".$fields->id."', NULL, NULL, NULL, '".$this->rfDB->getUserId()."', NULL, NULL, -2, $parentNode);
			");
			$this->rfDB->query($sql_I);
			$parentNode++;
			// $parentNode = $newId_I = $this->rfDB->insert_id();
			//$sql_I = ("UPDATE  `_data` SET  `__sort__` =  '$newId_I' WHERE  `_data`.`id` =$newId_I LIMIT 1;");				
			//$this->rfDB->query($sql_I);
		}
		// Set newContribution Session Var: $_SESSION['addcontribution']. XML Output will automatically select this node with
		// the css class mooTree_selected. So a XML reload in the tree will visually mark the new contribution
		$_SESSION['lastTemplate'] = $_templateid;
		return ($_SESSION['addcontribution'] = $newId);
	}
	
	/**
	 * set the state of a contribution to "Close" (default) or $action
	 *
	 * @param int $id 
	 * @return void
	 * @author Urs Hofer
	 */
	function closeContribution($contribId, $action = 'Close') {
		if ($contribId == "multi") foreach ($_POST['data'] as $contribId) {
				$sql = ("UPDATE  `_contributions` SET  `_status` =  '$action' WHERE  `_contributions`.`id` =$contribId LIMIT 1 ;");
				$this->rfDB->query($sql);
		}
		else {
			$sql = ("UPDATE  `_contributions` SET  `_status` =  '$action' WHERE  `_contributions`.`id` =$contribId LIMIT 1 ;");
			$this->rfDB->query($sql);
		}
		$this->buffer .= "ok";

	}				
	
	/**
	 * deletes a contribution and all its data
	 *
	 * @param int $id 
	 * @return void
	 * @author Urs Hofer
	 */
	function trashContribution($contribId) {
		// Delete Binaries
		$_contribs = array();
		if ($contribId == "multi") $_contribs = $_POST['data'];
		else $_contribs[0] = $contribId;

		foreach ($_contribs as $_contribId) {
			$sql = ("SELECT id from _data WHERE _forcontribution = '".$_contribId."' and _databinary <> '' ");
			while ($todel = $this->rfDB->fetch_object($this->rfDB->query($sql))) {
				$this->buffer .= "deleting binaries: ".$todel->id;
				$this->deleteFromBinary($todel->id);
			}
			// Delete Data
			$sql = ("DELETE from _data WHERE _forcontribution = '".$_contribId."'");
			$this->rfDB->query($sql);
			// Delete Contribution
			$sql = ("DELETE from _contributions WHERE id = '".$_contribId."'");
			$this->rfDB->query($sql);
		}

		$this->buffer .= "ok";
	}				
	
	/**
	 * clones the data from a contribution to the actual one
	 *
	 * @param string $contribId 
	 * @param string $fromId 
	 * @return void
	 * @author Urs Hofer
	 */
	function cloneData($contribId,$fromId) {
		$sql = ("SELECT * from _data WHERE _forcontribution = ".$fromId);
		$oldData = $this->rfDB->query($sql);
		while ($oldVals = $this->rfDB->fetch_object($oldData)) {
			
			/* TODO: Clone binary data: Upload, Thumb und alle -preview[x].jpg */
			$_binary_clone = "";
			if ($oldVals->_databinary) {
				$image_processed = $this->rfDB->_splitTableData($oldVals->_databinary);
				foreach ($image_processed as &$urls) {
					if (is_file(REPOSITORYDIR.'/'.$urls[1]))  {
						$marker = time();
						/* Copy Files */
						copy(REPOSITORYDIR.'/'.$urls[1], REPOSITORYDIR.'/'.$marker.$urls[1]);
						if (is_file(REPOSITORYDIR.'/thumbs/'.$urls[1]."-thmb.jpg"))
							copy(REPOSITORYDIR.'/thumbs/'.$urls[1]."-thmb.jpg", REPOSITORYDIR.'/thumbs/'.$marker.$urls[1]."-thmb.jpg");						
						if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview.jpg"))
							copy(REPOSITORYDIR.'/'.$urls[1]."-preview.jpg", REPOSITORYDIR.'/'.$marker.$urls[1]."-preview.jpg");						
						$preview_counter = 1;
						while (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview".$preview_counter.".jpg")) {
							copy(REPOSITORYDIR.'/'.$urls[1]."-preview".$preview_counter.".jpg", REPOSITORYDIR.'/'.$marker.$urls[1]."-preview".$preview_counter.".jpg");						
							$preview_counter++;
						}
						/* Store new url in referenced array */
						$urls[1] = $marker.$urls[1];
					}
				}
				$_binary_clone = $this->rfDB->_glueTableData($image_processed);
			}			
			
			$sql_I = ("			
				UPDATE `_data` 
				SET `_datatext`='".mysql_real_escape_string($oldVals->_datatext)."', `_datainteger` = '".$oldVals->_datainteger."' , `_databinary` = '".$_binary_clone."'
				WHERE _fortemplatefield = ".$oldVals->_fortemplatefield." AND _forcontribution = $contribId;
			");
			$this->rfDB->query($sql_I);
		}		
		$this->buffer .= "ok";
	}
	
	function cloneContribution($_contrib,$newName) {
		if (!$newName) return false;
		$newId = $this->newContribution($_contrib->_forissue, $_contrib->__split__, $newName, $_contrib->_fortemplate);
		$this->cloneData($newId,$_contrib->id);
		$this->buffer .= "ok $newId";
		return $newId;
	}
	
	function renameContribution($contribId,$newName) {
		if ($contribId<>$_SESSION['contribid']) return false;	// Something bad happened: the function is not called by the current open contribution
		$sql = ("
			UPDATE  `_contributions` SET  `_name` =  '".mysql_real_escape_string($newName)."' WHERE  `id` =$contribId LIMIT 1 ;
		");
		$this->rfDB->query($sql);				
		$this->buffer .= "ok";		
	}
	
	/**
	 * returns the id of the last added contribution of a user
	 *
	 * @return int
	 * @author Urs Hofer
	 */
	function getLatestContrib() {
		$sql = ("
			SELECT MAX(id) as max FROM _contributions WHERE __user__ = '".$this->rfDB->getUserName()."' || __user__ = '".$this->rfDB->getUserId()."'
		");
		$logId = $this->rfDB->fetch_object($this->rfDB->query($sql));
		return $_SESSION['addcontribution']=$logId->max;
	}
	
	/**
	 * returns the id of the last modified contribution according to the log file.
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function getLastmodifiedContrib() {
		$sql = ("SELECT * FROM _log 
		LEFT JOIN _contributions ON _log._user LIKE CONCAT('%>',CAST(_contributions.id AS CHAR))
		WHERE _log._user LIKE 'Store<%' 
		AND (_log.__user__ = '".$this->rfDB->getUserName()."' || _log.__user__ = '".$this->rfDB->getUserId()."')
		AND _contributions.id IS NOT NULL 
		ORDER BY _log._date DESC, _log.id DESC LIMIT 1");
		$logId = $this->rfDB->fetch_object($this->rfDB->query($sql));
		preg_match('/:>(.*)/',$logId->_user,$matches);
		return $_SESSION['addcontribution']=$matches[1];		
	}
	
	/**
	 * checks the field datas of a contribution: right number of fields, right order,
	 * right field names. adds missing fields or deletes unused fields-
	 *
	 * @param object $_contrib 
	 * @return void
	 * @author Urs Hofer
	 */
	function checkContribution($_contrib) {
		$_templateFields = $this->rfDB->getTemplateFields($_contrib->_fortemplate);
		$_dataFields = $this->rfDB->getFieldsByElement($_contrib->id, false);

		$_reSorted = $_hasChanged = $_hasAdded = $_hasDeleted = 0;



		// Cycle thru list of necessary fields defined in the template
		// Check if the field already exists but on wrong position		
		// Reuse as many old fields as possible
		foreach ($_templateFields as $_key=>$_templateField) {
			foreach ($_dataFields as $_dkey=>$_dataField) {
				if ($_dataField->t_id == $_templateField->id) {
					$this->rfDB->query("UPDATE  `_data` SET  `__sort__` =  '".$_key."' WHERE  `_data`.`id` = ".$_dataField->id." LIMIT 1 ;");			
					unset($_dataFields[$_dkey]);
					unset($_templateFields[$_key]);					
					$_reSorted++;
					break;
				}
			}
		}
		// Cycle thru remaining template fields
		// Either add take an existing field or add a new one
		foreach ($_templateFields as $_key=>$_templateField) {
				// Change exsiting fields to new template relations
				if ($_existingData = reset($_dataFields)) {
					$this->rfDB->query("UPDATE  `_data` SET  `_fortemplatefield` =  '".$_templateField->id."' WHERE  `_data`.`id` = ".$_existingData->id." LIMIT 1 ;");			
					$_hasChanged++;
					unset($_dataFields[key($_dataFields)]);
				}
				// or add a field into the db...
				else {
					$sql_I = ("			
						INSERT INTO `_data` (`id`, `_forcontribution`, `_fortemplatefield`, `_datatext`, `_databinary`, `_datainteger`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
						('', '".$_contrib->id."', '".$_templateField->id."', NULL, NULL, NULL, '".$this->rfDB->getUserId()."', NULL, NULL, -2, '".$_key."');
					");
					$this->rfDB->query($sql_I);
					$_hasAdded++;					
				}

		}

		// Remaining old fields can be deleted. Only used when changing a template.
		// Deleting Template Fields delete all corresponding data fields via sql triggers
		foreach ($_dataFields as $_dataField) {
			if ($_dataField->d_binary) $this->deleteFromBinary($_dataField->id);				
			$this->rfDB->query("DELETE from _data WHERE id = ".$_dataField->id." LIMIT 1");				
			$_hasDeleted++;
		}
		return (array($_reSorted,$_hasChanged,$_hasAdded,$_hasDeleted));
	}
	
	/**
	 * changes the template in the contribution.
	 *
	 * @param object $_contrib 
	 * @param int $_newTemplateId 
	 * @return void
	 * @author Urs Hofer
	 */
	function changeTemplate($_contrib,$_newTemplateId) {
		// Update Contributions
		$sql = ("UPDATE  `_contributions` SET  `_fortemplate` =  '$_newTemplateId' WHERE  `_contributions`.`id` =".$_contrib->id." LIMIT 1;");
		$this->rfDB->query($sql);
		$_contrib->_fortemplate = $_newTemplateId;			
		$this->checkContribution($_contrib);
	}
	
	/**
	 * transforming the old form field id's to new ones for the save as function
	 *
	 * @param int $contribId 
	 * @return void
	 * @author Urs Hofer
	 */
	function transformFormFields($contribId) {
		foreach($this->form as $_type=>$_fields) {
			if (is_array($_fields)) {
				if ($_type<>"files") foreach($_fields as $_fieldId=>$_content) {
					$_oldInfo = $this->rfDB->getFieldData($_fieldId);
					$sql = "SELECT id FROM _data WHERE _fortemplatefield = ".$_oldInfo->d_fortemplatefield." AND _forcontribution = ".$contribId;
					$_newfieldId = @$this->rfDB->fetch_object(@$this->rfDB->query($sql));				
					$this->form[$_type][$_newfieldId->id] = $_content;
					unset($this->form[$_type][$_fieldId]);
				}
				else if (is_array($_fields['data']['error'])) foreach($_fields['data']['error'] as $_fieldId=>$_content){
					$_oldInfo = $this->rfDB->getFieldData($_fieldId);
					$sql = "SELECT id FROM _data WHERE _fortemplatefield = ".$_oldInfo->d_fortemplatefield." AND _forcontribution = ".$contribId;
					$_newfieldId = @$this->rfDB->fetch_object(@$this->rfDB->query($sql));
					$this->form[$_type]['data']['error'][$_newfieldId->id] 		= $_content;
					$this->form[$_type]['data']['name'][$_newfieldId->id]		= $this->form[$_type]['data']['name'][$_fieldId];
					$this->form[$_type]['data']['type'][$_newfieldId->id]		= $this->form[$_type]['data']['type'][$_fieldId];
					$this->form[$_type]['data']['tmp_name'][$_newfieldId->id]	= $this->form[$_type]['data']['tmp_name'][$_fieldId];
					$this->form[$_type]['data']['size'][$_newfieldId->id]		= $this->form[$_type]['data']['size'][$_fieldId];	
					unset($this->form[$_type]['data']['error'][$_fieldId]);
					unset($this->form[$_type]['data']['name'][$_fieldId]);
					unset($this->form[$_type]['data']['type'][$_fieldId]);
					unset($this->form[$_type]['data']['tmp_name'][$_fieldId]);
					unset($this->form[$_type]['data']['size'][$_fieldId]);	
				}
			}			
		}
	}
	
	/**
	 * changes the state of an issue (edition)
	 *
	 * @param int $_issueid 
	 * @param string $_state (open/closed)
	 * @return void
	 * @author Urs Hofer
	 */
	function setIssueState($_issueid,$_state) {
		$sql = ("
			update _issues set _status='$_state' where id=$_issueid
		");
		$this->rfDB->query($sql);		
		$this->buffer .= "ok";
	}
	
	/**
	 * clone an Issue with the same properties, but a new name
	 *
	 * @param int $_issueid 
	 * @param string $_newname 
	 * @return void
	 * @author Urs Hofer
	 */
	function cloneIssue($_issueid,$_newname) {
		$sql = ("
			INSERT INTO _issues ( _name, _opendate, _closedate, _status, _infotext, _forbook, _singleplugin, _allplugin, _rtfplugin, _xmlplugin, _narrationplugin, __user__, __config__, __split__, __parentnode__, __sort__)			
			SELECT _name, _opendate, _closedate, _status, _infotext, _forbook, _singleplugin, _allplugin, _rtfplugin, _xmlplugin, _narrationplugin, __user__, __config__, __split__, __parentnode__, __sort__
			FROM _issues WHERE id = $_issueid
		");
		$this->rfDB->query($sql);	
		$_newid = $this->rfDB->insert_id();
		$sql = ("
			UPDATE  _issues SET  __sort__ =  $_newid, _name = '$_newname' WHERE  id = $_newid LIMIT 1 ;
		");
		$this->rfDB->query($sql);		
		
		if ($_elements = $this->rfDB->getElementsByIssue($_issueid)) {
			foreach ($_elements as $_value) {
				if ($_value->id) {
					$_contrib = $this->rfDB->getElementsNamebyId($_value->id);
					$_contrib->_forissue = $_newid;
					$this->cloneContribution($_contrib,$_contrib->_name);
				}
			}
		}		
		
		

		$this->buffer .= "ok";	
	}
	
	/**
	 * stores the settings in the system tag
	 * $_POST['store'] contains the array of a popup table. Rows with $_POST['store'][0] == "added" are new.
	 * They contain in the last index the id of the cloned row.
	 *
	 * @param string $mode 
	 * @param int $id		(has multiple purposes, general id of the parent data, e.g. _forbook id)
	 * @return void
	 * @author Urs Hofer
	 */
	function storeSystemSettings($_mode,$_id=false) {
		switch ($_mode) {
			/* Checks all Contributions for Field order / Field Count */
			case 'DataConsistency':
				$this->buffer .= '<div class="alertScrollContent">'.Changes.':<br/>';
				$sql = ("SELECT id from _templatenames");
				$_q = $this->rfDB->query($sql);
				while ($row = $this->rfDB->fetch_object($_q)) {
					$this->rfDB->getTemplateFields($row->id);
				}

				$sql = ("SELECT id from _contributions");
				$_q = $this->rfDB->query($sql);
				while ($row = $this->rfDB->fetch_object($_q)) {
					$_contrib = $this->rfDB->getElementsNamebyId($row->id);			
					$_r = $this->checkContribution($_contrib);
					if ($_r[1]||$_r[2]||$_r[3]) {
						$this->buffer .= $_contrib->_name."[".$_contrib->id."]:<br/>Kpt ".$_r[0]."/ Chg ".$_r[1]."/ Add ".$_r[2]."/ Del ".$_r[3]."<br/>";
					}
				}
				$this->buffer .= "</div>";
				break;
			/* Delete all Data without contribution or template field, delete all contributions without template */
			case 'CleanOrphans':
				$this->buffer .= '<div class="alertScrollContent">'.Changes.':<br/>';
				$this->buffer .= '<p>'.Fields.": ";
				$sql = ("SELECT _data.id FROM _data
				LEFT JOIN _contributions ON _data._forcontribution = _contributions.id
				WHERE _contributions.id IS NULL");
				$_q = $this->rfDB->query($sql);
				while ($row = $this->rfDB->fetch_object($_q)) {
						$sql = ("DELETE FROM _data WHERE `id` = ".$row->id." LIMIT 1"); 
						$this->rfDB->query($sql);
						$this->buffer .= $row->id." / ";
				}
				
				$this->buffer .= '</p><p>'.Contributions.": ";
				$sql = ("SELECT _contributions.id FROM _contributions
				LEFT JOIN _issues ON _contributions._forissue = _issues.id
				LEFT JOIN _templatenames ON _contributions._fortemplate = _templatenames.id				
				WHERE _issues.id IS NULL || _templatenames.id IS NULL");
				$_q = $this->rfDB->query($sql);
				while ($row = $this->rfDB->fetch_object($_q)) {
						$sql = ("DELETE FROM _contributions WHERE `id` = ".$row->id." LIMIT 1"); 
						$this->rfDB->query($sql);
						$this->buffer .= $row->id." / ";
				}				
				
				$this->buffer .= "</p></div>";
				break;				
			/* Delete all Contributions with Status <> Open && Closed */
			case 'EmptyTrash':
				$this->buffer .= '<div class="alertScrollContent">'.Changes.':<br/>';
				$sql = ("DELETE FROM _contributions WHERE _status <> 'Open' && _status <> 'Close'"); 
				$this->buffer .= '<p>'.Deleted.": ";
				$this->buffer .= $this->rfDB->affected_rows($this->rfDB->query($sql));
				$this->buffer .= "</p></div>";
				break;				
			/* Stores the sort order form */
			case 'Storesort':
 				foreach ($_POST['ordermode_book'] as $key => $value) {
					if ($this->getOrder($key) <> $_POST['orderby_book'][$key]." ".$value) {
						$this->storeOrder($key,$_POST['orderby_book'][$key], $value);
					}
				}
				break;
			/* Modifies a book */
	 		case 'addBook':
				$sql = ("UPDATE _books SET `__sort__` = -1"); $this->rfDB->query($sql);						
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _books (`_name`, `__user__`, `__parentnode__`, `__sort__`) VALUES ('".mysql_real_escape_string($_row[1])."', '".$this->rfDB->getUserId()."', '-1', ".$__sort__.")");
						$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _books SET `_name` = '".mysql_real_escape_string($_row[1])."', `__sort__` =  ".$__sort__." WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}
				}
				$sql = ("DELETE FROM _books WHERE `__sort__` = -1"); $this->rfDB->query($sql);
				// Recursive delete: Done with triggers.
			  	break;
			/* Modifies a chapter */
	 		case 'addChapter':
				$sql = ("UPDATE _formats SET `__sort__` = -1 WHERE _forbook = ".$_id); $this->rfDB->query($sql);
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _formats (`_name`, `_forbook`, `__user__`, `__parentnode__`, `__sort__`) VALUES ('".mysql_real_escape_string($_row[1])."', ".$_id.", '".$this->rfDB->getUserId()."', '-1', ".$__sort__.")");
						$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _formats SET `_name` = '".mysql_real_escape_string($_row[1])."', `__sort__` =  ".$__sort__." WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}			
				}
				$sql = ("DELETE FROM _formats WHERE `__sort__` = -1 && _forbook = ".$_id); $this->rfDB->query($sql);
				// TODO: Recursive delete!

			  	break;
			/* Stores the contributions of a chapter */
	 		case 'sortChapter':
	
				$_data = $this->rfDB->_splitTableData($_POST['store']);
				
				/* Reverse Order if DESC */
				if (stristr($this->getOrder($this->rfDB->getProjectNamebyEditionId($_id)->id), 'desc')) 
				{
					$_data = array_reverse($_data);
				}
	
				foreach ($_data as $__sort__=>$_row) {
					$sql = ("UPDATE _contributions SET  `__sort__` =  ".$__sort__." WHERE id = ".$_row[0]." LIMIT 1");
					$this->rfDB->query($sql);
				}
			  	break;			
			/* Modifies issues */			
			case 'addIssue':
				$sql = ("UPDATE _issues SET `__sort__` = -1 WHERE _forbook = ".$_id); $this->rfDB->query($sql);
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					$_plugins = array();
					foreach (explode('<:>',$_row[4]) as $_plugs) {
						list($_key,$_plugid) = explode('-',$_plugs);
						$_plugins[$_key] .= $_plugid."<::::::>";
					}
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _issues (`_name`, `_opendate`, `_closedate`, `_status`, `__config__`, `_infotext`, `_forbook`, `_singleplugin`,`_allplugin`,`_rtfplugin`,`_xmlplugin`,`_narrationplugin`,`__user__`, `__parentnode__`, `__sort__`) 
 						VALUES ('".mysql_real_escape_string($_row[1])."', ".time().", ".strtotime($_row[3]).", '".$_row[5]."', '".$_row[6]."', '".mysql_real_escape_string($_row[2])."',".$_id.", '".$_plugins['_singleplugin']."', '".$_plugins['_allplugin']."', '".$_plugins['_rtfplugin']."', '".$_plugins['_xmlplugin']."', '".$_plugins['_narrationplugin']."', '".$this->rfDB->getUserId()."', '-1', ".$__sort__.")");
						$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _issues SET 
							`_name` = '".mysql_real_escape_string($_row[1])."',
							`__sort__` =  ".$__sort__.",
							`_closedate` =  '".strtotime($_row[3])."',
							`_status` =  '".$_row[5]."',
							`__config__` =  '".$_row[6]."',
							`_infotext` =  '".mysql_real_escape_string($_row[2])."',
							`_singleplugin` =  '".$_plugins['_singleplugin']."',
							`_allplugin` =  '".$_plugins['_allplugin']."',
							`_rtfplugin` =  '".$_plugins['_rtfplugin']."',
							`_xmlplugin` =  '".$_plugins['_xmlplugin']."',
							`_narrationplugin` =  '".$_plugins['_narrationplugin']."'
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}			
				}
				$sql = ("DELETE FROM _issues WHERE `__sort__` = -1 && _forbook = ".$_id); $this->rfDB->query($sql);
				// Recursive Deletes are done with MySQL Triggers
				break;	
			/* Stores the template lists */							
			case 'Listtemplates':
				$sql = ("UPDATE _templatenames SET `__sort__` = -1"); $this->rfDB->query($sql);
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					$_chapters = $_books = "";
					foreach (explode('<:>',$_row[5]) as $_chapt) {
						list($_key,$_chapterid) = explode('-',$_chapt);
						$_chapters .= $_chapterid."<::::::>";
					}
					foreach (explode('<:>',$_row[6]) as $_book) {
						list($_key,$_booksid) = explode('-',$_book);
						$_books .= $_booksid."<::::::>";
					}
										
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _templatenames (_name, _helptext, _helpimage, _inchapter, _forbook, _category, _public, __user__, __parentnode__ , __sort__) 
 						VALUES ('".mysql_real_escape_string($_row[1])."','".mysql_real_escape_string($_row[2])."','".mysql_real_escape_string($_row[3])."','".$_chapters."','".$_books."', '".mysql_real_escape_string($_row[4])."','".mysql_real_escape_string($_row[7])."', '".$this->rfDB->getUserId()."', '-1', ".$__sort__.")");
						$this->rfDB->query($sql);
						/* Clone Fields for the New Template */
						$_newTemplateId = $this->rfDB->insert_id();
						if ($_oldTemplateId = $_row[8]) {
						
							$sql = ("
								INSERT INTO _templates (
									`_fortemplate`, 
									`_fieldname`, 
									`_helpdescription`, 
									`_helpimage`, 
									`_fieldtype`, 
									`_maxlines`, 
									`_textlength`, 
									`_imagewidth`,
									`_imageheight`,
									`_cols`,
									`_colNames`,
									`_history`,
									`_growing`,
									`_lengthInfluence`,
									`__user__`, 
									`__parentnode__`, 
									`__sort__`)
								SELECT 
									$_newTemplateId, 
									`_fieldname`, 
									`_helpdescription`, 
									`_helpimage`, 
									`_fieldtype`, 
									`_maxlines`, 
									`_textlength`, 
									`_imagewidth`,
									`_imageheight`,
									`_cols`,
									`_colNames`,
									`_history`,
									`_growing`,
									`_lengthInfluence`,
									`__user__`, 
									`__parentnode__`, 
									`__sort__`
								FROM _templates WHERE _fortemplate = $_oldTemplateId
							");

							$this->rfDB->query($sql);	
						}
						
						
					} else {
						$sql = ("UPDATE _templatenames SET 
							`_name` = '".mysql_real_escape_string($_row[1])."',
							`__sort__` =  ".$__sort__.",
							`_helptext` =  '".mysql_real_escape_string($_row[2])."',
							`_helpimage` =  '".mysql_real_escape_string($_row[3])."',
							`_inchapter` =  '".$_chapters."',
							`_forbook` =  '".$_books."',
							`_category` =  '".mysql_real_escape_string($_row[4])."',
							`_public` =  '".mysql_real_escape_string($_row[7])."'
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}			
				}
				$sql = ("DELETE FROM _templatenames WHERE `__sort__` = -1"); $this->rfDB->query($sql);
				$this->buffer .= $this->rfTPL->templateSorting(false);
				break;
			/* Stores the fields of a template */				
			case 'Listfields':
				$sql = ("UPDATE _templates SET `__sort__` = -1 WHERE _fortemplate = ".$_id); $this->rfDB->query($sql);
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					$_row[13] = str_replace(array('|','||'), array('%%%%%','<::::::>'),$_row[13]);
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _templates (
							`_fortemplate`, 
							`_fieldname`, 
							`_helpdescription`, 
							`_helpimage`, 
							`_fieldtype`, 
							`_maxlines`, 
							`_textlength`, 
							`_imagewidth`,
							`_imageheight`,
							`_cols`,
							`_colNames`,
							`_history`,
							`_growing`,
							`_lengthInfluence`,
							`__user__`, 
							`__parentnode__`, 
							`__sort__`) 
						VALUES (
							'".$_id."', 
							'".mysql_real_escape_string($_row[1])."', 
							'".mysql_real_escape_string($_row[2])."', 
							'".mysql_real_escape_string($_row[3])."', 
						 	'".mysql_real_escape_string($_row[7])."',
							'".$_row[8]."', 
							'".$_row[9]."', 
							'".$_row[10]."', 
							'".$_row[11]."', 
							'".$_row[12]."',
							'".mysql_real_escape_string($_row[4])."', 
							'".$_row[5]."',
							'".$_row[6]."', 
							'".$_row[13]."', 
							'".$this->rfDB->getUserId()."', 
							NULL, 
							".$__sort__.")");
						$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _templates SET 
							`_fortemplate`		='".$_id."', 
							`_fieldname`        ='".mysql_real_escape_string($_row[1])."',
							`_helpdescription`  ='".mysql_real_escape_string($_row[2])."',
							`_helpimage`        ='".mysql_real_escape_string($_row[3])."',
							`_fieldtype`        ='".mysql_real_escape_string($_row[7])."',
							`_maxlines`         ='".$_row[8]."', 
							`_textlength`       ='".$_row[9]."', 
							`_imagewidth`       ='".$_row[10]."', 
							`_imageheight`      ='".$_row[11]."', 
							`_cols`             ='".$_row[12]."',
							`_colNames`         ='".mysql_real_escape_string($_row[4])."',
							`_history`          ='".$_row[5]."',
							`_growing`          ='".$_row[6]."', 
							`_lengthInfluence`  ='".$_row[13]."', 
							`__sort__`          =".$__sort__."
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}			
				}
				$sql = ("DELETE FROM _templates WHERE `__sort__` = -1 && _fortemplate = ".$_id); $this->rfDB->query($sql);
				$this->buffer .= $this->rfTPL->templateSorting(false); 
				break;
			/* Stores user information */							
			case 'Listusers':
				$_group  = $this->rfDB->getUserGroup();
				if ($_group=="root") $sql = ("UPDATE users SET `usergroup` = 'delete'");
				else $sql = "UPDATE users SET `usergroup` = 'delete' WHERE usergroup = 'user' || id = ".$this->rfDB->getUserId();
				$this->rfDB->query($sql);						
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO users (`username`, `password`, `usergroup`) VALUES ('".mysql_real_escape_string($_row[1])."', '".md5($_row[2])."', '".mysql_real_escape_string($_row[3])."')");
						$this->rfDB->query($sql);
					} else {
						$sql = ("SELECT password FROM users WHERE id = ".$_row[0]." LIMIT 1");
						$_oldpw = $this->rfDB->fetch_object($this->rfDB->query($sql));
						$_pw = ($_row[2]==$_oldpw->password)?($_oldpw->password):(md5($_row[2]));						
						$sql = ("UPDATE users SET `username` = '".mysql_real_escape_string($_row[1])."', `password` = '".$_pw."', `usergroup` = '".mysql_real_escape_string($_row[3])."' WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}
				}			
				$sql = ("DELETE FROM users WHERE `usergroup` = 'delete'"); $this->rfDB->query($sql);
				break;	
			/* Stores group and rights information */							
			case 'Listrights':
				if ($this->rfDB->getUserGroup()=="root") $sql = ("UPDATE _rights SET `__sort__` = -1");
				else $sql = 'UPDATE _rights SET `__sort__` = -1 WHERE __user__ = "'.$this->rfDB->getUserId().'" || __user__ = "'.$this->rfDB->getUserName().'"';
				$this->rfDB->query($sql);										
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					$_issues = $_books = $_templates = $_users = "";
					foreach (explode('<:>',$_row[5]) as $_d) {
						list($_key,$_id) = explode('-',$_d);
						$_users .= $_id."<::::::>";
					}
					foreach (explode('<:>',$_row[4]) as $_d) {
						list($_key,$_id) = explode('-',$_d);
						$_issues .= $_id."<::::::>";
					}					
					foreach (explode('<:>',$_row[3]) as $_d) {
						list($_key,$_id) = explode('-',$_d);
						$_books .= $_id."<::::::>";
					}
					foreach (explode('<:>',$_row[2]) as $_d) {
						list($_key,$_id) = explode('-',$_d);
						$_templates .= $_id."<::::::>";
					}					
					
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _rights (
							`_group`, 
							`_fortemplate`, 
							`_forissue`, 
							`_forbook`, 
							`_foruser`, 
							`__user__`, 
							`__parentnode__`, 
							`__sort__`) 
						VALUES (
							'".mysql_real_escape_string($_row[1])."', 
							'".mysql_real_escape_string($_templates)."', 
							'".mysql_real_escape_string($_issues)."', 
						 	'".mysql_real_escape_string($_books)."',
						 	'".mysql_real_escape_string($_users)."',
							'".$this->rfDB->getUserId()."', 
							'-1', 
							".$__sort__.")");
							$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _rights SET 
							`_group` 		= '".mysql_real_escape_string($_row[1])."', 
							`_fortemplate`	= '".mysql_real_escape_string($_templates)."', 
							`_forissue`		= '".mysql_real_escape_string($_issues)."', 
							`_forbook`		= '".mysql_real_escape_string($_books)."',
							`_foruser`		= '".mysql_real_escape_string($_users)."',
							`__sort__`      =".$__sort__."
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}
				}			
				$sql = ("DELETE FROM _rights WHERE `__sort__` = -1"); $this->rfDB->query($sql);
				break;		
			case 'Listplugins':
				$sql = ("UPDATE _plugins SET `__sort__` = -1");
				$this->rfDB->query($sql);										
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _plugins (
							`_name`, 
							`_callback`, 
							`_config`, 
							`_page`, 
							`__user__`, 
							`__parentnode__`, 
							`__sort__`) 
						VALUES (
							'".mysql_real_escape_string($_row[1])."', 
							'".mysql_real_escape_string($_row[2])."', 
							'".mysql_real_escape_string($_row[3])."', 
						 	'".mysql_real_escape_string($_row[4])."',
							'".$this->rfDB->getUserId()."', 
							'-1', 
							".$__sort__.")");
							$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _plugins SET 
							`_name` 		= '".mysql_real_escape_string($_row[1])."', 
							`_callback`		= '".mysql_real_escape_string($_row[2])."', 
							`_config`		= '".mysql_real_escape_string($_row[3])."', 
							`_page`			= '".mysql_real_escape_string($_row[4])."', 
							`__sort__`      =".$__sort__."
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}
				}			
				$sql = ("DELETE FROM _plugins WHERE `__sort__` = -1"); $this->rfDB->query($sql);
				break;
			case 'Listfieldprocessors':
				$sql = ("UPDATE _fieldpostprocessor SET `__sort__` = -1");
				$this->rfDB->query($sql);										
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					$_templates = "";
					foreach (explode('<:>',$_row[2]) as $_d) {
						list($_key,$_id) = explode('-',$_d);
						$_templates .= $_id."<::::::>";
					}
					
					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _fieldpostprocessor (
							`_name`, 
							`_code`, 
							`_forfield`, 
							`__user__`, 
							`__parentnode__`, 
							`__sort__`) 
						VALUES (
							'".mysql_real_escape_string($_row[1])."', 
							'".mysql_real_escape_string($_row[3])."', 
							'".mysql_real_escape_string($_templates)."', 
							'".$this->rfDB->getUserId()."', 
							'-1', 
							".$__sort__.")");
							$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _fieldpostprocessor SET 
							`_name` 		= '".mysql_real_escape_string($_row[1])."', 
							`_code`			= '".mysql_real_escape_string($_row[3])."', 
							`_forfield`		= '".mysql_real_escape_string($_templates)."', 
							`__sort__`      =".$__sort__."
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}
				}			
				$sql = ("DELETE FROM _fieldpostprocessor WHERE `__sort__` = -1"); $this->rfDB->query($sql);
				break;
			case 'Listbatchs':
				$sql = ("UPDATE _batch SET `__sort__` = -1");
				$this->rfDB->query($sql);										
				foreach ($this->rfDB->_splitTableData($_POST['store']) as $__sort__=>$_row) {
					$_templates = "";
					foreach (explode('<:>',$_row[3]) as $_d) {
						list($_key,$_id) = explode('-',$_d);
						$_templates .= $_id."<::::::>";
					}

					if ($_row[0] == "added") {
						$sql = ("INSERT INTO _batch (
							`_name`, 
							`_description`,               
							`_forbook`, 
							`_precode`, 
							`_postcode`, 
							`__user__`, 
							`__parentnode__`, 
							`__sort__`) 
						VALUES (
							'".mysql_real_escape_string($_row[1])."', 
							'".mysql_real_escape_string($_row[2])."', 
							'".mysql_real_escape_string($_templates)."', 
							'".mysql_real_escape_string($_row[4])."', 
							'".mysql_real_escape_string($_row[5])."', 
							'".$this->rfDB->getUserId()."', 
							'-1', 
							".$__sort__.")");
							$this->rfDB->query($sql);
					} else {
						$sql = ("UPDATE _batch SET 
							`_name` 		= '".mysql_real_escape_string($_row[1])."', 
							`_description`	= '".mysql_real_escape_string($_row[2])."', 
							`_forbook`		= '".mysql_real_escape_string($_templates)."', 
							`_precode`		= '".mysql_real_escape_string($_row[4])."', 
							`_postcode`		= '".mysql_real_escape_string($_row[5])."',    							
							`__sort__`      =".$__sort__."
							 WHERE id = ".$_row[0]." LIMIT 1");
						$this->rfDB->query($sql);
					}
				}			
				$sql = ("DELETE FROM _batch WHERE `__sort__` = -1"); $this->rfDB->query($sql);
				break;		
		}                                                                                                  
		
		
		//$this->buffer .= "ok";
	}
	
	/**
	 * converts a page of a pdf to a png and directly returns it with the right header
	 *
	 * @param int $_page 
	 * @param string $_file 
	 * @return void
	 * @author Urs Hofer
	 */
	function PDF2Image($_page,$_file) {
		
		/* Closing session to allow multiple parallel connections */
		session_write_close(); 		
		
		header("Content-Type: image/png");

		/* Read the image */

		if (is_file(REPOSITORYDIR."/previews/".$_file)) {
			$im_1 = new Imagick(REPOSITORYDIR."/previews/".$_file.'['.($_page?$_page:0).']');
			$im_1->thumbnailImage(200,200, true);
			$im_1->setImageFormat("png");
			$this->buffer = $im_1;
			return;
		}

		// Error Pic
		$im = imagecreate(200, 200);
		$bg = imagecolorallocate($im, 255, 255, 204);
		$textcolor = imagecolorallocate($im, 255, 0, 0);
		imagestring($im, 2, 10, 95, 'Fehler. Vorschau erneut laden.', $textcolor);	 
		$error = false;
		imagepng($im);
		exit();
	}
	
	/**
	 * converts a video file to an animated gif sequence
	 *
	 * @param string $_movie 
	 * @return void
	 * @author Urs Hofer
	 */
	function Video2Gif($_movie, $_movie_get=false) {
		
		/* Closing session to allow multiple parallel connections */
		session_write_close(); 		
		
		if ($_movie_get) {
			list($fieldtype,$fieldid) = explode("-",$_movie_get);
			$_movie = $this->rfDB->getFieldData($fieldid)->$fieldtype;
		}
		
		if (!is_dir(REPOSITORYDIR.'/tmp')) {
			if (!mkdir (REPOSITORYDIR.'/tmp'))  die ("Could not create temporary directory.");
		}
		if (file_exists($_movie)) {
			$result = explode("  ", exec("md5sum $_movie"));
			$output_file_path =	REPOSITORYDIR."/tmp/".$result[0].".gif";
//			$output_file_path = REPOSITORYDIR."/tmp/".md5_file($_movie).".gif";
			/* send cache if there is one */
			if (!file_exists($output_file_path)) {
				/* else create one */
	//			$frames = array('10%', '20%', '30%', '40%', '50%', '60%', '70%', '80%', '90%');
				$frames = array('20%', '35%', '50%', '65%', '80%');
				// set the delay between frames in the output GIF
				$joiner = new Thumbnail_Joiner(50);
				foreach (new Thumbnail_Extractor($_movie, $frames, '150x113', ffmpeg) as $key => $frame) {
				    $joiner->add($frame);
				}
				if (!$joiner->save($output_file_path)) {
					// Error Pic
					$im = imagecreate(150, 113);
					$bg = imagecolorallocate($im, 204, 204, 204);
					$textcolor = imagecolorallocate($im, 255, 0, 0);
					imagestring($im, 2, 5, 5, "Error creating preview", $textcolor);	 
					imagestring($im, 2, 5, 20, basename($_movie), $textcolor);	 
					imagestring($im, 2, 5, 35, "Source is corrupt.", $textcolor);	 
					imagegif($im, $output_file_path);	
				}
			}
			$this->buffer.= '<img src="'.REPOSITORY."/tmp/".md5_file($_movie).'.gif">';
		}
		else $this->buffer.="Movie does not exist";
	}

	/**
	 * converts a video in a format which is compatible with the html 5 tag (ogg/theora or mp4)
	 *
	 * @param string $_scriptname 
	 * @param string $_basedir 
	 * @param string $_sourcefile 
	 * @param string $_target 
	 * @return void
	 * @author Urs Hofer
	 */
	function ExecuteVideoConversion($_scriptname, $_basedir, $_sourcefile, $_target) {
		
		$lockfile = sys_get_temp_dir() . '/videoconversion.lock'; 
		file_put_contents($lockfile, getmypid());
		
		$_sourcefile = base64_decode($_sourcefile);
		$_targetfile = $_target.'.ogv';
		$_targetfile_process = $_target.'_.ogv';
		
		

		if (!is_file($_targetfile)) {
			$exec = ffmpeg2theora.' -v 3 -x 320 -y 240 -o "'.$_targetfile_process.'" "'.$_sourcefile.'"';
			echo $exec."\n";
			exec($exec);	
			rename($_targetfile_process, $_targetfile);
			$this->buffer .= "Processed: $_targetfile";
		}
		else $this->buffer .= "Skipped ogv";
		
		$_targetfile = $_target.'.mp4';	
		$_targetfile_process = $_target.'_.mp4';
		if (!is_file($_targetfile)) {
			$exec = ffmpeg.' -i "'.$_sourcefile.'" -s 320x240 -acodec libfaac -ar 22050 -ab 64k -vcodec libx264 -threads 8 -f mp4 "'.$_targetfile_process.'"';
			exec($exec);	
			echo $exec."\n";
			rename($_targetfile_process, $_targetfile);
			$this->buffer .= "Processed: $_targetfile";
		}
		else $this->buffer .= "Skipped mp4";	
		
		unlink($lockfile);
			
	}
	
	
	/**
	 * triggers a video conversion
	 *
	 * @param string $_format 
	 * @param string $_file 
	 * @return void
	 * @author Urs Hofer
	 */
	function Video2HTML5($_file) {
		$_browser = $this->getBrowser();
		$_targetfile = REPOSITORYDIR."/tmp/".$_file;
		$_targetfile_web = SITEPREFIX.'/img/novideo';
		if (is_file($_targetfile.'.mp4') && is_file($_targetfile.'.ogv')) {
			$_mesg = "file_ok";
			$_targetfile_web = REPOSITORY."/tmp/".$_file;		
		}
		elseif (!is_file(sys_get_temp_dir() . '/videoconversion.lock')) {
			exec('echo "/usr/bin/php '.DOCROOT.SITEPREFIX.'/main.php '.DOCROOT.' '.$_file.' '.$_targetfile.' video"|at now');	
			$_mesg = conversion_started;
		}
		else {
			$_mesg = conversion_running;
		}
		
		$this->buffer = json_encode((object)array("mesg"=>$_mesg,"file"=>$_targetfile_web.($_browser['name']=='Safari'?'.mp4':'.ogv')));
		
	}
	
	/**
	 * returns a file from the udb repository if the access is okay
	 *
	 * @param string $_file 
	 * @return void
	 * @author Urs Hofer
	 */
	
    function _mime_content_type($filename) {
	 $mime_types = array(

	            'txt' => 'text/plain',
	            'htm' => 'text/html',
	            'html' => 'text/html',
	            'php' => 'text/html',
	            'css' => 'text/css',
	            'js' => 'application/javascript',
	            'json' => 'application/json',
	            'xml' => 'application/xml',
	            'swf' => 'application/x-shockwave-flash',
	            'flv' => 'video/x-flv',

	            // images
	            'png' => 'image/png',
	            'jpe' => 'image/jpeg',
	            'jpeg' => 'image/jpeg',
	            'jpg' => 'image/jpeg',
	            'gif' => 'image/gif',
	            'bmp' => 'image/bmp',
	            'ico' => 'image/vnd.microsoft.icon',
	            'tiff' => 'image/tiff',
	            'tif' => 'image/tiff',
	            'svg' => 'image/svg+xml',
	            'svgz' => 'image/svg+xml',

	            // archives
	            'zip' => 'application/zip',
	            'rar' => 'application/x-rar-compressed',
	            'exe' => 'application/x-msdownload',
	            'msi' => 'application/x-msdownload',
	            'cab' => 'application/vnd.ms-cab-compressed',

	            // audio/video
	            'mp3' => 'audio/mpeg',
	            'qt' => 'video/quicktime',
	            'mov' => 'video/quicktime',

	            // adobe
	            'pdf' => 'application/pdf',
	            'psd' => 'image/vnd.adobe.photoshop',
	            'ai' => 'application/postscript',
	            'eps' => 'application/postscript',
	            'ps' => 'application/postscript',

	            // ms office
	            'doc' => 'application/msword',
	            'rtf' => 'application/rtf',
	            'xls' => 'application/vnd.ms-excel',
	            'ppt' => 'application/vnd.ms-powerpoint',

	            // open office
	            'odt' => 'application/vnd.oasis.opendocument.text',
	            'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
	        );

	        $ext = strtolower(array_pop(explode('.',$filename)));
	        if (array_key_exists($ext, $mime_types)) {
	            return $mime_types[$ext];
	        }
	        elseif (function_exists('finfo_open')) {
	            $finfo = finfo_open(FILEINFO_MIME);
	            $mimetype = finfo_file($finfo, $filename);
	            finfo_close($finfo);
	            return $mimetype;
	        }
	        else {
	            return 'application/octet-stream';
	        }
    }
	
	function UDBAccess($_file) {
		$_file_in_db = basename($_file);
		$_path_parts = pathinfo($_file);
		$_file = REPOSITORYDIR."/".$_file;
		if (file_exists($_file)) {
			/* If logged in, downloads are always ok */			
			$_access_ok = $this->auth==1?true:false;
			if (!$_access_ok) switch (dirname($_file)) {
				/*
					/udb/thumbs
					/udb/previews
					/udb/pdfgen
					/udb
					/udb/tmp is always closed for non-logged in users...
				*/
				case REPOSITORYDIR.'/tmp':
					break;
				case REPOSITORYDIR.'/previews':
					if (defined("PDF_PREVIEW_ACCESS") && PDF_PREVIEW_ACCESS=='true') {
						$_access_ok = true;
						break;
					}
					/* Check for Filename in Database */
					$sql = 'SELECT _pdf._file FROM _pdf LEFT JOIN _issues ON _issues.id = _pdf._issue
					WHERE _pdf._file LIKE "%'.$_file_in_db.'%" AND (_issues.__config__=1 || _issues.__config__=3 || _issues.__config__=5) LIMIT 1';
					if ($this->rfDB->num_rows($this->rfDB->query($sql))>0) $_access_ok = true;
					break;
				case REPOSITORYDIR.'/pdfgen':
					if (defined("PDF_FULLTEXT_ACCESS") && PDF_FULLTEXT_ACCESS=='true') {
						$_access_ok = true;
						break;
					}
					$sql = 'SELECT _pdf._file FROM _pdf LEFT JOIN _issues ON _issues.id = _pdf._issue
					WHERE _pdf._file LIKE "%'.$_file_in_db.'%" AND (_issues.__config__=1 || _issues.__config__=4 || _issues.__config__=5) LIMIT 1';
					if ($this->rfDB->num_rows($this->rfDB->query($sql))>0) $_access_ok = true;
					break;
				case REPOSITORYDIR.'/thumbs':
					$_file_in_db = str_replace('-thmb.jpg', '', $_file_in_db);
				default:
					if (substr($_file_in_db, -12)=='-preview.jpg')
						$_file_in_db = str_replace('-preview.jpg', '', $_file_in_db);

					if (preg_match('/-preview[1-9].jpg/',substr($_file_in_db, -13)))
						$_file_in_db = substr($_file_in_db, 0, -13);


					if (defined("UPLOAD_UDB_ACCESS") && UPLOAD_UDB_ACCESS=='true') {
						$_access_ok = true;
						break;
					}
					/* Check for Filename in Database */
					$sql = 'SELECT _issues.__config__,_data._databinary FROM _data
					LEFT JOIN _contributions ON _data._forcontribution = _contributions.id		
					LEFT JOIN _issues ON _issues.id = _contributions._forissue
					WHERE _data._databinary LIKE "%'.$_file_in_db.'%" AND (_issues.__config__=1 || _issues.__config__=2) LIMIT 1';
					if ($this->rfDB->num_rows($this->rfDB->query($sql))>0) $_access_ok = true;
					break;
			}


			
			if ($_access_ok || (defined("PUBLIC_UDB_ACCESS") && PUBLIC_UDB_ACCESS=='true') || (defined("BY_TYPE_ACCESS") && in_array($_path_parts['extension'], explode(',',BY_TYPE_ACCESS)))) {
				if ($_SERVER['MOD_X_SENDFILE_ENABLED']) {

					// seconds, minutes, hours, days
					$expires = 60*60*24*14;
					header("Pragma: public");
					header("Cache-Control: maxage=".$expires);
					header('Expires: ' . gmdate('D, d M Y H:i:s', time()+$expires) . ' GMT');

					header('X-Sendfile: '.$_file);
					header('Content-Type: '.$this->_mime_content_type($_file));
				}
				else {
				    header('Content-Type: '.$this->_mime_content_type($_file));
				    header('Content-Length: ' . filesize($_file));
				    ob_clean();
				    flush();
				    readfile($_file);
				}
			    exit;
			}
			else {
				$this->_die(403);
			}
		}
		
		$this->_die();
	}
	
	/**
	 * prints the error page and dies
	 *
	 * @param string $_err 
	 * @return void
	 * @author Urs Hofer
	 */
	function _die($_err=404) {
		switch ($_err) {
			case 403:
				$_h = "HTTP/1.0 403 Forbidden";
				$_m = "No Access to resource";
				break;
			case 404:
				$_h = "HTTP/1.0 404 Not Found";
				$_m = "Resource not found";
				break;
		}
		header($_h);			
		die('<!DOCTYPE html>
		<html>
		<head>
		<meta http-equiv="Content-type" content="text/html; charset=utf-8">
		<title>Rokfor™ System Error:'.$_err.' '.$_m.'</title></head>
		<body bgcolor="white">
		<h1>Error '.$_err.'</h1>
		<h2>'.$_m.'</h2>
		</body>
		</html>');
	}
	
	function api() {
		switch ($this->action) {
			case 'Previewlist':
				/* Previewlist: Dumps all generated Single-Sheet Documents (a.k.a previews) odered by it's date*/
				/* /rf/Api/Previewlist/SessionId|Guest */

				$this->auth = $this->rfDB->checkSession($this->parameter);
				if ($this->parameter <> "Guest" || PDF_PREVIEW_ACCESS!==true) {
					if (!$this->auth) {
						echo json_encode("no session stored");						
						break;
					}
				}
				$_files = array();
				$_dir = new DirectoryIterator(REPOSITORYDIR."/previews");
				foreach ($_dir as $_fileinfo) {     
			       if ($_fileinfo->isFile()) $_files[$_fileinfo->getMTime()] = 'http://'.PROJECT_NAME.REPOSITORY.'/previews/'.$_fileinfo->getFilename();
				}
				ksort($_files);
				echo json_encode($_files);
				break;
			case 'XML':
				/* Exporter: Generate a xml and returns it to the browser... */
				/* /rf/Api/XML/SessionId/Bookname */
				/* /rf/Api/XML/SessionId/Bookname/Issuename */


				$_books_allowed_for_guests = array();
				if (defined("BOOKS_ALLOWED_FOR_GUESTS")) {
					$_books_allowed_for_guests = explode(",", BOOKS_ALLOWED_FOR_GUESTS);
					foreach ($_books_allowed_for_guests as &$_value) $_value = trim($_value, "\"\t\n\x0B\0\r ");
				}

				if ($this->parameter <> "Guest" || !in_array($this->values[0], $_books_allowed_for_guests)) {
					$this->auth = $this->rfDB->checkSession($this->parameter);
					if (!$this->auth) {
						echo json_encode("no session stored");						
						break;
					}
				}
				else $this->rfDB->setUserRestriction(false);
				
				/* Template */
				$xmlstr = "<?xml version='1.0' standalone='yes'?>
				<book>
				</book>";				
				$_book = new SimpleXMLElement($xmlstr);
				if (!$_bookid = $this->rfDB->getProjectId(urldecode($this->values[0]))) {
					echo json_encode("book unknown");						
					break;
				}
				$_bookinfo = reset($this->rfDB->getProjects(true, $_bookid->id));
				$_book->addAttribute('id', $_bookid->id);
				$_book->addAttribute('name', $_bookinfo->_name);				
				
									
				foreach ($this->rfDB->getEditions($_bookid->id) as $issue) {
					
					/* If a issue is selected skip the loop if the issue does not match the selected one */
					if ($this->values[1] && ($issue->_name <> $this->values[1])) {
						continue;
					}
					
					$_issue = $_book->addChild('issue');
					$_issue->addAttribute('id', $issue->id);
					$_issue->addAttribute('name', $issue->_name);								
					$_issue->addAttribute('status', $issue->_status);								

					foreach ($this->rfDB->getParts($_bookid->id) as $chapter) {
						$_chapter = $_issue->addChild('chapter');
						$_chapter->addAttribute('id', $chapter->id);
						$_chapter->addAttribute('name', $chapter->_name);				

						foreach ($this->rfDB->getElements($chapter->id, false, $issue->id, false, '__sort__ ASC') as $contribution) {
							$_contribution = $_chapter->addChild('contribution');
							$_contribution->addAttribute('id', $contribution->id);
							$_contribution->addAttribute('name', $contribution->_name);				
							$_contribution->addAttribute('status', $contribution->_status);				
							$template = $this->rfDB->getTemplateNamebyId($contribution->id);
							$_contribution->addAttribute('template', $template->_name);				
							$_contribution->addAttribute('user', $this->rfDB->getUserNameById($contribution->__user__));				
							$_contribution->addAttribute('last_modified', $contribution->_moddate);
							
							/* Inserting into _pdf log with __config__ 0 means single page */
							$_sql = ("SELECT * FROM _pdf WHERE __config__ = 0 && _issue = ".$contribution->id." order by _date DESC");
							$_query = @$this->rfDB->query($_sql);
							while ($_exports = @$this->rfDB->fetch_object($_query)) {				
								$_field = $_contribution->addChild('export');
								$_field->addAttribute('url', 'http://'.PROJECT_NAME.REPOSITORY.'/previews/'.$_exports->_file);
								$_field->addAttribute('time', $_exports->_date);
								$_field->addAttribute('pages', $_exports->_pages);								
							}

							foreach ($this->rfDB->getFieldsId($contribution->id) as $field) {
								/* Add some Data */
								switch ($field->_fieldtype) {
									case 'Bild':
										$_field = $_contribution->addChild('field');
										foreach ($this->rfDB->_splitTableData($field->d_binary) as $urls) {
											if (is_file(REPOSITORYDIR.'/'.$urls[1]))  {
												$_url = $_field->addChild('file');
												$_url->addAttribute('url', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]);
												if (is_file(REPOSITORYDIR.'/thumbs/'.$urls[1]."-thmb.jpg")) $_url->addAttribute('thumb', 'http://'.PROJECT_NAME.REPOSITORY.'/thumbs/'.$urls[1]."-thmb.jpg");
												if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview.jpg")) $_url->addAttribute('preview', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]."-preview.jpg");
												if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview0.jpg")) $_url->addAttribute('preview', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]."-preview0.jpg");											
												if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview1.jpg")) $_url->addAttribute('preview', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]."-preview1.jpg");											
												$_url->addAttribute('description', $urls[0]);											
											}
										}
										break;
									case 'Zahl':
									case 'TypologySlider':
										$_field = $_contribution->addChild('field', $field->d_integer);
										break;	
									case 'Tabelle':
										$_field = $_contribution->addChild('field');
										foreach ($this->rfDB->_splitTableData($field->d_text) as $rows) {
											$_row = $_row->addChild('tr');
											foreach ($rows as $columns) {											
												$_column = $_row->addChild('td');
												$node= dom_import_simplexml($_column); 
											   	$no = $node->ownerDocument; 
											   	$node->appendChild($no->createCDATASection($columns));												
											}
										}
										break;																		
									default:
										$_field = $_contribution->addChild('field');
										if ($field->d_text) {
											$node= dom_import_simplexml($_field); 
										   	$no = $node->ownerDocument; 
										   	$node->appendChild($no->createCDATASection($field->d_text));
										}										
										break;
								}
								/* Add Field Info */
								$_field->addAttribute('id', $field->id);
								$_field->addAttribute('name', $field->_fieldname);
								$_field->addAttribute('type', $field->_fieldtype);								
							
							}
						}
					}
				}

				/* Pretty Print and Output */
				$dom = DOMDocument::loadXML($_book->asXML(), LIBXML_NOBLANKS); 
				$dom->formatOutput = true;
				header("Content-Type: text/xml");
				echo $dom->saveXML();				
				break;
			case 'XML-Tree':
				/* Exporter: Generate a xml and returns it to the browser... */
				/* This time, the tree is not ordered by the book -> issue -> chapter structure, */
				/* But by a hierarchical logic */
				/* /rf/Api/XML/SessionId/Bookname/NestFieldName/DataFields[optional, comma separated] */

				$_books_allowed_for_guests = array();
				if (defined("ISSUES_ALLOWED_FOR_GUESTS")) {
					$_issues_allowed_for_guests = explode(",", ISSUES_ALLOWED_FOR_GUESTS);
					foreach ($_issues_allowed_for_guests as &$_value) $_value = trim($_value, "\"\t\n\x0B\0\r ");
				}

				if ($this->parameter <> "Guest" || !in_array($this->values[0], $_issues_allowed_for_guests)) {
					$this->auth = $this->rfDB->checkSession($this->parameter);
					if (!$this->auth) {
						echo json_encode("no session stored");						
						break;
					}
				}
				else $this->rfDB->setUserRestriction(false);

				$_nestfield = $this->values[1];
				if (!$_nestfield) {
					echo json_encode("nestfield missing");	
					break;
				}

						
				/* Template */
				$xmlstr = "<?xml version='1.0' standalone='yes'?>
				<tree>
				</tree>";				
				$_tree = new SimpleXMLElement($xmlstr);
				if (!$_issueid = $this->rfDB->getEditionIdbyEditionname(urldecode($this->values[0]))) {
					echo json_encode("issue unknown");						
					break;
				}
				
				function _add_contrib (&$contribution, &$_parent, &$_this, $sql, $_nestfield) {

					$_datafields = array();
					if ($_this->values[2]) {
						$_datafields = explode(',', $_this->values[2]);
					}

					$_contribution = $_parent->addChild('contribution');	
					$_contribution->addAttribute('id', $contribution->id);
					$_contribution->addAttribute('name', $contribution->_name);				
					$_contribution->addAttribute('status', $contribution->_status);				
					$template = $_this->rfDB->getTemplateNamebyId($contribution->id);
					$_contribution->addAttribute('template', $template->_name);				
					$_contribution->addAttribute('user', $_this->rfDB->getUserNameById($contribution->__user__));				
					$_contribution->addAttribute('last_modified', $contribution->_moddate);

					/* Inserting into _pdf log with __config__ 0 means single page */
					$_sql = ("SELECT * FROM _pdf WHERE __config__ = 0 && _issue = ".$contribution->id." order by _date DESC LIMIT 1");
					$_query = @$_this->rfDB->query($_sql);
					while ($_exports = @$_this->rfDB->fetch_object($_query)) {				
						$_field = $_contribution->addChild('export');
						$_field->addAttribute('url', 'http://'.PROJECT_NAME.REPOSITORY.'/previews/'.$_exports->_file);
						$_field->addAttribute('time', $_exports->_date);
						$_field->addAttribute('pages', $_exports->_pages);								
					}	
					
					foreach ($_this->rfDB->getFieldsId($contribution->id) as $field) {
						if (count($_datafields)==0 || in_array($field->_fieldname, $_datafields)) { 
						/* Add some Data */
						switch ($field->_fieldtype) {
							case 'Bild':
								$_field = $_contribution->addChild('field');
								foreach ($_this->rfDB->_splitTableData($field->d_binary) as $urls) {
										if (is_file(REPOSITORYDIR.'/'.$urls[1]))  {
											$_url = $_field->addChild('file');
											$_url->addAttribute('url', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]);
											if (is_file(REPOSITORYDIR.'/thumbs/'.$urls[1]."-thmb.jpg")) $_url->addAttribute('thumb', 'http://'.PROJECT_NAME.REPOSITORY.'/thumbs/'.$urls[1]."-thmb.jpg");
											if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview.jpg")) $_url->addAttribute('preview', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]."-preview.jpg");
											if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview0.jpg")) $_url->addAttribute('preview', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]."-preview0.jpg");											
											if (is_file(REPOSITORYDIR.'/'.$urls[1]."-preview1.jpg")) $_url->addAttribute('preview', 'http://'.PROJECT_NAME.REPOSITORY.'/'.$urls[1]."-preview1.jpg");											
											$_url->addAttribute('description', $urls[0]);											
										}
									}
									break;
								case 'Zahl':
								case 'TypologySlider':
									$_field = $_contribution->addChild('field', $field->d_integer);
									break;	
								case 'Tabelle':
									$_field = $_contribution->addChild('field');
									foreach ($_this->rfDB->_splitTableData($field->d_text) as $rows) {
										$_row = $_row->addChild('tr');
										foreach ($rows as $columns) {											
											$_column = $_row->addChild('td');
											$node= dom_import_simplexml($_column); 
										   	$no = $node->ownerDocument; 
										   	$node->appendChild($no->createCDATASection($columns));												
										}
									}
									break;																		
								default:
									$_field = $_contribution->addChild('field');
									if ($field->d_text) {
										$node= dom_import_simplexml($_field); 
									   	$no = $node->ownerDocument; 
									   	$node->appendChild($no->createCDATASection($field->d_text));
									}										
									break;
							}
							/* Add Field Info */
							$_field->addAttribute('id', $field->id);
							$_field->addAttribute('name', $field->_fieldname);
							$_field->addAttribute('type', $field->_fieldtype);								
						}
					}
					

					// Check for children
					$query = $_this->rfDB->query($sql);
					while ($_children = $_this->rfDB->fetch_object($query)) {				
						$_child = (int)$_this->rfDB->getFieldsByName($_nestfield, $_children->id)->d_text;
						if ($_child == $contribution->id) {
							_add_contrib($_children,$_contribution, $_this, $sql, $_nestfield);
						}
					}
				}
				
				// Cycle thru all elements with nestfield == -1
				// Those are the parents
				$sql = 'SELECT _contributions.* FROM _contributions 
				WHERE _contributions._status <> "Deleted" && 
				_contributions._forissue = '.$_issueid;
				$query = $this->rfDB->query($sql);
				while ($_contribution = $this->rfDB->fetch_object($query)) {				
					$_parent = (int)$this->rfDB->getFieldsByName($_nestfield, $_contribution->id)->d_text;
					if ($_parent == -1) _add_contrib($_contribution,$_tree, $this, $sql, $_nestfield);
				}
				
				/* Pretty Print and Output */
				$dom = DOMDocument::loadXML($_tree->asXML(), LIBXML_NOBLANKS); 
				$dom->formatOutput = true;
				header("Content-Type: text/xml");
				echo $dom->saveXML();				
				break;				
			case 'Export':
				/* Exporter: Generate a file and returns it on the fly to the browser... */
				/* /rf/Api/Export/SessionId|Guest/Single/ContribId/[PlugId]/[Link] */
				/* If Link is omitted, a pdf will be returned. If set to Link, only the url will be returned */
				/* If set to File, the raw filename will be returned */

				if ($this->parameter <> "Guest" || PDF_PREVIEW_ACCESS!==true) {
					$this->auth = $this->rfDB->checkSession($this->parameter);
					if (!$this->auth) {
						echo json_encode("no session stored");						
						break;
					}
				}
				ob_start();
				$pdflink = $this->rfTPL->showExporter($this->values[0],$this->values[1],$this->values[2]?$this->values[2]:false,true);
				ob_end_clean();
				if ($this->values[3] == "Link") echo json_encode(array("ok", "http://".PROJECT_NAME.REPOSITORY."/previews/".$pdflink));
					else if ($this->values[3] == "File") echo json_encode(array("ok", $pdflink));
				else $this->UDBAccess("previews/".$pdflink);
				break;
				
			/* Call a stored batch function */
			/* /rf/Api/Export/SessionId/Function Id/Issue Id/Book Id */
			case 'Executebatch':
				$this->auth = $this->rfDB->checkSession($this->parameter);
				if (!$this->auth) {
					echo json_encode("no session stored");						
					break;
				}
				else {				
					/* Compatibility to the normal browser call */
					$_POST['function_id'] = $this->values[0];
					$_POST['issue_id'] = $this->values[1];
					$_POST['book_id=1'] = $this->values[2];
					if ($this->batchProcessorPost()) echo json_encode("ok");
					else echo json_encode("error in batch function. check the batch syntax in the rokfor backend.");
				}
				break;
			case 'Store':
				/* Store: Stores Content in a Text Field 
	 			   /rf/Api/Store/SessionId
				   Post Variables: 	- book(string), 
									- issue(string), 
									- chapter(string), 
									- element(string),
					 				- template(string), 
									- data(json encoded string)
				   JSON format data: [[fieldname 1,fielddata 1],[fieldname2,fielddata 2],...] */
				$this->auth = $this->rfDB->checkSession($this->parameter);
				if (!$this->auth) {
					echo json_encode("no session stored");						
					break;
				}
				else {
					if (!$_bookid = $this->rfDB->getProjectId($_POST["book"])->id) {
						echo json_encode("book not found");			
						break;
					}
					if (!$_issueid = $this->rfDB->getEditionIdbyEditionname($_POST["issue"], $_bookid)) {
						echo json_encode("issue not found within book $_bookid");						
						break;
					}
					if (!$_chapterid = $this->rfDB->getPartByName($_POST["chapter"], $_bookid)) {
						echo json_encode("chapter not found within book $_bookid");						
						break;
					}	
					/* Add or use an contribution */
					if (!$_contribid = $this->rfDB->getElementsIdbyName ($_POST["element"], false, $_chapterid, $_issueid)->id) {
						$_templateid = false;
						foreach ($this->rfDB->getTemplates($_bookid, $_chapterid) as $_t) {
							if ($_t->_name == $_POST["template"]) $_templateid = $_t->id;
						}
						if (!$_templateid) {
							echo json_encode("template not found");						
							break;							
						}
						$_contribid = $this->rfDB->addElement ($_POST["element"],$_templateid,$_chapterid,$_issueid);
					}
					$_data = json_decode($_POST["data"]);
					
					$retval = array();
					foreach ($_data as $value) {
						if ($_fieldId = $this->rfDB->getFieldsByName($value[0], $_contribid)->id) {
							$this->rfDB->setFieldData($this->_clean_from_rteditor($value[1], true),$_fieldId, '_datatext');
							$retval[$value[0]] = "ok";
						}
						else {
							$retval[$value[0]] = "not found";							
						}
					}
					echo json_encode($retval);						
					break;
				}
			break;
			case 'Login':
				/* Login: Starts a new Session */
				/* /rf/Api/Login/Username/Pass(md5) */
				if ($this->auth = $this->rfDB->newSession($this->parameter,$this->values[0])) {
					echo json_encode(array("ok", $this->rfDB->getSessionId()));
				}
				else {
					echo json_encode(array("wrong username or password",$this->auth));
				}
				break;
			case 'Logout':
				/* Login: Deletes Session */
				/* /rf/Api/Logout/SessionId */
				if ($this->rfDB->cleanSessionId($this->parameter)) echo json_encode("ok");
				else echo json_encode("no session stored");
				break;
			default:
				unset($_SESSION['sessionid']);
				$this->_die();
				break;
		}		
		/* API Calls always exit here! */
	    exit;		
	}
	
	/**
	 * stores a word in the user dictionary
	 *
	 * @param string $_word 
	 * @return void
	 * @author Urs Hofer
	 */
	function saveCorrex($word) {
		$pspell_config = pspell_config_create("de" , "" , "", "utf-8");
		if (pspell_config_personal($pspell_config, LOGDIR."/".PROJECT_NAME."_custom_dictionary.pws")) {
			$pspell_link = pspell_new_config($pspell_config);
			if (pspell_add_to_personal($pspell_link, $word)) {
				if (pspell_save_wordlist($pspell_link)) {
					$this->buffer .= $word." ".isSaved.".";
				} 	else $this->buffer .= ErrSaveWordlist;
			} 		else $this->buffer .= ErrAddWordlist;
		} 			else $this->buffer .= ErrOpenWordlist;
	}
	
	function moveContribution($_contrib,$_bookissuechapter) {
		list ($_b,$_forissue,$__split__) = explode("-", $_bookissuechapter);
		$sql = ("UPDATE  `_contributions` SET  `__split__` =  '$__split__', `_forissue` =  '$_forissue' WHERE  `_contributions`.`id` =".$_contrib->id." LIMIT 1;");		
		$this->rfDB->query($sql);
		$this->buffer = "Moved contrib ".$_contrib->id." to Book $_b, Issue $_forissue, Chapter $__split__";
	}
	
	
	/**
	 * createBuffer: generates the html/xml output.
	 * 
	 * @param boolean $ajax		
	 * @return void
	 * @author Urs Hofer
	 **/
	function createBuffer()
	{
		
		$_clearSession = false;								
		
		
		/* File Control */
		if ($this->action == 'Filecontrol') {
			$this->UDBAccess($this->parameter);
		}
		
		/* API Access */
		if ($this->api===true) {
			header("Access-Control-Allow-Origin: *");
			header("Access-Control-Allow-Headers: {$_SERVER['HTTP_ACCESS_CONTROL_REQUEST_HEADERS']}");
			$this->api();
		}

		/* HTML or AJAX (html without Headers) access */
					
		/* REDIRECTORS */
	
		// Expired redirection
		if ($this->auth==-1) {
			$this->rfTPL->redirect('/Expired');
		}
		// Logout/Not Valid Redirection
		if ($this->auth==0&&!$_SESSION['redirected']) {
			$_SESSION['redirected']=true;
			$this->rfTPL->redirect('/Login');
		}		

		/* START OUTPUT */
	
		// Print Header if it's not an ajax call
		if (!$this->ajax) $this->rfTPL->head();

		// Login / Expired Message if not authenticated
		if ($this->auth==0) {
			unset($_SESSION['redirected']);
			$this->rfTPL->login($this->authmsg);			
		}
		// Logged In
		if ($this->auth==1) {
			switch ($this->action) {

				// Ajax: Tree content
				case 'Tree':
					$this->rfTPL->xml($this->treeXML($this->parameter,$this->values));
					break;
				// Ajax: Context Menu
				case 'Context':
					$this->rfTPL->context();
					break;				
				// Ajax: Main Form
				case 'Form':
					$_contrib = $this->rfDB->getElementsNamebyId($this->parameter);
					// Update Session
					$this->updateSessionVars($_contrib);
					$this->rfTPL->mainformHeader($_contrib,$this->rfDB->getTemplateNamebyId($this->parameter));	
					foreach ($this->rfDB->getFieldsByElement($this->parameter) as $_fieldinfo) {
						$this->rfTPL->mainformField($this->rfDB->getFieldData($_fieldinfo->id));	
					}
					$this->rfTPL->mainformFooter();						

					break;
				// Ajax: Add a new contribution	
				case 'Newcontribution':				
					$this->newContribution($this->parameter,$this->values[0],$_POST['contribname'],$_POST['templatename']);
					break;
				// Ajax: Set State of Contribution to "Close"
				case 'Closecontribution':
					$this->closeContribution($this->parameter);				
					break;
				case 'Changetemplate':
					$_contrib = $this->rfDB->getElementsNamebyId($this->parameter);	
					$this->changeTemplate($_contrib,$this->values[0]);
					break;
				// Ajax: Set State of Contribution to "Deleted"
				case 'Deletecontribution':
					$this->closeContribution($this->parameter,'Deleted');	
					$_clearSession = true;								
					break;
				// Ajax: Set State of Contribution to "Open"					
				case 'Unreleasecontribution':
					$this->closeContribution($this->parameter,'Open');				
					break;
				// Ajax: Set State of Contribution to "Open"					
				case 'Undeletecontribution':
					$this->closeContribution($this->parameter,'Open');				
					break;
				// Ajax: Trash Contribution and delete all Data / Binaries
				case 'Trashcontribution':
					$this->trashContribution($this->parameter);	
					$_clearSession = true;			
					break;					
				// Ajax: Return Progress Info of File upload
				case 'Progress':
					$this->getProgress($this->parameter);
					break;
				// Ajax: Shows other Users online
				case 'OtherUsers':
					$this->showOtherUsers();
					break;
				// Ajax: Store Data
				case 'Store':	
					if ($this->parameter=="New") {
						$this->parameter = $this->newContribution($_SESSION['issueid'],$_SESSION['chapterid'],$_POST['newname'],$_SESSION['templateid']);
						$this->transformFormFields($this->parameter);
					}
					$_contrib = $this->rfDB->getElementsNamebyId($this->parameter);
					$this->updateSessionVars($_contrib);					
					$this->storeData($_contrib);
					break;	
				case 'Clonedata':
					$this->cloneData($this->parameter,$this->values[0]);
					break;
				case 'Movecontribution':
					$_contrib = $this->rfDB->getElementsNamebyId($this->parameter);				
					$this->moveContribution($_contrib,$_POST['issuechapter']);
					break;
				case 'Clonecontribution':
					$_contrib = $this->rfDB->getElementsNamebyId($this->parameter);				
					$this->cloneContribution($_contrib,$_POST['prompt']);
					break;
				case 'Renamecontribution':
					$this->renameContribution($this->parameter,$_POST['prompt']);
					break;
				// Ajax: Delete a Binary File from Array
				case 'Deletebinary':
					$this->deleteFromBinary($this->parameter,$this->values[0]);
					break;
				// Ajax: Close (Archive) an Issue
				case 'Closeissue':
					$this->setIssueState($this->parameter,"closed");
					break;
				// Ajax: Reopen (UnArchive) an Issue
				case 'Reopenissue':
					$this->setIssueState($this->parameter,"open");
					break;				
				// Ajax: Clone a Issue
				case 'Cloneissue':
					$this->cloneIssue($this->parameter,$_POST['prompt']);
					break;
				// Ajax: Print a html form for a pop up (rfGui Component)
				case 'Forms':			
					$this->rfTPL->printAjaxForm($this->parameter,$this->values[0],$this->values[1],$this->values[2],$this->values[3], $this);				
					break;
				// Ajax: Prints the Table content for a popup (rfGui Component)
				case 'Table':			
					$this->rfTPL->printAjaxTableeditor($this->parameter,$this->values[0],$this->values[1]);				
					break;
				case 'Showinfo':
					$this->rfTPL->displayInfo();
					break;
				case 'Animatedpreview':
					$this->Video2Gif($_POST['movieFile'],$this->parameter);
					break;
				case 'Videoconversion':
					$this->Video2HTML5($this->parameter);
					break;
				case 'System':
					$this->storeSystemSettings($this->parameter,$this->values[0]);		
					break;
				case 'Showbatches':
					$this->rfTPL->showBatches();
					break;
				case 'Searchreplaceform':
					$this->rfTPL->showSearchReplaceForm();
					break;
				case 'Executebatch':
					$this->batchProcessorPost();
					break;
				case 'Executesearch':
					$this->searchreplaceResults();
					break;				
				case 'Export':
					$this->rfTPL->showExporter($this->parameter,$this->values[0]);		
					break;
				case 'Render':
					$this->rfTPL->showExporter($this->parameter,$this->values[0],$this->values[1],true);		
					break;					
				case 'Pdfconverter':
					$this->PDF2Image($this->parameter,$this->values[0]);
					break;
				case 'Savecorrex':
					$this->saveCorrex($_POST['word']);
					break;
				// Base Frame if no command			
				default:
					$this->updateSessionVars();
					// Reduce multisession users to regular ones.
					$this->rfTPL->baseframe($this->rfDB->getUserGroup()=='api'?'user':$this->rfDB->getUserGroup(),$this->rfDB->getUserName());			
					break;
			}
		}


		// Clear Session
		if ($_clearSession) $this->updateSessionVars();

		// Print Javascript Domready events in any case if there are some stored
		$this->rfTPL->domready();		

		// Print Footer if it's not an ajax call		
		if (!$this->ajax) $this->rfTPL->foot();			
	}
	
	/**
	 * distiller: converts a whole issue into a pdf
	 *
	 * @param int $_issueid 
	 * @param int $_pluginid 
	 * @param string $_type 
	 * @param string $_mode 
	 * @param int $_sessionid 
	 * @return void
	 * @author Urs Hofer
	 */
	function distiller($_scriptname, $_basedir, $_issueid, $_plugid, $_type, $_mode, $_sessionid, $_distillerid) {

		if (!is_dir(REPOSITORYDIR.'/pdfgen')) {
			if (!mkdir (REPOSITORYDIR.'/pdfgen'))  die ("Could not create thumbnail directory.");
		}


		/* Update db if necessary */
		if (!in_array('_plugin', $this->rfDB->db->get_fields('_pdf')))
		{
			$sql = ("
				ALTER TABLE  `_pdf` ADD  `_issue` INT( 32 ) NULL AFTER  `_date` ,
				ADD  `_plugin` INT( 32 ) NULL AFTER  `_issue` ,
				ADD  `_pages` INT( 32 ) NULL AFTER  `_plugin` ;
			");
			$this->rfDB->query($sql);				
		}


		$sql = "SELECT * FROM _plugins WHERE id = $_plugid";
		$query = @$this->rfDB->query($sql);
		$_plug = @$this->rfDB->fetch_object($query);		

		/* due to backward compatibility with some plugins */
				
		if (!$_db) $_db = &$this->rfDB->db; 					// Compatibility
		if (!defined("rootdir")) define("rootdir", DOCROOT);	// Compatibility
		$_plugin = $_plug->_name;								// Compatibility
		$post_processor_argv = array(							// Compatibility
			$_scriptname,
			$_issueid, 
			$_basedir, 
			$_plugin, 
			$_type, 
			$_mode,
			$_sessionid
		);
		global $_rokforDB;										// Compatibility
		$_rokforDB = new rokfor_DB();							// Compatibility
				
		include (GENERATORDIR."/engines/".$this->rfTPL->_getEngine($_plug->_name)."/core.inc");

		$tex = new TeX($_plugin,$_rokforDB); if ($tex->error) {
			die($tex->error);
		}
		
		/* Pre Processor Callback	- Executed after the whole File is generated */
		if (method_exists('preprocessor', 'generic_v2')) call_user_func(array('preprocessor', 'generic_v2'),$tex, REPOSITORYDIR."/pdfgen");

		$tex->finalize($_issueid);
		if ($pdflink = $tex->generatePDF(REPOSITORYDIR."/pdfgen")) {	

			/* Analyzing PDF - Caching Pages Pass one - maybe used by the postprocessor */
			$_pages = (int)exec('pdfinfo "'.(REPOSITORYDIR."/pdfgen/".$pdflink).'" 2>/dev/null | grep Pages | cut -d ":" -f 2');
			$sql = ("
					UPDATE  _pdf SET  _pages = '".$_pages."', __sort__ = _pdf.id WHERE  _pdf.__config__ = '".$_distillerid."' LIMIT 1 ;
				");
			$_db->query($sql);

			/* Post Processor Callback	- Executed after the whole File is generated */
			if (method_exists('postprocessor', 'generic_v2')) call_user_func(array('postprocessor', 'generic_v2'),$tex, $post_processor_argv,$pdflink,  REPOSITORYDIR."/pdfgen");
			/* Analyzing PDF - Caching Pages again - they might be changed by the postprocessor */
			$_pages = (int)exec('pdfinfo "'.(REPOSITORYDIR."/pdfgen/".$pdflink).'" 2>/dev/null | grep Pages | cut -d ":" -f 2');
			if ($_pages>0) {
				$sql = ("UPDATE  _pdf SET  _file = '$pdflink', _pages = '".$_pages."', __sort__ = _pdf.id WHERE  _pdf.__config__ = '".$_distillerid."' LIMIT 1 ;");
				$_db->query($sql);
			}
			$sql = ("UPDATE  _pdf SET  _file = '$pdflink', __sort__ = _pdf.id WHERE  _pdf.__config__ = '".$_distillerid."' LIMIT 1 ;");
			$_db->query($sql);			

			
			
			$this->sendMail(USERMAIL, "Successfully distilled the PDF: http://".PROJECT_NAME.REPOSITORY."/pdfgen/".$pdflink."\n\nLog File:\n\n".$tex->_logString, "Rokfor: PDF Distiller Result");
			
		}
		else {
			$this->sendMail(USERMAIL, "No pdf file was produced.\nCheck the logs.", "PDF GENERATOR ERROR");
		}		
	}
	
	/**
	 * Send a email to $_to, with the message $_mailmesg and the subject $_subject
	 *
	 * @param string $_to 
	 * @param string $_mailmesg 
	 * @param string $_subject 
	 * @return void
	 * @author Urs Hofer
	 */
	function sendMail($_to, $_mailmesg, $_subject=false) {
		$_headers = 'From: PDF Generator <pdf@tr51.org>' . "\n" .
		    'Reply-To: PDF Generator <pdf@tr51.org>' . "\n" .
			'CC: PDF Generator <pdf@tr51.org>' . "\n" .
		    'X-Mailer: Microsoft Office Outlook 12.0';		
		mail($_to, $_subject?$_subject:"PDF Generator", $_mailmesg, $_headers);	
	}
	
} // END class 

?>