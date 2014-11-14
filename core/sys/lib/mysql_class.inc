<?php
// $Id: mysql_class.inc 641 2007-12-14 16:48:29Z u.hofer $
//
//      MySQL-standard class to use in any project.
//

/*
   VARIOMAT - the web based content manager.
   Copyright (C) 2001-2005  Urs Hofer, Tr51.
   u.hofer@tr51.org / http://www.tr51.org
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/



class db {
  ///------------------------------------------variables------------------------------

  /////
  // var-definition
  // global vars
  /////

  var $link_id     = 0;
  var $query_id    = 0;
  var $record      = array();
  var $errdesc     = "";
  var $errno       = 0;
  var $version     = "";
  var $show_error  = 1;
   
  var $server      = "";
  var $user        = "";
  var $password    = "";
  var $database    = "";
 
  var $use_unbuffered_query = true;

  var $appname  = "MySQL-Class (unknown project)";
  
  ///------------------------------------------constructor----------------------------
  
  /////
  // constructor
  /////

  function db($server,$user,&$password,$database,$phpversion) {
    $this->server=$server;
    $this->user=$user;
    $this->password=$password;
    $this->database=$database;
    
    $password = "";
    
    $this->connect();
}    

  ///------------------------------------------functions------------------------------
    
  function connect() {
    $this->link_id=@mysql_connect($this->server,$this->user,$this->password);
    $this->password = "";
//	mysql_query("SET NAMES 'utf8' COLLATE 'utf8_unicode_ci'",$this->link_id);
    if (!$this->link_id) $this->error("Link-ID == false, connect failed");
    if ($this->database!="") $this->select_db($this->database);
  }
 
  function close($query_id=-1) {
    if ($query_id!=-1) $this->query_id=$query_id;
    return @mysql_close($this->query_id);
  }

  function geterrdesc() {
    $this->error=mysql_error();
    return $this->error;
  }
  
  function escape($str) {
  	return mysql_escape_string($str);
  }
  
  function geterrno() {
    $this->errno=mysql_errno();
    return $this->errno;
  }
 
  function getversion() {
    if($this->link_id) list($this->version)=$this->query_first("SELECT VERSION()",0,0,MYSQL_BOTH,0);
    if(!$this->version) $this->version="unknown";
    return $this->version;
  }

  function select_db($database="") {
    if ($database!="") $this->database=$database;
    if(!@mysql_select_db($this->database, $this->link_id)) $this->error("cannot use database ".$this->database);
  }
  
  function query($query_string,$limit=0,$offset=0,$showerror=1) {
    if($limit!=0) $query_string.=" LIMIT $offset, $limit";
    $this->query_id = mysql_query($query_string,$this->link_id);
    if ($showerror==1 && !$this->query_id) $this->error("Invalid SQL: ".$query_string);
    return $this->query_id;
  }
 
  function unbuffered_query($query_string,$LOW_PRIORITY=0,$limit=0,$offset=0,$showerror=1) {
    if(!$this->use_unbuffered_query) return $this->query($query_string,$limit,$offset,$showerror);
    else {
      //if($LOW_PRIORITY==1) $query_string=preg_replace("/^(INSERT|UPDATE|DELETE|REPLACE)(.*)/si", "\\1 LOW_PRIORITY\\2", $query_string);
      if($limit!=0) $query_string.=" LIMIT $offset, $limit";
      $this->query_id = mysql_unbuffered_query($query_string,$this->link_id);
      if ($showerror==1 && !$this->query_id) $this->error("Invalid SQL: ".$query_string);
      return $this->query_id;
    }
  }

  function fetch_array($query_id=-1,$type=MYSQL_BOTH) {
    if ($query_id!=-1) $this->query_id=$query_id;
    $this->record = mysql_fetch_array($this->query_id, $type);
    return $this->record;
  }

  function fetch_object($query_id=-1) {
    if ($query_id!=-1) $this->query_id=$query_id;
    if (is_resource($this->query_id))  
        $this->record = mysql_fetch_object($this->query_id);
    else
        $this->record = false;
    return $this->record;
  }

  function fetch_assoc($query_id=-1) {
    if ($query_id!=-1) $this->query_id=$query_id;
    $this->record = mysql_fetch_assoc($this->query_id);
    return $this->record;
  }
 
  function fetch_row($query_id=-1) {
    if ($query_id!=-1) $this->query_id=$query_id;
    $this->record = mysql_fetch_row($this->query_id);
    return $this->record;
  }

  function query_first($query_string,$limit=0,$offset=0,$type=MYSQL_BOTH,$showerror=1) {
    $this->query($query_string,$limit,$offset,$showerror=1);
    $returnarray=$this->fetch_array($this->query_id,$type);
    return $returnarray;
  }

  function query_first_object($query_string,$limit=0,$offset=0,$showerror=1) {
    $this->query($query_string,$limit,$offset,$showerror=1);
    $returnarray=$this->fetch_object($this->query_id);
    return $returnarray;
  }

  function num_rows($query_id=-1) {
    if ($query_id!=-1) $this->query_id=$query_id;
    return @mysql_num_rows($this->query_id);
  }
 
  function affected_rows() {
    return @mysql_affected_rows($this->link_id);
  }
 
  function insert_id() {
    return @mysql_insert_id($this->link_id);
  }

  function list_tables() {
    return @mysql_list_tables($this->database);
  }

  function tablename($obj_id,$int_i) {
    return @mysql_tablename($obj_id,$int_i);
  }

  function result($obj_id,$int_i) {
    return @mysql_result($obj_id,$int_i);
  }

  function error($errormsg) {
    $this->errdesc=mysql_error();
    $this->errno=mysql_errno();
    
    $errormsg="<b>Database error in $this->appname:</b> $errormsg\n<br>";
    $errormsg.="<b>mysql error:</b> $this->errdesc\n<br>";
    $errormsg.="<b>mysql error number:</b> $this->errno\n<br>";
    $errormsg.="<b>mysql version:</b> ".$this->getversion()."\n<br>";
    $errormsg.="<b>php version:</b> ".phpversion()."\n<br>";
    $errormsg.="<b>Date:</b> ".date("d.m.Y @ H:i")."\n<br>";
    $errormsg.="<b>Script:</b> ".getenv("REQUEST_URI")."\n<br>";
    $errormsg.="<b>Referer:</b> ".getenv("HTTP_REFERER")."\n<br><br>";

    if($this->show_error) {
		$_error = debug_backtrace (); 
		$_mesg = "<p>".$errormsg."</p>";
		$_mesg .= "<p><pre>".htmlentities(print_r($_error,true))."</pre></p>";		 
   	    die('<div style="width:100%; height: 100%; overflow: scroll">'.$_mesg.'</div');	
    }
    else {
    	echo ("<!-- My SQL Error: Debug Mode prohibited! //-->");
    }
  }
 
  function data_seek($result,$nr)
  {
    if(!mysql_data_seek($result,$nr)) $this->error("Can't seek to row $i in result");
  }

  //giving back tha fields of a table
  function get_fields($tab)
  {
    $fields=$this->query('SHOW COLUMNS FROM '.$tab);
    while ($row=$this->fetch_array($fields))
      $result[]=$row['Field'];  

    return $result;
  }

}
?>