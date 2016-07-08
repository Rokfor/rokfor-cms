<?php

/**
 * rokfor_DB class
 * database access matching the logic of the pdf-engine
 * This logic assumes the following structure:
 * Project > Issue > Parts > Elements > Fields
 *
 *
 * @package Rokfor
 * @author Urs Hofer
 **/

class db {

  /**
   * pdo object
   *
   * @var string
   */
  private $db;
  
  /**
   * version
   *
   * @var string
   */
  var $version;

  /**
   * last query id
   *
   * @var string
   */
  private $query_id;
  
  /**
   * application name
   *
   * @var string
   */
  var $appname;
  
  /**
   * throw error exceptions if set to true
   *
   * @var string
   */
  var $show_error;
  
  
  /**
   * storing private credentials 
   **/
  
  private $host;
  private $user;
  private $pass;
  private $dbname;
  
  function db($host,$user,$pass,$dbname,$version = false) {
    $this->db = new mysqli($host, $user, $pass, $dbname);
    if ($this->db->connect_error) {
      $this->error("Error Connecting mysqli");
      return false;
    }
    $this->version = $version;
    $this->appname = "";
    $this->show_error = false;

    $this->host = $host;
    $this->user = $user;
    $this->pass = $pass;
    $this->dbname = $dbname;
    
    return true;
  }
  
  function __destruct () {
    $this->db->close();
  }
  
  function close() {
    echo "closing";
    $this->db->close();
    unset($this->db);
  }
  
  function reopen() {
    $this->db = new mysqli($this->host, $this->user, $this->pass, $this->dbname);    
  }
  
  function escape($sql) {
    return $this->db->escape_string($sql);
  }
  
  function query($query_string,$limit = 0, $offset = 0, $showerror = 1) {
    if($limit != 0) $query_string.= " LIMIT $offset, $limit";    
    $this->query_id = $this->db->query($query_string);
    if ($showerror==1 && !$this->query_id) $this->error("Invalid SQL: ".$query_string);
    return $this->query_id;
  }
  
  function unbuffered_query($query_string,$limit = 0, $offset = 0, $showerror = 1) {
    if($limit != 0) $query_string.= " LIMIT $offset, $limit";    
    $this->query_id = $this->db->query($query_string, MYSQLI_USE_RESULT);
    if ($showerror==1 && !$this->query_id) $this->error("Invalid SQL: ".$query_string);
    return $this->query_id;
  }
  
  function affected_rows($q) {
    return $this->db->affected_rows();
  }
  
  function num_rows($q = false) {
    if ($q) {
      $this->query_id = $q;
    }
    return $this->query_id->num_rows;
  }
  
  function insert_id($i = false) {
    return $this->db->insert_id;    
  }
  
  function fetch_object($q = false) {
    if ($q) {
      $this->query_id = $q;
    }
    return $this->query_id->fetch_object();
  }
  
  function fetch_assoc($q = false) {
    if ($q) {
      $this->query_id = $q;
    }
    return $this->query_id->fetch_assoc();
  }
  
  function fetch_array($q = false) {
    if ($q) {
      $this->query_id = $q;
    }
    return $this->query_id->fetch_array();    
  }
  
  function query_first_object($query_string, $limit=0, $offset=0, $showerror=1) {
    $this->query($query_string, $limit, $offset, $showerror);
    return $this->fetch_object();
  }
  
  function get_fields($table) {
    $fields = $this->query('SHOW COLUMNS FROM '.$this->escape($table));
    while ($row=$this->fetch_array($fields))
      $result[]=$row['Field'];  
    return $result;
  }
  
  function get_field($q = false, $field){
    if ($q) {
      $this->query_id = $q;
    }
    $arr = $this->query_id->fetch_array();
    return $arr[$field] ? $arr[$field] : false;
  }
  
  function error($errormsg = "") {
    $errormsg="<b>Database error in $this->appname:</b> $errormsg\n<br>";
    $errormsg.="<b>mysql error:</b> $this->db->$errno\n<br>";
    $errormsg.="<b>mysql error number:</b> $this->errno\n<br>";
    $errormsg.="<b>mysql version:</b> ".$this->getversion()."\n<br>";
    $errormsg.="<b>php version:</b> ".phpversion()."\n<br>";
    $errormsg.="<b>Date:</b> ".date("d.m.Y @ H:i")."\n<br>";
    $errormsg.="<b>Script:</b> ".getenv("REQUEST_URI")."\n<br>";
    $errormsg.="<b>Referer:</b> ".getenv("HTTP_REFERER")."\n<br><br>";

    if($this->show_error) {
      $_mesg = "<p>".$errormsg."</p>";
      $_mesg .= "<p><pre>".htmlentities(print_r($_error,true))."</pre></p>";		
      throw new Exception($_mesg, 1); 
    }
  }
}


class rokfor_DB {

	/**
	 * @var Database Object, inherited from mysql.class
	 **/
	var $db = NULL;

	/**
	 * @var Array of Projects available
	 **/
	var $projects = array();
	
	/**
	 * @var Pointer to active Project
	 **/
	var $activeProject;

	/**
	 * @var Array of editions for a project
	 **/
	var $editions = array();

	/**
	 * @var Pointer to active Edition
	 **/
	var $activeEdition;
	
	/**
	 * @var Array of parts for an edition
	 **/
	var $parts = array();

	/**
	 * @var Pointer to active part
	 **/
	var $activePart;	
	
	/**
	 * @var Array of elements of a part of an edition of a project
	 **/
	var $elements = array();

	/**
	 * @var Pointer to active part
	 **/
	var $activeElement;	

	/**
	 * @var Array of Field-Data Objects for a selected element
	 **/
	var $fields = array();	

	/**
	 * @var Data-Object of last Field
	 **/
	var $data;


	/**
	 * @var cache of field
	 **/
	static $cache = array();
	
	
	/**
	 * @var activeSession
	 *
	 **/
	var $activeSession;

	/**
	 * @var activeUsername
	 *
	 **/
	var $activeUsername;

	/**
	 * @var activeUsergroup
	 *
	 **/
	var $activeUsergroup;

	/**
	 * @var activeUserId
	 *
	 **/
	var $activeUserId;
	
	/**
	 * @var userRestriction
	 */
	var $userRestriction;

	/**
	 * @var accessTable
	 */
	var $accessTable = array(
		"Projects" => array(),
		"Editions" => array(),			
		"Templates" => array()
	);

	/**
	 * constructor
	 *
	 * @return void
	 * @author Urs Hofer
	 **/
	
	function rokfor_DB ($rootdir=false,$dbase=DBASE,$dbuser=DBUSER,$dbpass=DBPASS,$dbname=DBNAME) 
	{
		$this->db = new db($dbase,$dbuser,$dbpass,$dbname,phpversion());
		$this->db->appname='ROKFOR (core)';
		$this->accessTable = array(
			"Projects" => array(),
			"Editions" => array(),			
			"Templates" => array()
		);
	}
	
	function _esc($_str) {
		return $this->db->escape($_str);
	}
	
	
	function _stripslashes($_str) {
	//	return $_str;
		if (get_magic_quotes_gpc()) {return stripslashes($_str);}
		else return $_str;
	}

	/**
	 * query: wrapping query from the mysql-library
	 *
	 * @param string $q 
	 * @return query
	 * @author Urs Hofer
	 */
	function query($q) {
		return $this->db->query($q);
	}

	/**
	 * affected_rows: wrapping affected_rows from the mysql-library
	 *
	 * @param string $q 
	 * @return query
	 * @author Urs Hofer
	 */
	function affected_rows($q) {
		return $this->db->affected_rows($q);
	}
              
	/**
	 * num_rows: wrapping affected_rows from the mysql-library
	 *
	 * @param string $q 
	 * @return query
	 * @author Urs Hofer
	 */
	function num_rows($q) {
		return $this->db->num_rows($q);
	}

	/**
	 * insert_id: wrapping insert_id from the mysql-library
	 *
	 * @param string $i 
	 * @return void
	 * @author Urs Hofer
	 */
	function insert_id($i = false) {
		return $this->db->insert_id($i);
	}

	/**
	 * fetch_object: wrapping fetch_object from the mysql-library
	 *
	 * @param dblink $link 
	 * @return object
	 * @author Urs Hofer
	 */
	function fetch_object($link) {		
		return $this->db->fetch_object($link);
	}

	/**
	 * newSession
	 *
	 * @param string $user 
	 * @param string $pass 
	 * @return 
	 * @author Urs Hofer
	 */
	function newSession($user,$pass){
		$this->clearCache();
		$userdb = $this->db->query("SELECT * FROM users WHERE password = '".$this->_esc($pass)."' AND username = '".$this->_esc($user)."'");  
		$checkusr = $this->db->fetch_object($userdb);
		if ($checkusr->usergroup == false) {
			return false;
		}
		else {
			$this->activeUserGroup = $checkusr->usergroup;
			$tcontrol = time();
			$pid=posix_getpid(); 
			$this->activeSession = $tcontrol.$pid.(mt_rand(0,1000));
			$this->activeUserId = $checkusr->id;
			/* Delete old sessions with the same user (if not api-group) */
			if ($checkusr->usergroup<>'api') $deleteold = $this->db->query('DELETE FROM session WHERE userid = '.$this->activeUserId);
			$dbobj = $this->db->query("INSERT INTO session (session,userid,starttime,currenttime) VALUES (".$this->activeSession.", ".$this->activeUserId.", $tcontrol, $tcontrol)");
		}
		return (true);
	}

	/**
	 * checkSession: returns 1 (ok), 0 (not valid), -1 (expired)
	 *
	 * @param int $id 
	 * @return int
	 * @author Urs Hofer
	 */
	function checkSession($id, $dont_update_time = false){
		$this->clearCache();		


		// Get active session
		if ($id) $checkses = $this->db->query_first_object('SELECT * FROM session WHERE session = "'.$this->_esc($id).'" LIMIT 1');
		// Return false if no session under this id
		if (!is_object($checkses)) return(0);

		//get activ user
		$checkusr = $this->db->query_first_object('SELECT * FROM users WHERE id = '.$checkses->userid.' LIMIT 1');
		// return false if user does not exist
		if (!is_object($checkusr)) return(0);

		if (time() - $checkses->currenttime>3600) {
			$deleteold = $this->db->query('DELETE FROM session WHERE session = "'.$this->_esc($id).'"');
			return -1;
		}

		// Clean all old sessions
		$deleteold = $this->db->query('DELETE FROM session WHERE currenttime < "'.(time()-3600).'"');
		

		//update session time
		if (!$dont_update_time) $userdb = $this->db->query('UPDATE session SET currenttime = '.time().' WHERE session = '.$this->_esc($id).' LIMIT 1');

		// Store in class vars
		$this->activeSession = $checkses->session;
		$this->activeUsername = $checkusr->username;
		$this->activeUsergroup = $checkusr->usergroup;
		$this->activeUserId = $checkusr->id;	

		return 1;
	}
	
	/**
	 * getSessionId
	 *
	 * @return int
	 * @author Urs Hofer
	 */
	function getSessionId() {
		return $this->activeSession;
	}
	
	/**
	 * deletes a session with a certain id
	 *
	 * @param int $id 
	 * @return void
	 * @author Urs Hofer
	 */
	function cleanSessionId($id) {
		$this->clearCache();
		return ($deleteold = $this->db->query('DELETE FROM session WHERE session = "'.$this->_esc($id).'"'));		
	}				
	
	
	/**
	 * getUserName
	 *
	 * @return string
	 * @author Urs Hofer
	 */
	function getUserName() {
		return $this->activeUsername;
	}
	
	/**
	 * getUserGroup
	 *
	 * @return string
	 * @author Urs Hofer
	 */
	function getUserGroup() {
		return $this->activeUsergroup;
	}
	
	/**
	 * getUserId
	 *
	 * @return int
	 * @author Urs Hofer
	 */
	function getUserId() {
		return $this->activeUserId;
	}
	
	/**
	 * setUserRestriction restricts Projects / Issues / Contributions access only for the current activated user
	 *
	 * @param boolean $_state 
	 * @return void
	 * @author Urs Hofer
	 */
	function setUserRestriction($_state = true) {

		if ($this->getUserGroup() <> "user") {
			$this->userRestriction = false;
			return $this->userRestriction;
		}

		$this->userRestriction = $_state;

		$this->accessTable = Array(
			"Projects" => Array(),
			"Editions" => Array(),			
			"Templates" => Array()
		);

		/* Load User Table */

		$sql = "SELECT * FROM _rights WHERE (_foruser LIKE '%:>".$this->getUserId()."<:%' OR _foruser LIKE '".$this->getUserId()."<:%') ";
		$query = $this->db->query($sql);
		while ($rights = $this->db->fetch_object($query)) {
			// Get Templates
			foreach (explode('<::::::>',$rights->_fortemplate) as $singletmpId) {
				if ($singletmpId && !in_array($singletmpId,$this->accessTable["Templates"])) $this->accessTable["Templates"][] = $singletmpId; 
			}
			// Get Books
			foreach (explode('<::::::>',$rights->_forbook) as $singletmpId) {
				if ($singletmpId && !in_array($singletmpId,$this->accessTable["Projects"])) $this->accessTable["Projects"][] = $singletmpId; 
			}
			// Get Issues
			foreach (explode('<::::::>',$rights->_forissue) as $singletmpId) {
				if ($singletmpId && !in_array($singletmpId,$this->accessTable["Editions"])) $this->accessTable["Editions"][] = $singletmpId; 
			}
		}

	}
	
	/**
	 * returns the rights per group, does not store anything class wide
	 *
	 * @param int $_userid 
	 * @return array
	 * @author Urs Hofer
	 */
	function showGroupRights($_group) {
		$_accessTable = Array(
			"Templates" => Array(),
			"Projects" => Array(),
			"Editions" => Array(),			
			"Users" => Array()			
		);
		$sql = "SELECT * FROM _rights WHERE id =".intval($_group);
		$query = $this->db->query($sql);
		while ($rights = $this->db->fetch_object($query)) {
			// Get Templates
			foreach (explode('<::::::>',$rights->_fortemplate) as $singletmpId) {
				$_accessTable["Templates"][] = $singletmpId;
			}
			// Get Books
			foreach (explode('<::::::>',$rights->_forbook) as $singletmpId) {
				$_accessTable["Projects"][] = $singletmpId;
			}
			// Get Issues
			foreach (explode('<::::::>',$rights->_forissue) as $singletmpId) {
				$_accessTable["Editions"][] = $singletmpId;
			}
			// Get Users
			foreach (explode('<::::::>',$rights->_foruser) as $singletmpId) {
				$_accessTable["Users"][] = $singletmpId;
			}			
		}
		return $_accessTable;
	}

	/**
	 * returns the username by id. in case of a passed username, it will be checked and returned
	 *
	 * @param string $id_or_name 
	 * @return void
	 * @author Urs Hofer
	 */
	function getUserNameById($id_or_name) {
		$_name = $this->fetch_object($this->query("SELECT username FROM users WHERE id = '".$this->_esc($id_or_name)."' || username = '".$this->_esc($id_or_name)."' LIMIT 1"));												
		return $_name->username;
	}

	/**
	 * getProjects() - Loads all current Projects into the array
	 * requires the _books table
	 *
	 * @return $this->projects
	 * @author Urs Hofer
	 **/

	function getProjects($reset=false, $id=false) 
	{
		if ($reset) $this->activeProject = NULL;
		$sql = 'SELECT * FROM _books '.(($id)?("WHERE id = ".intval($id)." "):('')).'ORDER BY __sort__ ASC';
		$query = $this->db->query($sql);
		$this->projects = array();
		while ($row = $this->db->fetch_object($query)) {
			if (!$this->userRestriction) $this->projects[] = $row;
			elseif (in_array($row->id, $this->accessTable["Projects"])) $this->projects[] = $row;
		}
		return $this->projects;
	}

	/**
	 * setProjectActive - Sets a pointer to an active project
	 * requires $this->projects set
	 * If $id is passed, activate the project with id = $id, $id can also resemble the name of the Project
	 * Otherwise, activate first
	 *
	 * @return true, false on error
	 * @author Urs Hofer
	 **/

	function setProjectActive($id = false) 
	{
		// Some Preliminary Checks. If nothing is chosen so far, 
		// activate the first Project		
		if (!is_object($this->projects[0])) $this->getProjects();
		if (!$id){
			if (is_object($this->projects[0])) {
				$this->activeProject = &$this->projects[0]; return true;
			}
		}
		else foreach($this->projects as $key=>$val) if ($val->id==$id || $val->_name==$id) {
			$this->activeProject = &$this->projects[$key]; return true;
		}
		return false;
	}
	
	
	/**
	 * deletes an edition
	 *
	 * @param string $_issueid 
	 * @return void
	 * @author Urs Hofer
	 */
	function deleteEdition($_issueid) {
		$this->clearCache();
		$sql = ("DELETE FROM _issues WHERE `id` = ".intval($_issueid)." LIMIT 1"); 
		return $this->query($sql);
	}
	
	/**
	 * rename an edition
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function renameEdition($_issueid, $_name)
	{
		$this->clearCache();
		$_issueid = (int)$_issueid;
		$sql = ("UPDATE _issues SET 
			`_name` = '".$this->_esc($_name)."'
			 WHERE id = ".$_issueid." LIMIT 1");
		return $this->query($sql);		
	}
	
	/**
	 * Adds an Edition
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function addEdition($_name, $_book, $_note, $_user = false, $_sort = false, $_status = 'Open', $_opentime = 0, $_closetime = 0, $_config = 0, $_p1 = "", $_p2 = "", $_p3 = "", $_p4 = "", $_p5 = "") {
		$this->clearCache();
		if ($_opentime == 0) $_opentime = time();
		if ($_closetime == 0) $_closetime = $_opentime * 2;
		if (!$_user) $_user = $this->getUserId();
		if (!$_sort) {
			$_sort = 1;
			$_e = $this->getEditions($_book);
			if (is_array($_e)) {
				$_sort = count($_e)+1;
			}
		}
		$sql = "INSERT INTO _issues (`_name`, `_opendate`, `_closedate`, `_status`, `__config__`, `_infotext`, `_forbook`, `_singleplugin`,`_allplugin`,`_rtfplugin`,`_xmlplugin`,`_narrationplugin`,`__user__`, `__parentnode__`, `__sort__`) 
				VALUES ('".$this->_esc($_name)."', ".$_opentime.", ".$_closetime.", '".$this->_esc($_status)."', '".$this->_esc($_config)."', '".$this->_esc($_note)."', ".$this->_esc($_book).", '".$this->_esc($_p1)."', '".$this->_esc($_p2)."', '".$this->_esc($_p3)."', '".$this->_esc($_p4)."', '".$this->_esc($_p5)."', '".$this->_esc($_user)."', '-1', ".$this->_esc($_sort).")";
		if ($this->query($sql)) {
			// Add read write permission for the issue to the current user
			$newId = $this->insert_id();


			$sql = "SELECT * FROM _rights WHERE (_foruser LIKE '%:>".$this->getUserId()."<:%' OR _foruser LIKE '".$this->getUserId()."<:%') ";
			$query = $this->db->query($sql);
			$_updaterights = array();
			while ($rights = $this->db->fetch_object($query)) {
				$_oldrights = explode('<::::::>',$rights->_forissue);
				$_oldrights[] = $newId;
				$_updaterights[$rights->id] = join('<::::::>',$_oldrights);
			}
			foreach ($_updaterights as $_groupid => $_rightstring) {
				$this->db->query("UPDATE _rights SET _forissue = '".$_rightstring."' WHERE id = ".$_groupid." LIMIT 1");		
			}
		}
	}		
	
	/**
	 * getEditions() - gets all editions for a project with id = $id and status = $status
	 *
	 * @return $this->editions or false on error
	 * @author Urs Hofer
	 **/
	function getEditions ($id = false, $reset = false, $status = 'open')
	{
		if ($reset) $this->activeEdition = NULL;
		
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) 
			return self::$cache[__FUNCTION__][serialize(func_get_args())];
		
		if ($status) {
			if ((stristr($status, "open") && stristr($status, "close")) || stristr($status, "both")) {
				$status = 'open" || _status = "closed';
			}
			else if (stristr($status, "open")) {
				$status = 'open';
			}		
			else if (stristr($status, "closed")) {
				$status = 'closed';
			}
			else {
				$status = 'open';
			}
		}		
		$sql = 'SELECT * FROM _issues WHERE _forbook = '.intval((!$id)?($this->activeProject->id):($id)).' '.(($status)?('AND (_status = "'.$status.'")'):('')).' ORDER BY __sort__ ASC';
		$query = $this->db->query($sql);
		$this->editions = array();
		while ($row = $this->db->fetch_object($query)) {
			$row->_name = $this->_stripslashes($row->_name);
			$row->_infotext = $this->_stripslashes($row->_infotext);
			if (!$this->userRestriction) $this->editions[] = $row;
			elseif (in_array($row->id, $this->accessTable["Editions"])) $this->editions[] = $row;
		}
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $this->editions ? $this->editions : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}

	/**
	 * setEditionActive - Sets a pointer to an active edition
	 *
	 * @return true, false on error
	 * @author Urs Hofer
	 **/
	function setEditionActive($id = false)
	{
		// Some Preliminary Checks. If nothing is chosen so far, 
		// activate the first Project and the first Edition
		if (!is_object($this->activeProject)) $this->setProjectActive();
		$this->getEditions(false,false,false);		
		if (!$id){
			if (is_object($this->editions[0])) {
				$this->activeEdition = &$this->editions[0]; return true;
			}
		}
		else foreach($this->editions as $key=>$val) if ($val->id==$id || $val->_name==$id) {
			$this->activeEdition = &$this->editions[$key]; return true;
		}
		return false;
	}

	/**
	 * getParts() - gets all parts for a project with id = $id
	 * 				if $id not set, uses $this->activeProject
	 *
	 * @return $this->parts or false on error
	 * @author Urs Hofer
	 **/
	function getParts ($id = false)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = 'SELECT * FROM _formats WHERE _forbook = '.intval((!$id)?($this->activeProject->id):($id)).' ORDER BY __sort__ ASC';
		$query = $this->db->query($sql);
		$this->parts = array();
	//	$this->activePart = NULL;
		while ($row = $this->db->fetch_object($query)) $this->parts[] = $row;
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $this->parts ? $this->parts : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}

	/**
	 * getPartByName: returns the id of a part according to its name
	 * if projectid is set, the parts will be only searched within the book
	 * if it is set to false, the current will be used
	 * if it is set to all or _forbook, all books will be searched
	 *
	 * @return void
	 * @author Urs Hofer
	 **/
	function getPartByName ($name,$projectid=false)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT id FROM _formats WHERE _forbook = '.
			(($projectid=="_forbook"||$projectid=="all")?
			('_forbook'):
			(intval((!$projectid)?($this->activeProject->id):($projectid)))).' 
		AND _name = "'.$this->_esc($name).'"');
		$query = $this->db->query($sql);
		$row = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $row->id ? $row->id : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}

	/**
	 * getPartByName: returns the id of a part according to its name
	 *
	 * @return void
	 * @author Urs Hofer
	 **/
	function getPartById($id)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT * FROM _formats WHERE id = '.(int)$id);
		$query = $this->db->query($sql);

		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;		
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}

	/**
	 * setPartActive - Sets a pointer to an active part within an project
	 *
	 * @return true, false on error
	 * @author Urs Hofer
	 **/
	function setPartActive($id = false)
	{
		if (!is_object($this->parts[0])) $this->getParts();
		if (!$id){
			if (is_object($this->parts[0])) {
				return ($this->activePart = &$this->parts[0]);
			}
		}
		else foreach($this->parts as $key=>$val) if ($val->id==$id || $val->_name==$id) {
			return ($this->activePart = &$this->parts[$key]);
		}
		return false;
	}
	
	/**
	 * undocumented function
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function renameElement($_name, $_id)
	{
		$this->clearCache();
		$sql = ("UPDATE  `_contributions` SET  `_name` =  '".$this->_esc($_name)."' WHERE  `_contributions`.`id` = ".intval($_id)." LIMIT 1 ;");
		return ($this->query($sql));
	}
	
	
	/**
	 * addElement() - adds an element (contribution) to the database
	 * 			if partId and editionId are not set, the active parts/editionId will be used
	 * 
	 *	 
 	 * @return id of the new contribution or false on error
	 * @author Urs Hofer
	 */
	function addElement ($_contribname,$_templateid,$_chapterid=false,$_issueid=false) {
		$this->clearCache();

		if (!$_chapterid) {
			if (!$this->activePart) return false;
			$_chapterid = $this->activePart;
		}

		if (!$_issueid) {
			if (!$this->activeEdition) return false;
			$_issueid = $this->activeEdition;
		}

		$initTime= time();
		$sql = ("
			INSERT INTO `_contributions` (`id`, `_fortemplate`, `_forissue`, `_name`, `_status`, `_newdate`, `_moddate`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
			('', '".intval($_templateid)."', '".intval($_issueid)."', '".$this->_esc($_contribname)."', 'Open', $initTime, $initTime, '".$this->getUserId()."', NULL, '".intval($_chapterid)."', -1, 1);
		");
		$this->query($sql);
		$newId = $this->insert_id();
		$sql = ("
			UPDATE  `_contributions` SET  `__sort__` =  '$newId' WHERE  `_contributions`.`id` =$newId LIMIT 1 ;
		");
		$this->query($sql);

		// Prepare Cell Fields in Datatable...
		$_fields = $this->getTemplateFields($_templateid);
		$parentNode = 0;
		foreach ($_fields as $fields) {
			$sql_I = ("			
				INSERT INTO `_data` (`id`, `_forcontribution`, `_fortemplatefield`, `_datatext`, `_databinary`, `_datainteger`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
				('', '$newId', '".$fields->id."', NULL, NULL, NULL, '".$this->getUserId()."', NULL, NULL, -2, $parentNode);
			");
			$this->query($sql_I);
			$parentNode++;
		}
		return ($newId);
	}

	/**
	 * closeElement() - sets the _state of an Element to Close
	 * @return $this->parts or false on error
	 * @author Urs Hofer
	 **/
	function closeElement ($id)
	{
		$this->clearCache();
 		$this->db->query("UPDATE _contributions SET _status = 'Close' WHERE id = ".intval($id)." LIMIT 1");		
	}

	/**
	 * openElement() - sets the _state of an Element to Open
	 * @return $this->parts or false on error
	 * @author Urs Hofer
	 **/
	function openElement ($id)
	{		
		$this->clearCache();
 		$this->db->query("UPDATE _contributions SET _status = 'Open' WHERE id = ".intval($id)." LIMIT 1");		
	}
	
	/**
	 * openElement() - sets the _state of an Element to Open
	 * @return $this->parts or false on error
	 * @author Urs Hofer
	 **/
	function _deleteFromBinary($fieldId,$imgId=false) {
		$this->clearCache();
		$_field_info = $this->getFieldData($fieldId);
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
		$this->setFieldData($udb_update,$fieldId,'_databinary');
	}
		
	function deleteElement ($contribId)
	{
		$this->clearCache();
		
		// Delete Binaries
		$sql = ("SELECT id from _data WHERE _forcontribution = '".intval($contribId)."' and _databinary <> '' ");
		while ($todel = $this->fetch_object($this->query($sql))) {
			$this->_deleteFromBinary($todel->id);
		}
		// Delete Data
		$sql = ("DELETE from _data WHERE _forcontribution = '".intval($contribId)."'");
		$this->query($sql);
		// Delete Contribution
		$sql = ("DELETE from _contributions WHERE id = '".intval($contribId)."'");
		$this->query($sql);
	}	
	
	/**
	 * lastmodElementUpdate($id) - sets the _moddate to current time
	 * @return $this->parts or false on error
	 * @author Urs Hofer
	 **/
	function lastmodElementUpdate($id)
	{
		$this->clearCache();
		$initTime= time();
 		$this->db->query("UPDATE _contributions SET _moddate = $initTime WHERE id = ".intval($id)." LIMIT 1");		
	}	
	

	/**
	 * getElements() - gets all elements (contributions) for a part within an issue of a project
	 * 				if $id not set, uses $this->activePart, if $status not set uses 'Close'. If status = false neglects all status info.
	 * 				$ed: edition-id (issue-id). if not set, the active edition set with setEditionActive will be used.
	 *                   if $ed is set to "all", all issues are looked at.
	 * 				$order: "_name ASC" or "_newdate DESC" or "_name ASC, _moddate DESC" or false
	 *
	 * @return $this->parts or false on error
	 * @author Urs Hofer
	 **/
	function getElements ($id = false, $status = 'Close', $ed = false, $templatename = false, $order = false)
	{
		if (is_array($id)) {
			foreach ($id as &$i) {
				$i = intval($i);
			}
		}
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		if ($status != false) {
			if ((stristr($status, "Open") && stristr($status, "Close")) || stristr($status, "Both")) {
				$status = 'Open" || _contributions._status = "Close';
			}
			else if (stristr($status, "Open")) {
				$status = 'Open';
			}		
			else if (stristr($status, "Close")) {
				$status = 'Close';
			}
			else if (stristr($status, "Deleted")) {
				$status = 'Deleted';
			}				
			else {
				$status = 'Close';				
			}	
		}
		
		$ed = ($ed == '_contributions._forissue' || $ed == 'all') ? 
				'_contributions._forissue' : 
					intval((!$ed) ? ($this->activeEdition->id) : ($ed));
		
		$sql = 'SELECT _contributions.*, _issues._name as _issuename FROM _contributions 
		LEFT JOIN _templatenames ON _templatenames.id = _contributions._fortemplate
		LEFT JOIN _issues ON _issues.id = _contributions._forissue
		WHERE _contributions._forissue = '.$ed
		.
		(((!$id && !$this->activePart->id) || $id == '_contributions.__split__') 
			? ('')
			: (
				is_array($id)
					? (' AND (_contributions.__split__ IN ('.join(',',$id).'))')
					: (' AND (_contributions.__split__ = '.intval((!$id)?($this->activePart->id):($id)).')'
			  )
		))	
		.	
		(($status)?(' AND (_contributions._status = "'.$status.'")'):(''))
		.
		(($templatename)?(' AND (_templatenames._name = "'.$this->_esc($templatename).'")'):('')).
		(($this->userRestriction && $this->getUserGroup()=='user')?(' AND ((_templatenames._public = \'true\') OR  (_contributions.__user__ = "'.$this->getUserId().'" || _contributions.__user__ = "'.$this->getUserName().'"))'):('')).
		(($order)?(' ORDER BY '.$this->_esc($order)):(' ORDER BY id ASC'));
		$query = $this->db->query($sql);
		$this->elements = array();
		$this->activeElement = NULL;
		while ($row = $this->db->fetch_object($query)) $this->elements[] = $row;
		
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $this->elements ? $this->elements : array();	
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}
	

	/**
	 * getElementsByIssue - returns all contributions of an issue (unsorted) as array of objects
	 * 
	 * @param int $id 
	 * @return array
	 * @author Urs Hofer
	 */
	function getElementsByIssue ($_issueId, $_state = false, $_order = false, $_chapter = false) {
		if (!$_issueId) return false;
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		
		if ($_state) {
			if ((stristr($_state, "Open") && stristr($_state, "Close")) || stristr($_state, "both")) {
				$_state = "Open' || _contributions._status = 'Close";
			}
			else if (stristr($_state, "Open")) {
				$_state = 'Open';
			}		
			else if (stristr($_state, "Close")) {
				$_state = 'Close';
			}	
			else if (stristr($status, "Deleted")) {
				$_state = 'Deleted';
			}				
		}
		
		
		$sql = 'SELECT _contributions.* FROM _contributions WHERE _contributions._forissue = '.intval($_issueId);
		if ($_chapter) $sql .= " AND __split__ = '".$this->_esc($_chapter)."'";
		if ($_state) $sql .= " AND (_contributions._status = '$_state')";
		if ($_order) $sql .= ' ORDER BY '.$this->_esc($_order);
		$query = $this->db->query($sql);
		$this->elements = array();
		while ($row = $this->db->fetch_object($query)) $this->elements[] = $row;
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $this->elements;	
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}
	
	/**
	 * getElementsNamebyId() - returns a name of an element by id
	 *
	 * @return string or false on error
	 * @author Urs Hofer
	 **/
	function getElementsNamebyId ($id=false)
	{
		if (!$id) {
			if (!$this->activeElement) return false;
			$id = $this->activeElement;
		}		
		
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		
		$sql = 'SELECT * FROM _contributions 
			WHERE id='.intval($id).' LIMIT 1';
		$query = $this->db->query($sql);
		$retval = false;
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}	
	
	/**
	 * getElementsIdbyName() - returns the id of an element by name
	 *
	 * @return $int or false on error
	 * @author Urs Hofer
	 **/
	function getElementsIdbyName ($name, $templatename = false, $chaptername = false, $forissue = false, $status = false)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = 'SELECT _contributions.id FROM _contributions 
		LEFT JOIN _templatenames ON _templatenames.id = _contributions._fortemplate
		WHERE _contributions._name="'.$this->_esc($name).'" 
		AND (_contributions._status <> "Deleted")'
		.
		(($templatename)?(' AND (_templatenames._name = "'.$this->_esc($templatename).'")'):(' ')).
		(($chaptername)?(' AND (_contributions.__split__ = "'.$this->_esc($chaptername).'")'):(' ')).
		(($forissue)?(' AND (_contributions._forissue = "'.intval($forissue).'")'):(' ')).			
		(($status)?('AND (_contributions._status = "'.$this->_esc($status).'") '):('')).
		'LIMIT 1';
//		echo $sql;
		
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}	

	/**
	 * setElementActive - Sets a pointer to an active element within a part of an issue in a project
	 *
	 * @return true, false on error
	 * @author Urs Hofer
	 **/
	function setElementActive ($id = false)
	{
		if (!$id){
			if (is_object($this->elements[0])) {
				$this->activeElement = &$this->elements[0]; return true;
			}
		}
		else foreach($this->elements as $key=>$val) if ($val->id==$id || $val->_name==$id) {
			$this->activeElement = &$this->elements[$key]; return true;
		}
		return false;
	}


	/**
	 * getFieldsId - gets id's of all data fields for an element.
	 * if $id not set, uses the activeElement, otherwise the fields for Element with id = $id
	 * if $resolveForeign is false, no template infos are merged (which is way faster in big tables)
	 *
	 * @return $this->fields array of data-objects, false on error
	 * @author Urs Hofer
	 **/

	function getFieldsId($id = false) {return $this->getFieldsByElement($id);}
	function getFieldsByElement($id = false, $resolveForeign = true)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];

		/* Legacy Check: If ParentNode = -1, update the order */
		$query = $this->db->query('SELECT id FROM _data WHERE __parentnode__ = -1 AND _forcontribution = '.intval((!$id)?($this->activeElement->id):($id)).' LIMIT 1');
		$row = $this->db->fetch_object($query);
		if ($row->id) {
			$this->clearCache();
			$parentnode = -1;			
			$count = 0;
			$sql = 'SELECT _data.id FROM _data	WHERE _data.__parentnode__ = <%%parentnode%%> AND _data._forcontribution = '.intval((!$id)?($this->activeElement->id):($id)).' LIMIT 1';
			while ($parentnode) {
				$query = $this->db->query(str_replace('<%%parentnode%%>', $parentnode, $sql));
				$row = $this->db->fetch_object($query);
				if ($parentnode = $row->id) {
				 	$this->db->query("UPDATE _data SET __parentnode__ = NULL, __sort__ = $count WHERE id = ".$row->id." LIMIT 1");
				}
				$count++;
			}			
		}
		
		if ($resolveForeign) $sql = 'SELECT _data.id,_templates.id as t_id, _templates._fieldname, _templates._fieldtype, 
		_data._datatext as d_text,
		_data._databinary as d_binary,
		_data._datainteger as d_integer,
		_data.__parentnode__ FROM _data
		LEFT JOIN _templates ON _templates.id = _data._fortemplatefield
		WHERE _data._forcontribution = '.intval((!$id)?($this->activeElement->id):($id)).' ORDER BY _data.__sort__';

		else $sql = 'SELECT _data.id, _data._fortemplatefield as t_id,
		_data._datatext as d_text,
	    _data._databinary as d_binary,
	    _data._datainteger as d_integer,
		_data.__parentnode__ FROM _data
		WHERE _data._forcontribution = '.intval((!$id)?($this->activeElement->id):($id)).' ORDER BY _data.__sort__';
		
		$this->fields = array();
		$query = $this->db->query($sql);
		while ($row = $this->db->fetch_object($query)) {
			if ($row->id) {
				$row->d_text = $this->_stripslashes($row->d_text);
				if ($resolveForeign) $this->fields[$row->_fieldname] = $row;
				else  $this->fields[] = $row;
			}
		}
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $this->fields;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}
	
	/**
	 * returns all Field-Definitions of a certain Template Id.
	 *
	 * @param string $id 
	 * @return void
	 * @author Urs Hofer
	 */
	function getTemplateFields($id)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		/* Legacy Check */
		$sql = ("SELECT * from _templates WHERE _fortemplate = '".intval($id)."' AND __parentnode__ = -1 LIMIT 1");
		$row = $this->db->fetch_object($this->db->query($sql));
		if ($row->id) {
			$this->clearCache();			
			$next = true;
			$count = 0;
			while($next) {
				if ($row->id) {
					$retval[] = $fields;
				 	$this->db->query("UPDATE _templates SET __parentnode__ = NULL, __sort__ = $count WHERE id = ".$row->id." LIMIT 1");
					$sql = ("SELECT * from _templates WHERE _fortemplate = '".intval($id)."' AND __parentnode__ = ".$row->id." LIMIT 1");
					$row = $this->db->fetch_object($this->db->query($sql));
				}
				else $next=false;
				$count++;
			}
		}		
			
		$sql = ("SELECT * from _templates WHERE _fortemplate = '".intval($id)."' ORDER BY __sort__");
		$query = $this->db->query($sql);
		$retval = array();
		while($fields = $this->db->fetch_object($query)) {
			if ($fields->id) $retval[] = $fields;
		}		
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval;			
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}	
	
	
	/**
	 * getFieldsByName - returns the id of a field in an element/contribution
	 * by name
	 *
	 * @return object
	 * @author Urs Hofer
	 **/
	function getFieldsByName ($fieldname, $elementId = false)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = 'SELECT _data.id, 
		_templates.id as t_id, 
		_templates._fieldname as t_fieldname, 		
		_templates._fieldtype as t_fieldtype, 	
		_templates._colNames as t_colnames,				
		_data._datatext as d_text,
	    _data._databinary as d_binary,
	    _data._datainteger as d_integer,
		_contributions._name as c_name
		FROM _data
		LEFT JOIN _templates ON _templates.id = _data._fortemplatefield
		LEFT JOIN _contributions ON _contributions.id = _data._forcontribution
		WHERE _templates._fieldname = "'.$this->_esc($fieldname).'"
		AND _data._forcontribution = '.intval((!$elementId)?($this->activeElement->id):($elementId)).' LIMIT 1';
		$query = $this->db->query($sql);
		$row = $this->db->fetch_object($query);
		if ($row->id) {
			$row->d_text = $this->_stripslashes($row->d_text);			
			self::$cache[__FUNCTION__][serialize(func_get_args())] = $row;
		}
		else {
			self::$cache[__FUNCTION__][serialize(func_get_args())] = false;
		}
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Return Field Names and Info by TemplateName
	 *
	 * @param string $templatename 
	 * @return void
	 * @author Urs Hofer
	 */
	function getFieldNamesByTemplateName($templatename) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ("
			SELECT  _templates.id, _templates._fieldname, _templatenames._name as _templatename from _templates
		   	LEFT JOIN _templatenames ON _templates._fortemplate = _templatenames.id 
			WHERE  _templatenames._name LIKE '".$this->_esc($templatename)."'
			ORDER BY _templatenames._name ;
		");	
		$query = $this->db->query($sql);
		while ($row = $this->db->fetch_object($query)) $retval[] = $row;
		self::$cache[__FUNCTION__][serialize(func_get_args())] = count($retval)>=1 ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}

	/**
	 * getFieldByTemplateId
	 * returns the content of a field by it's template-id and the contribution id
	 *
	 * @return array[object]
	 * 
	 * @author Urs Hofer
	 */
	function getFieldByTemplateId ($fieldid, $contribution) {
        $_d = $this->getFieldsByTemplateId ($fieldid, false, $contribution, false, false);
        return reset($_d);		
	}


	/**
	 * getFieldsByTemplateId
	 * returns all UNIQUE Data Fields with a certain  template id ($fieldid) of a defined issue ($issue, if set!)
	 * and a certain contribution id ($contribution, if set!)
	 *
	 * @return array[object]
	 * 
	 * @author Urs Hofer
	 */
	function getFieldsByTemplateId ($fieldid, $issue = false, $contribution = false, $status = false, $group = true) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];

		$sql = ('SELECT _data.id, 
			_data._datatext as d_text,
			_data._databinary as d_binary,
			_data._datainteger as d_integer,
			_data._forcontribution			
			'.
			(($issue)?(",\n_contributions._name as _name, _contributions.id as c_id"):(''))
			.'
			FROM _data 
			'.
			(($issue)?('LEFT JOIN _contributions ON _data._forcontribution = _contributions.id'):(''))
			.'
			WHERE _data._fortemplatefield= '.intval($fieldid).' 
			'.
			(($issue)?('AND _contributions._forissue = '.intval($issue).' '):(''))
			.'
			'.
			(($contribution)?('AND _data._forcontribution = '.intval($contribution).' '):(''))
			.'				
			'.
			(($status)?('AND (_contributions._status = "'.$this->_esc($status).'") '):(''))
			.'	
			'.
			(($group)?(' GROUP BY _data._datatext '):(''))
			.'							
			
			ORDER BY _data._datatext;
		');
		$query = $this->db->query($sql);
		$retval = array();
		while ($row = $this->db->fetch_object($query)) {
			$row->d_text = $this->_stripslashes($row->d_text);
			$retval[] = $row;
		}
		self::$cache[__FUNCTION__][serialize(func_get_args())] = count($retval)>=1 ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];
	}


	/**
	 * getFieldData - get data of a field with template infos
	 * $id for a field is strictly required
	 *
	 *  Returned Data Object:	
	 * 
	 * 	(
	 * 	    [d_id] => Int
	 * 	    [d_forelement] => Int
	 * 	    [d_text] => String
	 * 	    [d_binary] => String
	 * 	    [d_integer] => Int
	 * 	    [t_id] => Int
	 * 	    [t_fortemplate] => Int
	 * 	    [t_fieldname] => String
	 * 	    [t_fieldtype] => String
	 * 	    [t_maxlines] => Int
	 * 	    [t_textlength] => Int
	 * 	    [t_imagewidth] => Int
	 * 	    [t_imageheight] => Int
	 * 	    [t_cols] => Int
	 * 	    [t_colnames] => String
	 * 	    [t_history] => String
	 * 	    [t_growing] => String
	 * 	    [t_len_influence] => String
	 * 	)
	 * 	
	 * 	
	 * @return $this->data
	 * @author Urs Hofer
	 **/
	function getFieldData($id)
	{
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return $this->data = self::$cache[__FUNCTION__][serialize(func_get_args())];
		
		if (!$id) die ();
		
		$sql = 'SELECT _data.id as d_id, 
		_data._forcontribution as d_forelement,
	    _data._datatext as d_text,
	    _data._databinary as d_binary,
	    _data._datainteger as d_integer,
	    _data._fortemplatefield as d_fortemplatefield,
	    _data.__parentnode__ as d_parentnode,		
		_templates.id as t_id,
		_templates._fortemplate as t_fortemplate,
		_templates._fieldname as t_fieldname,
		_templates._fieldtype as t_fieldtype,
		_templates._maxlines as t_maxlines,
		_templates._textlength as t_textlength,
		_templates._imagewidth as t_imagewidth,
		_templates._imageheight as t_imageheight,
		_templates._cols as t_cols,
		_templates._colNames as t_colnames,
		_templates._history as t_history,
		_templates._growing as t_growing,
		_templates._lengthInfluence as t_len_influence,
		_templatenames._name as t_name,
		_contributions._name as c_name,
		_contributions.id as c_id		
		FROM _data
		LEFT JOIN _templates ON _templates.id = _data._fortemplatefield
		LEFT JOIN _templatenames ON _templatenames.id = _templates._fortemplate
		LEFT JOIN _contributions ON _data._forcontribution = _contributions.id		
		WHERE _data.id = '.intval($id).' LIMIT 1';
		$this->data = NULL;
		$query = $this->db->query($sql);
		$row = $this->db->fetch_object($query);
		$row->d_text = $this->_stripslashes($row->d_text);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $this->data = $row;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}
	
	
	/**
	 * setFieldData sets the value of a field
	 *
	 * @param string $data 
	 * @param int $id 
	 * @param string $type 	
	 * @return void
	 * @author Urs Hofer
	 */
	function setFieldData($data,$id,$type){
		$this->clearCache();
		if (!(in_array($type, array("_datatext", "_databinary",	"_datainteger")))) die ('setFieldData: type must be either: "_datatext", "_databinary",	"_datainteger"');
		if (!$id) die ('setFieldData: id must be set!'); 
		$sql = "UPDATE _data SET $type = '".$this->_esc($data)."' WHERE id = ".intval($id)." LIMIT 1";
		return ($query = $this->db->unbuffered_query($sql));
	}
	
	/**
	 * getCloudData - gets id of a data-field, gets the data from the db and returns a split-cloud-array
	 *
	 * @return array data[word=>array[x,y]]
	 * @author Urs Hofer
	 **/
	function getCloudData($fieldId,$xmin=-100,$xmax=100,$ymin=-100,$ymax=100)
	{
		$this->getFieldData($fieldId);
		$rawdata = $this->data->d_text;
		return $this->_splitCloudData($rawdata,$xmin,$xmax,$ymin,$ymax);
	}	

	/**
	 * _splitCloudData - gets Data of a cloud, puts it into array and normalizes it
	 * 
	 * Data are stored in the database like this:
	 * asd<::::::>60<::::::>65<;;;;;;>fsd<::::::>122<::::::>71<;;;;;;>asdd<::::::>15<::::::>25<;;;;;;>
	 *
	 * @return array data[word=>array[x,y]]
	 * @author Urs Hofer
	 **/
	
	function _splitCloudData($rawdata,$xmin=-100,$xmax=100,$ymin=-100,$ymax=100)
	{
		
		/**
		 * Frontend: Size of Cloud X
		 **/
		if (!defined("FRNTEND_CLOUD_X")) define("FRNTEND_CLOUD_X", 338);
		/**
		 * Frontend: Size of Cloud Y
		 **/
		if (!defined("FRNTEND_CLOUD_Y")) define("FRNTEND_CLOUD_Y", 290);
		
		
		$return  = array();		
		foreach ($this->_splitClean("<;;;;;;>", $rawdata) as $entry) {
			list($key,$x,$y) = $this->_splitClean("<::::::>", $entry);
			//if (array_key_exists($key, $return))
			//{
			//	$count = 1;
			//	while ( array_key_exists($key."_#".$count, $return)) $count++;
			//	$key = $key."_#".$count;
			//}
			
			// 3d over time
			if (stristr($y,'<>')&&stristr($x,'<>')) {
				$_return = array();
				if (stristr($x,'<>')) {
					$_subvals_x = $this->_splitClean('<>', $x);
					foreach ($_subvals_x as $_subval_x) {
						list($_time,$_svx) = $this->_splitClean('_', $_subval_x);
						$_return[$_time][0]= (($xmax - $xmin)/FRNTEND_CLOUD_X*$_svx)+$xmin;
					}
				}

				if (stristr($y,'<>')) {
					$_subvals_y = $this->_splitClean('<>', $y);				
					foreach ($_subvals_y as $_subval_y) {
						list($_time,$_svy) = $this->_splitClean('_', $_subval_y);
						$_return[$_time][1]= (($ymax - $ymin)/FRNTEND_CLOUD_Y*$_svy)+$ymin;
					}
				}
		 		$return[$key] = $_return;				
			}

			// No 3d
			else {
		 		$return[$key] = array(array((($xmax - $xmin)/FRNTEND_CLOUD_X*$x)+$xmin,(($ymax - $ymin)/FRNTEND_CLOUD_Y*$y)+$ymin));
			}
		}
		return $return;
	}

	/**
	 * _splitClean
	 * like explode, but cleans the final array from empty results
	 *
	 * @return array
	 * @author Urs Hofer
	 **/
	function _splitClean($_pattern,$_string)
	{
		$ret = array();
		$expl = explode($_pattern, $_string);
		foreach ($expl as $val) if ($val<>"") $ret[] = $val;
		return $ret;
	}
	
	/**
	 * getSliderData - gets Data for a Slider, normalizes the value, returns it. 
	 *
	 * @return int
	 * @author Urs Hofer
	 **/
	function getSliderData($fieldId,$xmin=0,$xmax=100)
	{
		$this->getFieldData($fieldId);
		return (($xmax - $xmin)/100*$this->data->d_integer)+$xmin;
	}
	
	/**
	 * getMatrixData - gets Data of a Matrix, normalizes it and returns it as an array
	 * 
	 * @return array
	 * @author Urs Hofer
	 **/
	function getMatrixData($fieldId,$xmin=-100,$xmax=100,$ymin=-100,$ymax=100)
	{
		$this->getFieldData($fieldId);
		list($x,$y) = explode("<::::::>", $this->data->d_text);
		return array((($xmax - $xmin)/FRNTEND_MATRIX_X*$x)+$xmin, (($ymax - $ymin)/FRNTEND_MATRIX_Y*$y)+$ymin);
	}
	
	/**
	 * getWordlistData - gets the Words of a wordlist and returns them in an array
	 *
	 * @return array
	 * @author Urs Hofer
	 **/
	function getWordlistData ($fieldId)
	{
		$this->getFieldData($fieldId);
		return ($this->_splitWordListData($this->data->d_text));
	}	
	
	/**
	 * splits serialized string into parts
	 *
	 * @param string $_rawdata 
	 * @return void
	 * @author Urs Hofer
	 */
	function _splitWordListData($_rawdata) {
		if (substr($_rawdata, -8)=='<;;;;;;>') $_rawdata = substr($_rawdata, 0, -8);
		return (explode("<;;;;;;>", $_rawdata));
	}
	
	/**
	 * prepares raw text for rtf editing.
	 *
	 * @param string $fielddata 
	 * @return string
	 * @author Urs Hofer
	 */
	function _prepare_for_rtfediting($fielddata) {
		if (defined('STORE_HTML') && STORE_HTML) {
			return $fielddata;
		}

		// Make Custom Tags visible
//		$fielddata = str_replace('<fn--', '&lt;fn--', $fielddata);
//		$fielddata = str_replace('--fn>', '--fn&gt;', $fielddata);
//		$fielddata = str_replace('<!--', '&lt;!--', $fielddata);
//		$fielddata = str_replace('--!>', '--!&gt;', $fielddata);

		// New Lines
		$fielddata = str_replace(array("\n\n","\n"),array("</p>\n<p>&nbsp;</p>\n<p>", "</p>\n<p>"),$fielddata);	
		$fielddata = str_replace("\r","<br />",$fielddata);	

		// Add the root block
		$fielddata = '<p>'.$fielddata.'</p>';
		// Add Non Blocking spaces for multiple spaces
		$fielddata = preg_replace('/( {2,}|^ )/em', 'str_repeat("&nbsp;", strlen("\1"))', $fielddata);


		// Preserve Tags

		$tg = array('<h5>', '<h6>', '</h5>', '</h6>', '</p>', '<br />','<u>','<i>','<b>','<blockquote>','<ul>','<li>','<ol>','</u>','</i>','</b>','</blockquote>','</ul>','</li>','</ol>','</a>');
		$pr = array('¿--h5--¿', '¿--h6--¿', '¿--/h5--¿', '¿--/h6--¿', '¿--/p--¿', '¿--br /--¿','¿--u--¿','¿--i--¿','¿--b--¿','¿--blockquote--¿','¿--ul--¿','¿--li--¿','¿--ol--¿','¿--/u--¿','¿--/i--¿','¿--/b--¿','¿--/blockquote--¿','¿--/ul--¿','¿--/li--¿','¿--/ol--¿','¿--/a--¿');

		// <a href=""><img src=""><span*>
		$fielddata = preg_replace('/<a(.*?)>/m', '¿--a$1--¿', $fielddata);
		$fielddata = preg_replace('/<img src="(.*?)">/m', '¿--img src="$1"--¿', $fielddata);
		$fielddata = preg_replace('/<span(.*?)>/m', '¿--span$1--¿', $fielddata);
		$fielddata = preg_replace('/<\/span(.*?)>/m', '¿--/span$1--¿', $fielddata);
		$fielddata = preg_replace('/<p(.*?)>/m', '¿--p$1--¿', $fielddata);
		$fielddata = preg_replace('/<iframe(.*?)>/m', '¿--iframe$1--¿', $fielddata);
		$fielddata = preg_replace('/<\/iframe(.*?)>/m', '¿--/iframe$1--¿', $fielddata);

		// Page Break
		$fielddata = preg_replace('/<!--pagebreak--!>/m', '¿--div style="page-break-after: always;"--¿¿--span style="display: none;"--¿&nbsp;¿--/span--¿¿--/div--¿', $fielddata);

		// HR
		$fielddata = preg_replace('/<hr(.*?)>/m', '¿--hr--¿', $fielddata);


		$fielddata = str_replace($tg, $pr, $fielddata);

		// Convert Other < / >
		$fielddata = str_replace('<', '&amp;lt;', $fielddata);
		$fielddata = str_replace('>', '&amp;gt;', $fielddata);

		// Redo Tags
		$fielddata = str_replace('¿--', '<', $fielddata);
		$fielddata = str_replace('--¿', '>', $fielddata);	

		// Clean up <p> and </p> before/after other block-level tags
		$fielddata = preg_replace('/<p>\s*(<ul>|<ol>|<blockquote>|<h5>|<h6>|<hr>|<div)/m', '$1', $fielddata);
		$fielddata = preg_replace('/(<\/ul>|<\/ol>|<\/blockquote>|<\/h5>|<\/h6>|<hr>|<\/div>)\s*<\/p>/m', '$1', $fielddata);
		// Add. a new open paragraph a block-level tag...
		$fielddata = preg_replace('/(<\/ul>|<\/ol>|<\/blockquote>|<\/h5>|<\/h6>|<hr>|<\/div>)/m', '$1<p>', $fielddata);

		return $fielddata;	
	}	

	/**
	 * splits serialized multi field text string into parts
	 *
	 * @param string $_rawdata 
	 * @return void
	 * @author Urs Hofer
	 */
	function _splitTextListData($_rawdata) {
		if (substr($_rawdata, -8)=='<::::::>') $_rawdata = substr($_rawdata, 0, -8);
		return (explode("<::::::>", $_rawdata));
	}

	/**
	 * splits serialized table data into parts. with $_force_col_count a number of columns can be
	 * forced, even if there is not enough data in the string. (creating empty cells)
	 *
	 * @param string $_rawdata 
	 * @param int $_force_col_count 	
	 * @return void
	 * @author Urs Hofer
	 */
	function _splitTableData($_rawdata,$_force_col_count=false) {
		$_array = explode('<;;;;;;>',$_rawdata);
		$_retval = Array();
		foreach ($_array as $_i) if ($_i) {
			$_row = array();
			if ($_force_col_count>0) {
				$_row_raw = explode('<::::::>',$_i);				
				for ($i=0; $i < $_force_col_count; $i++) { 
					$_row[$i] = $_row_raw[$i]?$_row_raw[$i]:"";
				}
			}
			else {
				$_row = explode('<::::::>',$_i);
			}
			foreach ($_row as &$_cell) $_cell = str_replace(array("&lt;::::::&gt;","&lt;;;;;;;&gt;"), array("<::::::>","<;;;;;;>"), $_cell);
			$_retval[] = $_row;
		}
		return ($_retval);
	}

	/**
	 * serializes 2d-array into <::::::> / <;;;;;;> form string
	 *
	 * @param string $_data 
	 * @return void
	 * @author Urs Hofer
	 */
	function _glueTableData($_data) {
		$_retval = Array();
		foreach ($_data as $_i) if (is_array($_i)) $_retval[] = implode('<::::::>',str_replace(array("<::::::>","<;;;;;;>"), array("&lt;::::::&gt;","&lt;;;;;;;&gt;"), $_i));		
		return (implode('<;;;;;;>',$_retval));
	}


	/**
	 * array_emtpy checks if an array is empty
	 *
	 * @param string $mixed 
	 * @return void
	 * @author Urs Hofer
	 */
	function array_empty($mixed) {
		if(empty($mixed[0])) return false;
		else return true;
	}


	/**
	 * Returns the template name of an element (contribution) with id $id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getTemplateObjectByElementId($id) {return $this->getTemplateNamebyId($id);}
	function getTemplateNamebyId($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		
		$elm = $this->getElementsNamebyId($id);
		$sql = 'SELECT * FROM _templatenames 
			WHERE id='.intval($elm->_fortemplate).' LIMIT 1';
		$query = $this->db->query($sql);
		$retval = false;
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * returns the Templates accessible for the user for the edition $id
	 * if all=true, all templates will be returned
	 *
	 * @param  int $id 
	 * @return array
	 * @author Urs Hofer
	 */
	function getTemplates($project=false,$part=false, $all=false) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		
		$part = (!$part)?($this->activePart->id):($part);				// Chapter
		$project = (!$project)?($this->activeProject->id):($project);	// Book
		
		if ($all) $sql = ("SELECT *  FROM _templatenames   ORDER BY __sort__");
		else $sql = ("SELECT *
				  FROM _templatenames 
				  WHERE (_forbook LIKE '%:>".$this->_esc($project)."<:%' OR _forbook LIKE '".$this->_esc($project)."<:%')
				  ".(($part=="all")?(''):("AND (_inchapter LIKE '%:>".$this->_esc($part)."<:%' OR _inchapter LIKE '".$this->_esc($part)."<:%')"))."				
				  ORDER BY _category,_name");
		$query = $this->db->query($sql);		

		$this->templates = array();
		while ($row = $this->db->fetch_object($query)) {
			if (!$this->userRestriction) $this->templates[] = $row;
			elseif (in_array($row->id, $this->accessTable["Templates"])) $this->templates[] = $row;
		}
		self::$cache[__FUNCTION__][serialize(func_get_args())] = count($this->templates)>0?$this->templates:false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the part name of an element (contribution) with id $id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getPartNamebyId($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$elm = $this->getElementsNamebyId($id);
		$sql = ('SELECT * FROM _formats WHERE  id = "'.intval($elm->__split__).'"');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the project name by its id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getProjectName($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT _name FROM _books
			 	WHERE  id = "'.intval($id).'"
				LIMIT 1');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the project id by its name
	 *
	 * @param string $name
	 * @return object
	 * @author Urs Hofer
	 */
	function getProjectId($name) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT id FROM _books
			 	WHERE  _name = "'.$this->_esc($name).'"
				LIMIT 1');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the project name of an part (chapter) with id $id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getProjectNamebyPartId($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT _books._name, _books.id FROM _books
				LEFT JOIN _formats ON _books.id = _formats._forbook
			 	WHERE  _formats.id = "'.intval($id).'"
				LIMIT 1');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the project name of an issue (edition) with id $id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getProjectNamebyEditionId($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT _books._name, _books.id FROM _books
				LEFT JOIN _issues ON _books.id = _issues._forbook
			 	WHERE  _issues.id = "'.intval($id).'"
				LIMIT 1');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the edition name of an element (contribution) with id $id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getEditionNamebyId($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$elm = $this->getElementsNamebyId($id);
		$sql = ('SELECT * FROM _issues WHERE  id = "'.intval($elm->_forissue).'"');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}

	/**
	 * Returns the edition info by edition id
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getEditionInfo($id) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT * FROM _issues WHERE  id = "'.intval($id).'" LIMIT 1');
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}



	/**
	 * Returns the edition id by Editionname
	 *
	 * @param int $id 
	 * @return object
	 * @author Urs Hofer
	 */
	function getEditionIdbyEditionname($name, $bookid = false) {
		if (self::$cache[__FUNCTION__][serialize(func_get_args())]) return self::$cache[__FUNCTION__][serialize(func_get_args())];
		$sql = ('SELECT id FROM _issues WHERE  _name = "'.$this->_esc($name).'"');
		if ($bookid) {
			$sql .= ('AND _forbook = '.intval($bookid));
		}
		$query = $this->db->query($sql);
		$retval = $this->db->fetch_object($query);
		self::$cache[__FUNCTION__][serialize(func_get_args())] = $retval ? $retval->id : false;
		return self::$cache[__FUNCTION__][serialize(func_get_args())];		
	}



	/**
	 * clearCache - resets the data cache
	 *
	 * @return void
	 * @author Urs Hofer
	 **/	
	function clearCache() {
		self::$cache = array();
	}
} // END class 




?>