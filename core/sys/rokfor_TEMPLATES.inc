<?php

/**
 * undocumented class
 *
 * @package default
 * @author Urs Hofer
 **/

class rokfor_TEMPLATES
{
	/**
	 * rfDB: Instance of rokfor_DB-Class
	 *
	 * @var class
	 */
	var $rfDB = null;
	
	/**
	 * caller: Instance of rokfor_MAIN-Class
	 *
	 * @var class
	 */
	var $caller = null;	
	
	/**
	 * output buffer. contains all html output
	 * @var array
	 */	
	var $buffer;

	/**
	 * searchstring: regular expression after a search. can be used to mark results in text fields
	 *
	 * @var string
	 */
	var $searchstring;

	/**
	 * customcontrol: stores the post array transmitted by individual (custom) selectors for some form fields
	 *
	 * @var array
	 */
	var $customcontrol;

	
	function rokfor_TEMPLATES (&$_db, &$buffer, &$caller) {
		$this->rfDB = &$_db;
		$this->caller = &$caller;
		$this->buffer = &$buffer;
		$this->startTime = microtime(true);
		touch(LOGDIR."/".PROJECT_NAME."_custom_dictionary.pws");
		$this->pspell = @pspell_new_personal(LOGDIR."/".PROJECT_NAME."_custom_dictionary.pws", 'de','','','utf-8',PSPELL_FAST);
		$this->customcontrol = array();
	}
	
	
	function _stripslashes($_str) {
		if (get_magic_quotes_gpc()) {return stripslashes($_str);}
		else return $_str;
	}
	
	/**
	 * Returns the somehow quirky influence array in a correct javascript array notation.
	 * the string has the form of factor%%%%%fieldname<::::::>...
	 *
	 * @param string $_inf 
	 * @param string $_contrib 
	 * @return void
	 * @author Urs Hofer
	 */
	function _ConvertInfluenceArray($_inf,$_f) {
		// Transform Influence Array
		$_inf_array = "[";
		foreach (explode('<::::::>', $_inf) as $_value) {
			list($_factor,$_fieldname) = explode("%%%%%", $_value);
			if ($_fieldname) {
				$_id = $this->rfDB->getFieldsByName ($_fieldname, $_f->c_id);
				if ($_id->t_id) $_inf_array .= '[["field'.$_id->t_id.'"],['.(($_f->t_id<>$_id->t_id)?($_factor):(1)).']],';
			}
		}
		$_inf_array = rtrim($_inf_array,',');
		$_inf_array .= "]";
		return ($_inf_array);				
	}

	/**
	 * spell Check Call Back Function
	 *
	 * @param string $word 
	 * @return void
	 * @author Urs Hofer
	 */
	function __spellCheckWord($word) {
		static $tagcheck=false;
		// Tag Handling
		// Ignore Complete Tags
	    if ($word[1] == '<' && $word[2] == '>') {
			return $word[0];
		}
		// Ignore Complete Ending Tags, switch checker mote on
	    if ($word[1] == '/' && $word[2] == '>') {
			$tagcheck = false;
			return $word[0];
		}
		// For opening Tags, switch ignore flag to true
	    if ($word[1] == '<') {
			$tagcheck = true;
		}
		if ($tagcheck)
			return $word[0];		
	    // Take the string match from preg_replaceace_callback's array
	    $word = $word[0];
	    // Ignore ALL CAPS
	    if (preg_match('/^[A-Z]*$/',$word)) return $word;
	    // Return dictionary words
	    if (pspell_check($this->pspell,$word))
	        return $word;

	    // No suggestions, color red
	    return '<span comment="rokfor_temp" class="correx">'.$word.'</span>';
	}

	/**
	 * Spell Check Initialization
	 *
	 * @param string $string 
	 * @return void
	 * @author Urs Hofer
	 */
	function _spellCheck($string) {
		$_words = preg_split("/([^\wÄäÖöÜüàéèÀÉÈçîô])/u", $string, NULL, PREG_SPLIT_DELIM_CAPTURE);
		foreach ($_words as $key=>$word) {
			
			if (stristr($word,"<")||substr($word, 0,1)=='&') $tagopen = true;
			
			if (!preg_match("/([^\wÄäÖöÜüàéèÀÉÈçîô])/u", $word) && !$tagopen) {
				if (pspell_check($this->pspell,$word))
			        $_text .= $word;
				else
			    	$_text .= '<span comment="rokfor_temp" class="correx">'.$word.'</span>';
			}
			else $_text .= $word;
			
			if (stristr($word,">")||substr($word, -1,1)==';') $tagopen = false;			
			
		}
//		return preg_replace_callback('/ ([\wÄäÖöÜüàéèÀÉÈçîô])[ \.,!?;:]/u',array('rokfor_TEMPLATES', '__spellCheckWord'),$string);
		return $_text;
	}
	
	/**
	 * parsing legends string (deriving from variomat field)
	 *
	 * @param string $string 
	 * @return array
	 * @author Urs Hofer
	 */
	function _parseLegends($string, $field=false, $issueid = false) {

		if (!$issueid) {
			$issueid = $_SESSION['issueid'];
		}


		/* Legacy mode: Split by ;, no parsing */

		if (!stristr($string,"=")) {
			return explode(';',$string);
		}

		$retval = array();
		$init = parse_ini_string ($string,true);
		if (!is_array(reset($init))) {
			$_init = $init;
			$init = array();
			$init[] = $_init;
		}



		$restrict_to_issue = false;
		$threeDee = false;						
		foreach ($init as $type=>$row) {
			$default = explode(';',trim($row['legends']));
			$history_command = $row['history'];
			$dateformat = $row['dateformat'];
			$fromtemplate = $row['fromtemplate'];
			$fromfield = $row['fromfield'];
			$fixedvalues =  explode(';',trim($row['values']));	
			$restrict_to_issue = false;

			/* show only open contribs */
			if ($row['restrict_to_open']==="true") $openstate = 'Open';
			else $openstate = 'Open" || _contributions._status = "Close';

			if ($row['restrict_to_issue']==="true") {
				$restrict_to_issue = true;
			}
			elseif ($row['restrict_to_issue']==="false") {
				$restrict_to_issue = false;			
			}			
			else if ($row['restrict_to_issue']){
				$restrict_to_issue = true;
				$fromissue = $this->rfDB->getEditionIdbyEditionname($row['restrict_to_issue'], $row['restrict_to_book'] === "true" ? $_SESSION['bookid'] : false);			
			}
			
			$restrict_to_chapter = $row['restrict_to_chapter']
									? $this->rfDB->getPartByName($row['restrict_to_chapter'],  $_SESSION['bookid'] && $row['restrict_to_book'] === "true" ? $_SESSION['bookid'] : 'all')
									: '_contributions._forchapter';
						
			$threeDee = $row['3d'];

			if ($history_command == "books") 
			{
				$description = HistoryBooks;	
				/* show only open contribs */
								
				$__select = $this->rfDB->getProjects();

				$_select = array();
				foreach ($__select as $select) {
					$_select[$select->id]->id = $select->id;
					$_select[$select->id]->_value = $select->_name;
				}				
			}

			if ($history_command == "issues") 
			{
				$description = HistoryIssue;	
				/* show only open contribs */
				if ($row['restrict_to_open']==="true") $openstate = 'open';
				else $openstate = 'open" || _status = "closed';
								
				$__select = $this->rfDB->getEditions($this->rfDB->getProjectId($row['frombook'])->id, false, $openstate);

				$_select = array();
				foreach ($__select as $select) {
					$_select[$select->id]->id = $select->id;
					$_select[$select->id]->_value = $select->_name;
				}				
			}
			if ($history_command == "chapters") 
			{
				$description = HistoryChapter;	
								
				$__select = $this->rfDB->getParts($this->rfDB->getProjectId($row['frombook'])->id);

				$_select = array();
				foreach ($__select as $select) {
					$_select[$select->id]->id = $select->id;
					$_select[$select->id]->_value = $select->_name;
				}				
			}
			if ($history_command == "self") 
			{
				$description = HistoryOwn;				
				$__select = $this->rfDB->getElements (false, false, $restrict_to_issue?($fromissue?$fromissue:$issueid):'_contributions._forissue', $field->t_name);
				$_select = array();
				foreach ($__select as $select) {
					if ($field->t_fieldname) $_select[] = $this->rfDB->getFieldsByName ($field->t_fieldname, $select->id);
				}				
				foreach ($_select as $key => $value) $_select[$key]->_value = $value->d_content;		

			}
			// Lädt alle Daten eines anderen Feldes eines anderen Templates in die History
			if ($history_command == "other") 
			{
				if (!$fromtemplate) $this->buffer .= (FromTemplateErr);
				if (!$fromfield) $this->buffer .= (FromFieldErr);
				$description = str_replace(array('$1','$2'),array($fromfield,$fromtemplate),HistoryOther);				
				$__select = $this->rfDB->getElements (false, $openstate, $restrict_to_issue?($fromissue?$fromissue:$issueid):'_contributions._forissue', $fromtemplate);
				$_select = array();
				foreach ($__select as $select) {
					if ($fromfield) $_select[] = $this->rfDB->getFieldsByName ($fromfield, $select->id);
				}				
				foreach ($_select as $key => $value) $_select[$key]->_value = $value->d_content;			
			}
			// Lädt alle Daten eines anderen Feldes eines anderen Templates in die History
			if ($history_command == "cloud") 
			{
				$description = HistoryOwn;					
				$__select = $this->rfDB->getElements (false, $openstate, $restrict_to_issue?($fromissue?$fromissue:$issueid):'_contributions._forissue', $field->t_name);
				$_select = array();
				foreach ($__select as $select) {
					if ($field->t_fieldname) $_value = $this->rfDB->getFieldsByName ($field->t_fieldname, $select->id);
					$words = explode('<;;;;;;>',$_value->d_content);
					foreach ($words as $_words) {
						$keyword = explode('<::::::>', $_words);
						if ($keyword[0]) {
							$_select[$keyword[0]] = (object)array("id"=>$keyword[0]."<::::::>0<::::::>0","_value"=>$keyword[0]);
						}
					} 
				}				
				ksort($_select);
			}						
			// Lädt alle Bezeichner der Contributions eines spezifischen Templates in die History
			if ($history_command == "contributional") 
			{
				if (!$fromtemplate) $this->buffer .= (FromTemplateErr);				
				$_select = $this->rfDB->getElements ($restrict_to_chapter, $openstate, $restrict_to_issue?($fromissue?$fromissue:$issueid):'_contributions._forissue', $fromtemplate, '_contributions._name ASC');
				$description = str_replace(array('$1'),array($fromtemplate),HistoryContributional);				
				foreach ($_select as $key => $value) $_select[$key]->_value = $value->_name.(!$row['restrict_to_chapter']?" &rarr; ".$value->_issuename:'');						
			}
			// Lädt alle Felder eines Templates in die History
			if ($history_command == "structural") {
				if (!$fromtemplate) $this->buffer .= (FromTemplateErr);				
				$description = str_replace(array('$1'),array($fromtemplate),HistoryStructural);		
				$_select = $this->rfDB->getFieldNamesByTemplateName($fromtemplate);
				foreach ($_select as $key => $value) $_select[$key]->_value = $value->_fieldname;
			}
			if ($history_command == "fixed")
			{
				if (!$fixedvalues) $this->buffer .= (FixedValuesErr);
				$_select = array();
				$description = FixedHistory;		
				foreach ($fixedvalues as $key => $value) {
					$_select[] = (object)array("id"=>$key,"_value"=>$value);
				}
			}
			$retval[$type] = array($default,$history_command,$fromtemplate,$fromfield,$fixedvalues,$restrict_to_issue,$threeDee,$_select,$description, $dateformat);		
		}		
		if (count($retval)>1) return $retval;
		else return $retval[0];
	}

	

	/**
	 * mark all found appearances of a search string in a text with <span> markers 
	 * searchstring is urlencoded
	 * @param string $searchstring 
	 * @param string $_datatext 
	 * @return string
	 * @author Urs Hofer
	 */
	function _mark_search_results($searchstring,$_datatext) {
		$o_e = error_reporting(E_ERROR);
		$_datatext = preg_replace($searchstring, '<span comment="rokfor_temp" class="searchresult">${0}</span comment="rokfor_temp">', $_datatext, -1);
		return $_datatext;
	}
	
	/**
	 * prepares raw text for rtf editing.
	 *
	 * @param string $fielddata 
	 * @return string
	 * @author Urs Hofer
	 */
	function _prepare_for_rtfediting($fielddata) {
		return $this->rfDB->_prepare_for_rtfediting($fielddata);	
	}
	
	/**
	 * _domready adds javascript to the domready part of mootols. if flush is set, the code will be returned.
	 *
	 * @param string $script 
	 * @return void
	 * @author Urs Hofer
	 */
	function _domready($script, $flush=false) 
	{
		static $__js = '<script>
		window.addEvent(\'domready\', function() {
		';
		static $__hascode = false;
		if ($script) {
			$__hascode = true;
			$__js .= $script."\n";    
		}
		if ($flush) {
			if ($__hascode) return $__js.'
				});
			</script>
				';
			else return "\n";
		}
	}	
	
	/**
	 * prints the domready scripts
	 *
	 * @return void
	 * @author Urs Hofer
	 **/
	function domready()
	{
		$this->buffer .= $this->_domready(false,true);
	}
	
	/**
	 * prints the header of the mainbody
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function head() 
	{
		switch (EDITOR) {
			case 'nicEdit':
				$_e = '	
				<script type="text/javascript" src="./js/3rdparty/nicEdit/nicEdit.js"></script>
				';
				break;
			case 'mooEditable':
				$_e = '
				<link rel="stylesheet" href="./css/3rdparty/MooEditable/MooEditable.css" type="text/css" />	
				<script type="text/javascript" src="./js/3rdparty/MooEditable/MooEditable.js"></script>
				';				
				break;		
			case 'ckeditor':
				$_e = '
				<script type="text/javascript" src="./js/3rdparty/ckeditor/ckeditor.js"></script>
				';				
				break;	
			case 'wymeditor':
				$_e = '
				<!-- jQuery library is required, see http://jquery.com/ -->
				<script type="text/javascript" src="./js/3rdparty/wymeditor/jquery/jquery.js"></script>
				<script>jQuery.noConflict();</script>
				<!-- WYMeditor main JS file, minified version -->
				<script type="text/javascript" src="./js/3rdparty/wymeditor/jquery.wymeditor.min.js"></script>
				';				
				break;				
				
				
			default:
				# code...
				break;
		}
		
		
		$this->buffer .= ('
			<!DOCTYPE html>
			<html>
			<head>
				<meta http-equiv="Content-type" content="text/html; charset=utf-8">
				<meta http-equiv="pragma" content="no-cache">	
				<meta http-equiv="cache-control" content="no-cache">					
				<meta name="expires" content="0">
				<title>Rokfor '.VERSION.file_get_contents(SYSDIR.'/version.txt').'</title>
				<link rel="icon" href="/favicon.ico" type="image/x-icon">				
				<link rel="shortcut icon" href="/favicon.ico">				
        <link rel="stylesheet" href="./js/min/?g=css" type="text/css" >
				<!--[if lt IE 9]>
					<link rel="stylesheet" href="./css/main/master.ltie9.css" type="text/css" media="screen" title="no title" charset="utf-8">
			        <script src="http://html5shiv.googlecode.com/svn/trunk/html5.js"></script>
			    <![endif]-->

				<script type="text/javascript" src="./js/constants.php"  charset="utf-8"></script>	
        <script type="text/javascript" src="./js/min/?g=js"  charset="utf-8"></script>
				<!--<script type="text/javascript" src="/rf/js/3rdparty/mootools-core.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/mootools-more.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/mootree.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/nutabs.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/string.utf8.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/string.md5.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/interface/rfGUI.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/interface/rfCALLBACKS.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/Datepicker/Picker.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/Datepicker/Picker.Attach.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/3rdparty/Datepicker/Picker.Date.js"  charset="utf-8"></script>
    		<script type="text/javascript" src="/rf/js/main.js"  charset="utf-8"></script>-->
				'.
				$_e
				.'		
			</head>
			<body id="index">
		');
	}

	/**
	 * prints the footer of the mainbody
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function foot() 
	{
		$this->buffer .= ('
			</body>
			</html>
		');
	}

	/**
	 * prints the login popup
	 *
	 * @param bool $_expired 
	 * @return void
	 * @author Urs Hofer
	 */
	function login($_expired)
	{
		
		$this->_domready('
		$(\'loginform\').dragCenter(true);
		$$(\'#loginform input\').addEvent(\'click\', function(e){
			this.focus(); e.stopPropagation();
		});
		$$(\'#loginform input\').addEvent(\'dblclick\', function(e){
			this.focus(); this.select(); e.stopPropagation();
		});
		$$(\'#loginform input\').addEvent(\'mousedown\', function(e){
			e.stopPropagation();
		});		
		
		$$(\'#loginform input\').addEvent(\'keypress\', function(e) {
		  if (e.key == \'enter\') {rfgui.rfLogin($(\'imp_login\'));}
		});		
		$(\'uname\').focus();
		');
		
		$this->buffer .= ('
			<div id="loginform" class="rfAlerts"><div>'.(($_expired)?($_expired):('')).'<form id="imp_login" method="POST" action="'.SITEPREFIX.'/Login">
			'.Benutzer.':<input type="text" name="uname" id="uname">
			<br/>'.Passwort.':<input type="password" name="pass" id="pass">
			<a href="#" onclick="return rfgui.rfLogin($(\'imp_login\'));" class="button ok green middle">'.Login.'</a>
			<br/>'.Compat.'</form></div>
		');
	}

	/**
	 * returns the book-sorting form (no output/just return) in the settings tab
	 *
	 * @return string
	 * @author Urs Hofer
	 */
	function bookSorting () {
		$retval = "";
		
		
		$values = array (
			"id" => Id,
			"_name" => Name,
			"_newdate" => Adddate,
			"_moddate" => Moddate,
			"__sort__" => Manualsort			
		);
		$order = array (
			"ASC"=>ASC,
			"DESC"=>DESC
		);		

		$_books  = $this->rfDB->getProjects();		
		foreach ($_books as $_book) {
			list($_field,$_by) = explode(' ',$this->caller->getOrder($_book->id));
			$_values = "";
			$_order = "";
			foreach ($values as $key => $value) {$_values .= '<option value="'.$key.'" '.(($key==$_field)?('selected="selected"'):('')).'>'.$value.'</option>';}
			foreach ($order as $key => $value) {$_order .= '<option value="'.$key.'" '.(($key==$_by)?('selected="selected"'):('')).'>'.$value.'</option>';}
				$retval .= '<p>&nbsp;
			<label for="orderby_book['.$_book->id.']">'.$_book->_name.'</label>
			<select  class="tiny" name="ordermode_book['.$_book->id.']">'.$_order.'</select><select class="small" name="orderby_book['.$_book->id.']">'.$_values.'</select>
			</p>';
		}

		$this->_domready("
		$$('#booksorting select').addEvent('change', function(e){
			rfcallbacks.changeOrder('booksorting');
		});
		");
		
		return ('
		<div><a class="toggler button grey wide">'.Sortorder.':</a>
		<form id="booksorting" name="booksorting" class="toggle">
		'.$retval.'
		</form>
		</div>	
		');
	}

	/**
	 * shows the search & replace form
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function showSearchReplaceForm() {
		$_issueselector = '<select multiple="multiple" id="issueselect" name="issues[]">';
		$_fieldlist = array();
		foreach ($this->rfDB->getProjects(false) as $_book) {
			$_issueselector .= '<optgroup label="'.$_book->_name.'">';
			$_eds = $this->rfDB->getEditions($_book->id);
			if ($_eds) foreach ($_eds as $_issue) {
				$_issueselector .= '<option value="'.$_issue->id.'">'.$_issue->_name.'</option>';
				$sql = "SELECT DISTINCT _fortemplate FROM _contributions WHERE _forissue = ".$_issue->id." AND _status = 'open'";
				$_bc = $this->rfDB->query($sql);
				while ($_templateid = $this->rfDB->fetch_object($_bc)) {				
					$_fields = $this->rfDB->getTemplateFields($_templateid->_fortemplate);
					foreach ($_fields as $_field) {
						// Add all but numbers
						if ($_field->_fieldtype<>"Zahl"&&$_field->_fieldtype<>"TypologySlider") $_fieldlist[$_issue->id][] = array($_field->id,$_field->_fieldname);
					}
				}
			}
			$_issueselector .= '</optgroup>';
		}
		$_issueselector .= '</select>';
		
		if ($_SESSION['templateid'] &&  $_SESSION["contribid"]) {
			$_currentTemplateFieldList = array();
			$_fields = $this->rfDB->getTemplateFields($_SESSION['templateid']);	
			foreach ($_fields as $_field) {
				$_currentTemplateFieldList[] = array($_field->id,$_field->_fieldname);
			}				
		}

		$_form .= '<form id="searchreplaceform" class="batchform">
		<p style="clear:both">'.Suchbegriff.':</p>
		<textarea id="search" name="search" rows="3"></textarea>
		<p style="clear:both; font-size: 10px;">
			<label class="wide" for="case">'.CaseSensitive.'</label><input type="checkbox" name="case" id="case" value="true"><br/>
			<label class="wide" for="case">'.UseRegex.'</label><input type="checkbox" name="regex" id="regex" value="true"><br/>
			'.Suchhelp.'</p>		
		<p style="clear:both">'.Ersetzen.':</p>
		<textarea id="replace" name="replace" rows="3"></textarea>
		<p style="clear:both">'.Range.':</p>
		<label class="wide" for="searchrange1">'.LimitSearch.'</label><input type="radio" '.($_SESSION["contribid"]?'checked="checked"':'disabled="disabled"').' name="searchrange" id="searchrange1" value="'.$_SESSION["contribid"].'">
		<br/>
		<label class="wide" for="searchrange2">'.NoLimitSearch.'</label><input onclick="$(\'issuelist\').setStyle(\'display\',\'block\');" type="radio" name="searchrange" id="searchrange2" value="issue">
		<br/>
		<label class="wide" for="searchrange3">'.AllSearch.'</label><input '.(!$_SESSION["contribid"]?'checked="checked"':'').' type="radio" name="searchrange" id="searchrange3" value="all">
		<div id="issuelist">
		<p style="clear:both">'.ExistingIssues.':</p>
		'.$_issueselector.'
		</div>
		<p style="clear:both">'.Fieldlist.':</p>
		<select multiple="multiple" id="fieldselect" name="fieldlist[]"></select>
		<input type="hidden" name="replaceaction" id="replaceaction" value="0">
		</form>';

		$this->_domready("
		var templatefields = JSON.decode('".json_encode($_fieldlist)."');
		".(($_currentTemplateFieldList)?("var currentfields = JSON.decode('".json_encode($_currentTemplateFieldList)."');"):(''))."

		/* Search in Current Contribution */

		$('searchrange1').addEvent('change', function(e){
			$('issuelist').setStyle('display','none');
			$('fieldselect').empty();
			new Element('option').set('value', 0).set('selected', 'selected').set('html','*Alle Felder verwenden*').inject($('fieldselect'));
			var x;		
			var selectedNames = [];
			if (currentfields) for (x=0; x<currentfields.length; x++) selectedNames[currentfields[x][0]] = currentfields[x][1];
			for (x=0; x<selectedNames.length; x++) if (selectedNames[x]) new Element('option').set('value', x).set('html',selectedNames[x]).inject($('fieldselect'));
		});
		if ($('searchrange1').checked) $('searchrange1').fireEvent('change');
		
		/* Search in all books */
		
		$('searchrange3').addEvent('change', function(e){
			$('issuelist').setStyle('display','none');
			$('fieldselect').empty();
			new Element('option').set('value', 0).set('selected', 'selected').set('html','*Alle Felder verwenden*').inject($('fieldselect'));
			var x,y;		
			Object.each(templatefields, function(e, key){
				Object.each(e, function(i_e, i_key){				
					new Element('option').set('value', i_e[0]).set('html',i_e[1]).inject($('fieldselect'));
				});
			});


		});		
		if ($('searchrange3').checked) $('searchrange3').fireEvent('change');		
		
		/* Search in some books */
		
		$('issueselect').addEvent('change', function(e){
			$('searchrange2').checked = true; 
			$('fieldselect').empty();
			new Element('option').set('value', 0).set('html','*Alle Felder verwenden*').inject($('fieldselect'));
			var i,x,z;		
			var selectedNames = [];
			for (i=0; i<$('issueselect').options.length; i++) {
				if ($('issueselect').options[i].selected) {
					var fields = templatefields[$('issueselect').options[i].value];
					if (fields) for (x=0; x<fields.length; x++) {
						selectedNames[fields[x][0]] = fields[x][1];
					}
				}
			}
			for (x=0; x<selectedNames.length; x++) {
				if (selectedNames[x]) {
					new Element('option').set('value', x).set('html',selectedNames[x]).inject($('fieldselect'));
				}
			}			
			
		});		

		$('dosearch').addEvent('click', function(e){ 
			$('replaceaction').value = 0;
			var callback = function(html) {rfgui.rfAlert(this, html);};
			rfgui._ajaxtransport('/Executesearch','searchresults',$('searchreplaceform'),false);
		});
		
		$('doreplace').addEvent('click', function(e){ 
			$('replaceaction').value = 1;
			var callback = function(html) {rfgui.rfAlert(this, html);};
			rfgui._ajaxtransport('/Executesearch','searchresults',$('searchreplaceform'),false);
		});		
		
		
		
		");		

		
		$this->buffer .= ('
		<div class="settings_container">		
			<div>'.$_form.'</div>
			<a class="button grey wide" href="#" id="dosearch">'.Suchen.'</a>
			<a class="button grey wide" href="#" id="doreplace">'.SearchReplace.'</a>			
			<div id="searchresults"></div>
		</div>');		
	}

	/**
	 * prints the batch-sorting form for the settings tab
	 *
	 * @return string
	 * @author Urs Hofer
	 */	                 
	function showBatches() {  

		/* Load Batches */
		$sql_bc = "SELECT * FROM _batch ORDER BY __sort__";  
		$_bc = $this->rfDB->query($sql_bc);
		$_cnt = 0;
		if ($this->rfDB->num_rows($_bc)>0) {
			while ($_fnct_data = $this->rfDB->fetch_object($_bc)) {
				$_batches .= '<a class="batches button grey wide">'.$_fnct_data->_name.'</a><div class="toggle"><p>'.$_fnct_data->_description.'</p>';
				foreach ($this->rfDB->getCrossRef($_fnct_data->id, '_batchid', '_bookid', 'R_batch_forbook') as $_bookId) {
					$_book = $this->rfDB->getProjects(false,$_bookId);
					if ($_book[0]->id) if ($_issues = $this->rfDB->getEditions($_book[0]->id)) {
						foreach ($_issues as $_issue) {
              $_fnct = create_function(
                '&$_db,&$_rokforDB,&$issueid,&$bookid', 
                $this->_stripslashes($_fnct_data->_precode)
              );
              if ($_fnct) {
                $_callback = $_fnct($this->rfDB->db, $this->rfDB, $_issue->id,$_book[0]->id);
          			$ret = true;  
              }
          		else {
          			$_callback = BatchPreError.": ".$_err['message'].' on line '.$_err['line'];       
          			$ret = false;  
          		}
							$_batches .= '<form id="batchform_'.$_issue->id.'_'.$_fnct_data->id.'" class="batchform">
							<p>'.$_book[0]->_name.': '.$_issue->_name.'</p>							
								<p> 
								'.$_callback.'
								</p>
								<input type="hidden" name="function_id" value="'.$_fnct_data->id.'">
								<input type="hidden" name="issue_id" value="'.$_issue->id.'">
								<input type="hidden" name="book_id" value="'.$_book[0]->id.'">
								<input type="submit" value="'.Run.' '.$_fnct_data->_name.'">
								<p style="clear:both">&nbsp;</p>
							</form>';				
						}
					}
				}
				$_batches .= '</div>';				
			}
		}
		else {
			$_batches .= '<a class="batches button grey wide">'.NoBatch.'</a><div class="toggle">';			
			$_batches .= NoBatchText;				
			$_batches .= '</div>';							
		}
		
		$this->_domready("
		$$('.batchform').addEvent('submit', function(e){
			e.preventDefault();                                                
			e.stop();
			var callback = function(html) {
	        	rfgui.rfAlert(this, html, true);
			};
			rfgui._ajaxtransport('/Executebatch','index',this,true,true,callback);
			return (false);
		});
		// Toggle System Tabs
		$$('.batches').addEvent('click', function(e){
			rfcallbacks.toggleSytemtab(this);
		});		
		");		
		
		$this->buffer .= ('
		<div class="settings_container">		
			<div>'.$_batches.'</div>	
		</div>');		
	}
	
	
	/**
	 * returns the template-list (no output/just return) in the settings tab
	 * $div enables or disables div wrapper
	 *
	 * @param div
	 * @return string
	 * @author Urs Hofer
	 */
	function templateSorting($div = true) {
		$_templates = $this->rfDB->getTemplates(false,false, true);			
		$_values = '<option value="-1" selected="selected">'.PleaseChoose.'</option>';
		if ($_templates) foreach ($_templates as $_value) {
			$_values .= '<option value="'.$_value->id.'">'.$_value->_name.'</option>';
		}
		
		if (!$div) 		$this->_domready('
		$(\'templateSorting\').getFirst(\'a\').addEvent(\'click\', function(e){
			rfcallbacks.toggleSytemtab(this);
		});		
		$(\'templateSorting\').getFirst(\'div\').setStyle(\'display\',\'block\');
		');

		return (
 		(($div)?('<div id="templateSorting">'):('')).'
		<a class="toggler button grey wide">'.TemplateProperties.':</a>
		<div class="toggle">
		<ul>
			<li id="modifyTemplates" onclick="rfcallbacks.showListalert(this,function(html,elm){$(\'templateSorting\').set(\'html\',html);});">+ '.NewTemplate.'</li>
		<ul>
		<p>&nbsp;<label for="changeTemplateFields">'.FieldManagement.'</label>
		<select onchange="rfcallbacks.showListalert(this,function(html,elm){$(\'templateSorting\').set(\'html\',html);});" id="changeTemplateFields" class="medium" name="changeTemplateFields">'.$_values.'</select>
		</p></div>
		'.(($div)?('</div>'):('')));
	}

	/**
	 * prints the baseframe.
	 * this is the general layout of all div's and elements used to fill in the 
	 * gui with ajax functions
	 *
	 * @param string $_user 
	 * @param string $_username 
	 * @return void
	 * @author Urs Hofer
	 */
		function baseframe($_user="user", $_username) {
		$this->_domready('
			//Main Tree
			tree = new MooTreeControl({div: \'tree_container\',mode: \'files\',grid: false,
				onSelect: function(node, state) {
					rfcallbacks.rfTreeAction(node, state, $(\'tree_button\'), $(\'tree_container\'), \''.$_user.'\');
				},
				onExpand: function(node, state) {
					rfcallbacks.rfTreeDoubleClickAction(node, state, $(\'tree_button\'), $(\'tree_container\'));
				},
				onMultiSelect: function(nodes, state) {
					rfcallbacks.rfTreeMultiSelectAction(nodes, state, $(\'tree_button\'), $(\'tree_container\'), \''.$_user.'\');
				}				
			},{icon:\'./img/3rdparty/mt/database.png\', text: rfstring_SYSTEM,open: true,forceReload: true});
			tree.root.load(\''.SITEPREFIX.'/Ajax/Tree/Current\');

			//Archive Tree
			tree_arch = new MooTreeControl({div: \'archive_container\',mode: \'files\',grid: false,
				onSelect: function(node, state) {
					rfcallbacks.rfTreeAction(node, state, $(\'archive_button\'), $(\'archive_container\'),\''.$_user.'\');
				},
				onExpand: function(node, state) {
					rfcallbacks.rfTreeDoubleClickAction(node, state, $(\'archive_button\'), $(\'archive_container\'));
				},
				onMultiSelect: function(nodes, state) {
					rfcallbacks.rfTreeMultiSelectAction(nodes, state, $(\'archive_button\'), $(\'archive_container\'), \''.$_user.'\');
				}				
			},{icon:\'./img/3rdparty/mt/database.png\', text: rfstring_SYSTEM,open: true,forceReload: true});
			tree_arch.root.load(\''.SITEPREFIX.'/Ajax/Tree/Archive\');

			// Activate Tabs
			tabs = new nuTabs($$(\'#menu-tabs a\'), $$(\'#tree-tabs .tabs\'), {});
			
			// Start Header Ping
			
			// Toggle System Tabs
			$$(\'.toggler\').addEvent(\'click\', function(e){
				rfcallbacks.toggleSytemtab(this);
			});

		');
		$this->buffer .=('
			<div id="tabs">
				<ul id="menu-tabs">
					<li><a href="#">'.Projekte.'</a></li>
					<li><a href="#">'.Archiv.'</a></li>
					<li><a href="'.SITEPREFIX.'/Ajax/Searchreplaceform">'.Suchen.'</a></li>
					'.
					(($_user=="user")?(''):('<li><a href="'.SITEPREFIX.'/Ajax/Showbatches">'.Batch.'</a></li>'))
                    .'
					<li><a href="#">'.System.'</a></li>
				</ul>	
			</div>
			<div id="tree">
				<div id="tree-tabs">
					<div class="tabs">
						<div id="tree_button" class="tree_buttons"></div>
						<div id="tree_container" class="tree_container"></div>
					</div>
					<div class="tabs">
						<div id="archive_button" class="tree_buttons"></div>
						<div id="archive_container" class="tree_container"></div>
					</div>
					<div class="tabs" id="tab_search"></div>
					'.
					(($_user=="user")?(''):('<div class="tabs" id="tab_batch"></div>'))
                    .'					
					<div class="tabs" id="tab_sys"><div class="settings_container">
					
					'.$this->bookSorting().'
					'.
					(($_user=="user")?(''):(
					$this->templateSorting().'
					<div>
					<a class="toggler button grey wide">'.UsersRights.':</a>
					<ul class="toggle">
						<li id="modifyUsers" onclick="rfcallbacks.showListalert(this);">'.ModifyUsers.'</li>
						<li id="modifyRights" onclick="rfcallbacks.showListalert(this);">'.ModifyRights.'</li>
					</ul>
					</div>
					<div>
					<a class="toggler button grey wide">'.CallbackManagement.':</a>
					<ul class="toggle">
						<li id="modifyPlugins" onclick="rfcallbacks.showListalert(this);">'.ModifyPlugins.'</li>
						<li id="modifyFieldpostprocessor" onclick="rfcallbacks.showListalert(this);">'.ModifyFieldpostprocessor.'</li>
						<li id="modifyBatchs" onclick="rfcallbacks.showListalert(this);">'.ModifyBatchs.'</li>
					</ul>
					</div>
					<div>
					<a class="toggler button grey wide">'.DatabaseManagement.':</a>
					<ul class="toggle">
						<li id="DataConsistency" onclick="rfcallbacks.checkDataConsistency(this)";>'.ModifyContributions.'</li>
						<li id="CleanOrphans" onclick="rfcallbacks.checkDataConsistency(this)";>'.CleanOrphans.'</li>
						<li id="EmptyTrash" onclick="rfcallbacks.checkDataConsistency(this)";>'.EmptyTrash.'</li>																		
					</ul>
					</div>
					'))
					.'
					</div></div>
				</div>
			</div>	
			<div id="mainform"></div>
			<div id="context">
			<div id="contextbuttons"></div>
			<div id="preview"></div>
			</div>
			<div id="intro">
				<div>
				<h1>'.Quickaccess.'</h1>
				<p><a href="#" onclick="rfcallbacks.openLastmodified(this);" class="button blue center">'.EditLast.'</a></p>
				<p><a href="#" onclick="rfcallbacks.cloneLatestContribution(this);" class="button green center">'.CloneLast.'</a></p>
				<p><a href="'.SITEPREFIX.'/help/index.html" target="_blank" class="button red center">'.LoadHelp.'</a></p>
				<p>'.Copyright.' '.VERSION.'(v2 branch)</p>				
				</div>
			</div>
			<div id="header">
				<p><a href="'.SITEPREFIX.'/Logout" class="button mini red">'.Logout.'</a></p>							
				<p><u>'.OtherUsers.'</u>: <span id="other"></span></p>
				<p><u>'.LoggedInAs.'</u>: '.$_username.'</p>
				<p><u>'.Datefield.'</u>: '.date("d.M Y H:i:s", time()).'</p>						
			</div>		
		');
	}

	/**
	 * prints the context menu buttons
	 *
	 * @param string $_contribid 
	 * @return void
	 * @author Urs Hofer
	 */
	function context() {
		if ($this->rfDB->getUserGroup()=="api"||$this->rfDB->getUserGroup()=="user"&&defined("HIDEBUTTONS"))
			$_bt_disabled = explode(",", trim(str_replace("\n", "", strtolower(HIDEBUTTONS))));	
		else
			$_bt_disabled = array();
		$this->buffer .= ('
			<div id="contextbuttons">
			<a href="#" class="button grey" id="rfSave" onclick="rfcallbacks.storeForm(\'dataform\',\'mainform\',\'index\');">'.Save.'</a>
			'.((!in_array("pageexport",$_bt_disabled))?('<a href="#" class="button grey" id="rfPreview" onclick="rfgui.rfAjax(this,\''.PageExport.'\',\'/Ajax/Export/Single/\'+rfgui.contribid,true);">'.PageExport.'</a>'):('')).'
			'.((!in_array("rename",$_bt_disabled))?('<hr/>
			<a href="#" class="button grey" id="rfRename" onclick="rfcallbacks.renameContribution(this);">'.Rename.'</a>'):('')).'
			'.((!in_array("pagemove",$_bt_disabled))?('<a href="#" class="button grey" onclick="rfcallbacks.moveContribution(this);">'.PageMove.'</a>'):('')).'			
			'.((!in_array("savenew",$_bt_disabled))?('<hr/>
			<a href="#" class="button grey" onclick="rfcallbacks.storeAs(this, \'dataform\',\'mainform\',\'index\');">'.SaveNew.'</a>'):('')).'
			'.((!in_array("savenext",$_bt_disabled))?('<a href="#" class="button grey" onclick="rfcallbacks.storeForm(\'dataform\',\'mainform\',\'index\', \'Next\');">'.SaveNext.'</a>'):('')).'
			'.((!in_array("saveprev",$_bt_disabled))?('<a href="#" class="button grey" onclick="rfcallbacks.storeForm(\'dataform\',\'mainform\',\'index\', \'Previous\');">'.SavePrev.'</a>'):('')).'
			'.((!in_array("changetemplate",$_bt_disabled))?('<hr/>
			<a href="#" class="button grey" id="rfChangeTemplate" onclick="rfcallbacks.changeTemplate(this);">'.ChangeTemplate.'</a>'):('')).'
			'.((!in_array("copydata",$_bt_disabled))?('<a href="#" class="button grey" id="rfClone" onclick="rfcallbacks.cloneData(this);">'.CopyData.'</a>
			<hr/>'):('')).'
			</div>
			<div id="preview"></div>		
		');
	}
	
	/**
	 * prints the main form header
	 *
	 * @param string $_e 
	 * @param string $_t 
	 * @return void
	 * @author Urs Hofer
	 */
	function mainformHeader($_e,$_t) {
		$this->_domready('
		rfgui.resetFieldLengthCalc();
		$$(\'.fieldToggler\').each(function(e){rfgui.fieldToggling(e)});
		rfgui.contribid = '.$_e->id.';	/* Store contribution id in rfgui object onload */
		');		
		$this->buffer .= '<div class="title">
				<p id="info">'.Added.': '.date("d-m-Y H:i:s",$_e->_newdate).'<br/>
				'.Changed.': '.date("d-m-Y H:i:s",$_e->_moddate).'<br/>			
				'.Template.': '.$_t->_name.' ('.$_t->id.')</p>			
				<h2>'.$_e->_name.'</h2>				
				<p>&gt; '.$_SESSION['bookname'].' ('.$_SESSION['bookid'].') &gt; '.$_SESSION['issuename'].' ('.$_SESSION['issueid'].') &gt; '.$_SESSION['chaptername'].' ('.$_SESSION['chapterid'].') &gt; ['.$_SESSION['contribid'].']</p>								
			</div>
			<form id="dataform" enctype="multipart/form-data" method="POST" name="dataform" target="uploadTarget" action="'.SITEPREFIX.'/Ajax/Store/'.$_e->id.'">
			<input type="hidden" name="APC_UPLOAD_PROGRESS" id="progress_key" value="'.uniqid().'"/>			
		';
	}
	
	/**
	 * prints a single field in the main form.
	 * subsequentially calls the renderer function for each field with the name "_".$fieldtype
	 *
	 * @param string $_f 
	 * @return void
	 * @author Urs Hofer
	 */
	function mainformField($_f) {
		if ($_f->t_fieldtype<>"*Ausgeschaltet*") {
			if (in_array($_f->t_fieldtype, array("TypologySlider","TypologyMatrix","TypologyKeyword","TypologyCloud"))) $_floating = "floating";
			$_renderer = "_".$_f->t_fieldtype;
			$this->buffer .= ('<div class="fieldContainer '.$_floating.'"><p><a class="button mini blue" href="#" onclick="rfgui.contextHelp('.$_f->t_id.');">'.Help.'</a></p>
			<div id="toggler-'.$_f->t_id.'" class="fieldToggler"></div>
			'.$this->$_renderer($_f).'
			</div>');
		}
	}
	
	/**
	 * prints the main form footer
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function mainformFooter() {
		$renderTime = microtime(true)-$this->startTime;
		$this->_domready('
		rfgui.startFieldLengthCalc();
		$(\'info\').set(\'html\', $(\'info\').get(\'html\') + \'<br/>'.$renderTime.' s.\'); 
		');
		$this->buffer .= '
		</form>
		<iframe id="uploadTarget" name="uploadTarget"></iframe>
		';
	}
	
	/**
	 * Display Information depending on the POST array
	 *
	 * @return void
	 * @author Urs Hofer
	 */
	function displayInfo() {
		switch ($_POST['level']) {
			case 4:
				# chapter
				break;
			case 3:
				# issue
				$sql = ('SELECT * FROM _issues WHERE  id = "'.$_POST['editionid'].'"');
				$query = @$this->rfDB->query($sql);
				$_i = @$this->rfDB->fetch_object($query);
				$this->buffer .= '
				<div id="info"><p>'.StatusTitle.':</p>
				<p>'.Issue.':<br>'.$_i->_name.'</p>				
				<p>'.CreatedBy.':<br> '.$this->rfDB->getUserNameById($_i->__user__).'</p>
				<p>'.Adddate.':<br>'.date("d.m.Y",$_i->_opendate).'</p>
				<p>'.Deadline.':<br>'.date("d.m.Y",$_i->_closedate).', '.(floor(($_i->_closedate-time())/86400)).' '.Days.'</p>				
				<p>'.HinweisLink.':<br>'.($this->_stripslashes($_i->_infotext)).'</p>
				</div>
				';
				break;
			case 2:
				# book
				$_i = $this->rfDB->getProjects(false, $_POST['projectid']);
				$this->buffer .= '
				<div id="info"><p>'.StatusTitle.':</p>
				<p>'.Projekt.':<br>'.$_i[0]->_name.'</p>
				<p>'.CreatedBy.':<br>'.$this->rfDB->getUserNameById($_i[0]->__user__).'</p>
				<p>'.Id.':<br>'.$_POST['projectid'].'</p>
				</div>
				';
				break;
			case 'field':
				$sql = 'SELECT * FROM _templates WHERE id='.$_POST['fieldid'].' LIMIT 1';
				$_i = @$this->rfDB->fetch_object(@$this->rfDB->query($sql));
				if (stristr($_i->_helpimage, '<::::::>')) $img = REPOSITORY.'/'.end(explode('<::::::>',$_i->_helpimage));
				else $img = $_i->_helpimage;
				$this->buffer .= '<div id="info"><p>'.$_i->_fieldname.':</p>';
				$this->buffer .= ($_i->_helpdescription)?($this->_stripslashes($_i->_helpdescription)):('');
				if (is_file(DOCROOT.$img)) {
					$this->buffer .= ('<img src="'.$img.'" border="0" width="140px"/><br/>');
				}
				$this->buffer .= '</div>';
				break;
		}
	}
	
	/**
	 * generic xml header
	 *
	 * @param string $_xml 
	 * @return void
	 * @author Urs Hofer
	 */
	function xml($_xml) {
		header('Content-type: application/xml');	
		$this->buffer .= $_xml;	
	}

	/**
	 * generic redirection header
	 *
	 * @param string $_url 
	 * @return void
	 * @author Urs Hofer
	 */
	function redirect($_url) {
		ob_end_clean();
		header('Location: '.SITEPREFIX.$_url);
		die('<script>window.location="'.SITEPREFIX.$_url.'";</script>');
	}
	
	/**
	 * Below here are the field renderers
	 */

	/**
	 * Numbers Renderer:
	 * Wenn in der Definition (colnames) "integer" steht, wird *KEIN* Datum-Selector ausgegeben.
	 *
	 * @param fieldobject $_f 
	 * @return void
	 * @author Urs Hofer
	 */
	function _Zahl($_f) {
		list($default,$history_command,$fromtemplate,$fromfield,$fixedvalues,$restrict_to_issue,$threeDee,$historydata,$description, $dateformat) = $this->_parseLegends($_f->t_colnames);
		$retval = ""; 
		$showDate = stristr($_f->t_colnames,"integer")?false:true;
		if ($showDate) {
		   	if ($_f->d_content <= 0) $_f->d_content = time();
		   	$retval .= '<label>'.Datefield."</label>: ";
			$_count = 1;
			foreach (array("j" => array(1,31,"."),"n" => array(1,12,"."),"Y" => array(1950,2050," / "),"H" => array(0,24,"."),"i" => array(0,59," ".Timefield)) as $_dateselector=>$_minmax) {
				if (!$dateformat || strstr($dateformat, $_dateselector)) {
				   	$retval .= '<select name="number['.$_f->d_id.']['.$_count.']">';
					for ($l=$_minmax[0]; $l <=$_minmax[1] ; $l++) $retval .= '<option value="'.$l.'" '.(($l == date($_dateselector, $_f->d_content))?(' selected="selected" '):('')).'>'.$l.'</option>';
					$retval .= "</select>".$_minmax[2];
				}
				else $retval .= '<input type="hidden" value="1" name="number['.$_f->d_id.']['.$_count.']">';
				$_count++;
			}
		}
		else $retval .= '<input name="number['.$_f->d_id.']" value="'.$_f->d_content.'">';			
		return ('<h3>'.$_f->t_fieldname.'</h3><div>
		'.$retval.'
		</div>');
	}

	/**
	 * Typology Selector renderer
	 *
	 * @param fieldobject $_f 
	 * @return void
	 * @author Urs Hofer
	 */	
	function _TypologySelect($_f) {
		list($default,$history_command,$fromtemplate,$fromfield,$fixedvalues,$restrict_to_issue,$threeDee,$historydata,$description) = $this->_parseLegends($_f->t_colnames);
		if (stristr($_f->t_colnames,'multiple=true')) $_multi = 'multiple="multiple"';
		/* Process multiple data */
		$_data = !is_array($_f->d_content) ? array($_f->d_content) : $_f->d_content;
		
		if (!$history_command) $history .= '<option disabled>'.HistoryDisabled.'</option>';
		else foreach ($historydata as $select) {
			$history .= '<option value="'.$select->id.'" '.((in_array($select->id, $_data))?('selected="selected"'):('')).'>'.$select->_value.'</option>';
		}

		return ('<p class="status">'.print_r($_f->d_content,true).$description.'</p><h3>'.$_f->t_fieldname.'</h3><div>
		<select '.$_multi.' class="'.($_multi?'big':'medium').'" name="text['.$_f->d_id.']'.(($_multi)?('[]'):('')).'">
		<option value="-1" '.((in_array(-1, $_data)||!isset($_data[0]))?('selected="selected"'):('')).'>'.SelectDisabled.'</option>
		'.$history.'
		</select>
		</div>');
	}	

	/**
	 * Text Renderer
	 *
	 * @param fieldobject $_f 
	 * @return void
	 * @author Urs Hofer
	 */	
	function _Text($_f) {
		// Checking if rtfeditor is disabled
		$_full_history = false;
		if ($_f->t_colnames && stristr($_f->t_colnames, 'fullhistory=true')) {
			$_full_history = true;
			$_f->t_colnames = str_ireplace('fullhistory=true', '', $_f->t_colnames);
		}
		// Checking if rtfeditor is disabled
		$_rtfeditor_disabled = false;
		if ($_f->t_colnames && stristr($_f->t_colnames, 'rtfeditor=false')) {
			$_rtfeditor_disabled = true;
			$_f->t_colnames = str_ireplace('rtfeditor=false', '', $_f->t_colnames);
		}
		$_codeeditor_enabled = false;
		if ($_f->t_colnames && stristr($_f->t_colnames,'codeeditor=true')) {
			$_rtfeditor_disabled = true;
			$_codeeditor_enabled = true;
			$_f->t_colnames = str_ireplace('codeeditor=true', '', $_f->t_colnames);
		}
    
    $_data = $_f->d_content;
    
		// If Colnames: Mehrere Felder erzeugen.
		$_columns = explode(";",$_f->t_colnames);
		if (count($_columns)>1) {
			$_editor = "ARRAY";
			$_f->t_maxlines = '[';
			foreach ($_columns as $_colId=>$_colName) {
				//Getting Maxlines
				preg_match("/<(.*)>(.*)/", $_colName, $_LengthName);
				$_LengthName[1]==0 ? $_ta_rows = 1 : $_ta_rows = $_LengthName[1];
				$_ta_cols = ceil($_f->d_contentlength / $_ta_rows); 
				$_rows = ' rows = "'.$_ta_rows.'" style="height:'.($_ta_rows*1.3).'em;" ';
				$retval .= '
				<p class="editor_text_array">
				<label for="field'.$_f->t_id.'_'.$_colId.'">'.$_LengthName[2].': </label> 
				<textarea class="medium'.($_codeeditor_enabled?' code ':'').'" id="field'.$_f->t_id.'_'.$_colId.'" name="text['.$_f->d_id.'][]" '.$_rows.' cols="'.$_ta_cols.'">'.htmlspecialchars($_data[$_colId]).'</textarea>
 				<span>('.$_LengthName[1].' '.Zeile.')</span>
				</p>
				';
				$_f->t_maxlines .= '['.$_LengthName[1].'],';
			}
			$_f->t_maxlines = rtrim($_f->t_maxlines,",").']';
		}
		// Single Editor
		else {
			$_editor = (!$_rtfeditor_disabled&&defined("EDITOR"))?EDITOR:"TEXTAREA";			
			// Prepare Data
			if ($_editor=="TEXTAREA") {
				$_f->d_content = htmlspecialchars($_f->d_content);
			}
			else {
				// Spell Checking - only for rtf editor!
				if (defined('ENABLE_SPELLCHECK') && ENABLE_SPELLCHECK && !$this->searchstring)	{
					$_f->d_content = $this->_spellCheck($_f->d_content);
				}				
				if ($this->searchstring) $_f->d_content = $this->_mark_search_results($this->searchstring,$_f->d_content);				
				// HTML Conversion
				$_f->d_content = $this->_prepare_for_rtfediting($_f->d_content);				
			}
			// Max Rows of Textarea: either Maxlines or (textlength / 80) or 30 Lines max.
			if ($_f->t_maxlines>0) $_rows = ' rows = "'.$_f->t_maxlines.'" style="height:'.($_f->t_maxlines*1.3).'em;" ';
			else  $_rows = ' rows = "'.((ceil($_f->t_textlength/80)>TEXTAREAMAXLINES)?(TEXTAREAMAXLINES):(ceil($_f->t_textlength/80))).'" ';
			// Create Textarea
			$retval = '<textarea id="field'.$_f->t_id.'" '.$_rows.' class="big'.($_codeeditor_enabled?' code ':'').'" name="text['.$_f->d_id.']">'.$_f->d_content.'</textarea>';
			// Add Javascript if necessary
			if (!$_rtfeditor_disabled) {
				switch ($_editor) {
					case 'nicEdit':
						$this->_domready('area'.$_f->t_id.' = new nicEditor({iconsPath : \'/js/3rdparty/nicEdit/nicEditorIcons.gif\'}).panelInstance(\'field'.$_f->t_id.'\');');
						break;
					case 'mooEditable':
						$this->_domready('$(\'field'.$_f->t_id.'\').mooEditable();');
						break;
					case 'ckeditor':
						$this->_domready('
						var instance = CKEDITOR.instances[\'field'.$_f->t_id.'\'];
						if (instance) {CKEDITOR.remove(instance);}
						CKEDITOR.replace(\'field'.$_f->t_id.'\', {
				        			customConfig : \''.SITEPREFIX.'/js/3rdparty/ckeditor/config.js\',
									height: $(\'field'.$_f->t_id.'\').getSize().y	/* Quirky: But ckedit does not inherit textarea height if defined with row */
				    	});
						');
						break;						
					case 'wymeditor':
						$this->_domready('
						jQuery(function() {
						    jQuery(\'#field'.$_f->t_id.'\').wymeditor();
						});
						');
						break;						
				}
			}
		}
		
		// Print History Selector

		if ($_f->t_history == "Ja") {
				$retval .= '
				<p class="editor_text_array history">
				<label for="history'.$_f->d_id.'">'.History.'</label>
				<select name="history'.$_f->d_id.'" onchange="rfgui.pasteHistory(this,\'field'.$_f->t_id.'\','.count($_columns).',\''.$_editor.'\')" class="medium">';
				$_h  = $this->rfDB->getFieldsByTemplateId($_f->t_id, $_full_history?false:$_SESSION['issueid']);
				foreach ($_h as $_s) {
					$_hist_text = htmlentities(($_full_history?'':$_s->_name.": ").str_replace("<::::::>"," ",$_s->d_content));
					$retval .= '<option value="'.addslashes(htmlentities(utf8_decode(rawurldecode($_s->d_content)))).'" '.((($_s->id == $_f->d_id))?('selected="selected"'):('')).'>'.
					((strlen($_hist_text)<80)?(trim($_hist_text)):(trim(substr($_hist_text,0,80))."..."))
					.'</option>';
				}
				$retval .= "</select></p>";
		}		

		// Add to the field array for maxlength calc
		
		if (!$_codeeditor_enabled) $this->_domready('
		rfgui.addFieldLengthCalc(\'field'.$_f->t_id.'\',\''.$_editor.'\', '.$this->_ConvertInfluenceArray($_f->t_len_influence,$_f).', '.$_f->t_textlength.', '.$_f->t_maxlines.','.count($_columns).');
		');
		
		return ('<p class="status" id="status_field'.$_f->t_id.'">'.Loading.'</p>'.((count($_columns)>1)?(''):('<p class="status" >'.Maxlines.": ".$_f->t_maxlines.'</p>')).'<h3>'.$_f->t_fieldname.'</h3>
		<div>'.$retval.'</div>');
	}
	
	
	function _Tabelle($_f) {
		$data = Array();
		$default = explode(';',$_f->t_colnames);
		// Overwrite if no old data with default values...
    $data = $_f->d_content ? $_f->d_content : $default;
	

		// Getting Field Sizes
		if ($_f->t_maxlines>0) {
			$_ta_rows = $_f->t_maxlines;
			$_ta_cols = ceil($_f->t_textlength / $_ta_rows); 
			$overflow = ' overflow:hidden; ';
		}
		else {
			$_ta_rows = ((ceil($_f->t_textlength/50)>5)?(5):(ceil($_f->t_textlength/50)));
			$_ta_cols = 50;
		}
	
		foreach ($data as $row=>$rows) {	
			$retval .= '
				<li style="white-space: nowrap;">'.(($_f->t_growing=="Ja")?('<div>
					<a href="#" class="t_add button mini green floatright">+</a>
					<a href="#" class="t_del button mini red floatright">-</a>
					&nbsp;
				</div>'):('')).'
			';

			foreach ($rows as $col=>$cols) { 
				$style = ' style="width:'.(floor(100/$_f->t_cols)).'%;'.$overflow.(($col%2<>0)?('background-color: #F1F5FA;'):('background-color: #FFFFFF;')).'"';			
				if ($default[$col]=="Datepicker") $retval .= '<input class="has_date_picker" type="text" value="'.($cols=="Datepicker"?htmlspecialchars(date("d.m.Y H:i",time())):$cols).'">';
				else $retval .= '<textarea maxchar="'.$_f->t_textlength.'" '.$style.' rows="'.$_ta_rows.'" cols="'.$_ta_cols.'">'.htmlspecialchars($cols).'</textarea>';	
			}
			$retval .= '</li>
			';
		}
		$this->_domready('
		rfgui.initTable(\'field'.$_f->t_id.'\',$(\'tbladd'.$_f->t_id.'\'));
		');		
		return ('<p class="status">'.(($_f->t_maxlines)?(MaxlinePerField.': '.$_f->t_maxlines.' '):('')).(($_f->t_textlength)?(MaxcharPerField.': '.$_f->t_textlength):('')).'</p><h3>'.$_f->t_fieldname.'</h3><div>
		<ul id="field'.$_f->t_id.'">
		'.$retval.'
		</ul>
		<input type="hidden" name="table['.$_f->d_id.']" value="'.htmlspecialchars(json_encode($_f->d_content)).'" id="table'.$_f->t_id.'">
		<p class="editor_text_array history">'.(($_f->t_growing=="Ja")?('<a href="#" id="tbladd'.$_f->t_id.'" class="button mini blue medium">'.Insert.'</a>'):('')).'</p>		
		</div>');		
	}
	
	function _Bild($_f) {
		/* Load Data */
		$data =  $_f->d_content;
		if (!is_array($data[0])) {
      $data = [["",""]];
    }
		$retval = '';

		/* Cycle thru Data */
		foreach ($data as $row=>$rows) {	
			$_header = '<div>'.(($_f->t_growing=="Ja")?('<a href="#" class="t_add button mini green floatright">+</a>'):('')).'<a href="#" class="t_del button mini red floatright">-</a>&nbsp;</div>';
			$retval .= '
				<li style="white-space: nowrap;">'.$_header.'
			';
			$retval .= ($rows[1]?('<a href="'.REPOSITORY.'/'.$rows[1].'" target="_blank"><img src="'.REPOSITORY.'/thumbs/'.$rows[1].'-thmb.jpg"></a>'):('')).'
				<input name="existing_files['.$_f->d_id.'][]" type="hidden" value="'.($rows[1]?$row:'added').'"/>
				<textarea rows = "'.($_f->t_maxlines>0?$_f->t_maxlines:1).'" style="width: 380px; margin: 5px; background-color: #F1F5FA; height:'.(($_f->t_maxlines>0?$_f->t_maxlines:1)*1.3).'em;" name="caption['.$_f->d_id.'][]">'.$this->_stripslashes($rows[0]).'</textarea><br/>
				<input style="margin: 5px;" name="data['.$_f->d_id.'][]" type="file"/>
				</li>
			';
		}
		$_addhtml = '<input name="data['.$_f->d_id.'][]" type="file"/><input name="existing_files['.$_f->d_id.'][]" type="hidden" value="added"/><textarea rows = "'.($_f->t_maxlines>0?$_f->t_maxlines:1).'" style="width: 380px; margin: 5px; background-color: #F1F5FA; height:'.(($_f->t_maxlines>0?$_f->t_maxlines:1)*1.3).'em;" name="caption['.$_f->d_id.'][]"></textarea>';
		$this->_domready('
		rfgui.initTable(\'field'.$_f->t_id.'\',$(\'tbladd'.$_f->t_id.'\'), false, \''.$_header.$_addhtml.'\');
		');		
		return ('<p class="status">'.Printsize.': '.$_f->t_imageheight.' x '.$_f->t_imagewidth.' '.Unit.'</p><h3>'.$_f->t_fieldname.'</h3><div>
		<ul id="field'.$_f->t_id.'">
		'.$retval.'
		</ul>
		<p class="editor_text_array history">'.(($_f->t_growing=="Ja")?('<a href="#" id="tbladd'.$_f->t_id.'" class="button mini blue medium">'.Insert.'</a>'):('')).'</p>		
		<input type="hidden" name="MAX_FILE_SIZE" value="200000000" />
		</div>');		
	}	
	
	/**
	 * undocumented function
	 *
	 * @param fieldobject $_f 
	 * @return string
	 * @author Urs Hofer
	 */
	function _FilePicker($_f) {
			$retval = ""; 
			$options = '<option value="false" selected>-DESELECTED-</option>\n';
			$history = $_select = array();
			foreach ($this->rfDB->getFieldsByTemplateId($_f->t_id,$_SESSION['issueid']) as $select) {
				$history[] = $select->d_content;
			}


			$objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($_f->t_colnames), RecursiveIteratorIterator::SELF_FIRST);
			$first = true;
			foreach($objects as $name => $object){
				if ($object->isDir()) {
					if (!$first) $options .='</optgroup>';
					$options .= '<optgroup label="'.str_replace($_f->t_colnames, "", $name).'">\n';
			    }
				else if (!(substr($object->getFilename(), 0,1)==".")) {
					if ($name==$_f->d_content) $options .= '<option value="'.$name.'" selected>'.$object->getFilename()."</option>\n";
					else if (!in_array($name, $history)) $options .= '<option value="'.$name.'">'.$object->getFilename()."</option>\n";
					else $options .= '<option value="'.$name.'" disabled="disabled">'.$object->getFilename()."</option>\n";				
				}
				$first = false;
			}

	    	$retval .= '<select onkeydown="rfgui.previewAnimated(this);" onchange="rfgui.previewAnimated(this);" class="medium" name="text['.$_f->d_id.']">';
			$retval .= $options;
			$retval .= '</select>';
			$retval .= '<p class="status">'.File.': '.end(explode("/", ($_f->d_content))).'&nbsp;<a class="button green mini floatright" href="#" onclick="rfgui.previewVideo(\''.base64_encode($_f->d_content).'\');">'.Preview.'</a></p>';

		return ('<h3>'.$_f->t_fieldname.'</h3><div>
		'.$retval.'
		</div>');
	}
	
	function _TypologyKeyword($_f) {
		$retval = $history = ""; 
		$histArr = Array();
		/* Legacy */
		if (!stristr($_f->t_colnames,'history')) {
			$_f->t_colnames .= "\nhistory=self\n";
		}
		$_resolve_foreign = stristr($_f->t_colnames,'resolve_foreign=true');
	
	
		list($default,$history_command,$fromtemplate,$fromfield,$fixedvalues,$restrict_to_issue,$threeDee,$historydata,$description) = $this->_parseLegends($_f->t_colnames,$_f);
		if (!$history_command) $history .= '<option disabled>'.HistoryDisabled.'</option>';
		else {
			if ($history_command=="self"||$history_command=="") {
				$addField = true;
				$__arr = array();
				foreach ($historydata as $select) {
					if ($select->_value) foreach (explode('<;;;;;;>',$select->_value) as $__t) $__arr[$__t] = $__t;
				}
				asort($__arr);
				foreach ($__arr as $select) {
					$history .= '<option value="'.$select.'">'.$select.'</option>';
				}				
			}
			else {
				$addField = false;
				$id_value_obj = array();
				foreach ($historydata as $select) {
					if ($_resolve_foreign) {
						$history .= '<option value="'.$select->id.'">'.$select->_value.'</option>';
						$id_value_obj[0][] = "'".$select->_value."':".$select->id;
						$id_value_obj[1][] = $select->id.":'".$select->_value."'";
					}
					else $history .= '<option value="'.$select->_value.'">'.$select->_value.'</option>';
				}
			}
		}


		$retval .= ('<div class="smallContainer"><div id="keyword-'.$_f->d_id.'" class="sortable">
			'.(($addField)?('<input type="text" class="newTask" />  <select class="recTask">'):(' <select class="recTask" style="width:191px">')).'
				'.$history.'
			</select>
			<div class="sortContainer"><ol class="sortlist"></ol></div>
			<input class="saveVal" type="hidden" value=\''.json_encode($_f->d_content).'\' name="text['.$_f->d_id.']">
		</div></div>');


		$this->_domready('rfgui.addSort(\'keyword-'.$_f->d_id.'\', '.(($addField)?('true'):('false')).' '.(($_resolve_foreign)?(',[{'.join(',',$id_value_obj[0]).'},{'.join(',',$id_value_obj[1]).'}]'):('')).');');
		return ('<p class="status">'.$description.'</p><h3>'.$_f->t_fieldname.'</h3>
		'.$retval.'
		');

	}

	function _TypologySlider($_f) {
		$retval = ""; 
		$default = $this->_parseLegends($_f->t_colnames,$_f);
		$retval .= ('<div><div id="slider-'.$_f->d_id.'" class="slider">
		<span class="minValLeg">'.$default[0].'</span>
		<span class="maxValLeg">'.$default[1].'</span>
		<div class="knob"><input class="saveVal" type="hidden" value="'.($_f->d_content===NULL?50:$_f->d_content).'"  name="number['.$_f->d_id.']"></div>
		</div></div>');

		$this->_domready('rfgui.addSlider(\'slider-'.$_f->d_id.'\');');
		return ('<h3>'.$_f->t_fieldname.'</h3>
		'.$retval.'
		');
	}
	
	function _TypologyMatrix($_f) {
		$retval = ""; 
		$default = $this->_parseLegends($_f->t_colnames,$_f);
		if ($_f->d_content=='') $_f->d_content = array(50,50);		
		$retval .= ('<div class="smallContainer">
		<div id="matrix-'.$_f->d_id.'" class="square">
			<div class="minXLeg">'.$default[0].'</div>
			<div class="maxXLeg">'.$default[1].'</div>
			<div class="minYLeg">'.$default[2].'</div>
			<div class="maxYLeg">'.$default[3].'</div>
			<div class="knob"><input class="saveVal" type="hidden" \''.json_encode($_f->d_content).'\' name="text['.$_f->d_id.']"></div>
		</div>
		</div>');

		$this->_domready('rfgui.addSquare(\'matrix-'.$_f->d_id.'\');');
		return ('<h3>'.$_f->t_fieldname.'</h3>
		'.$retval.'
		');		
	}
	
	
	/**
	 * Cloud Widget
	 *
	 * @param dataobject $_f
	 * @return string
	 * @author Urs Hofer
	 */
	function _TypologyCloud($_f) {
		$retval = $history = ""; 
		$histArr = Array();
		list($default,$history_command,$fromtemplate,$fromfield,$fixedvalues,$restrict_to_issue,$threeDee,$historydata,$description) = $this->_parseLegends($_f->t_colnames,$_f);
		$_resolve_foreign = stristr($_f->t_colnames,'resolve_foreign=true');
		$id_value_obj = array();

		if (!$history_command) {
			$history .= '<option disabled>'.HistoryDisabled.'</option>';
		}
		else {
			foreach ($historydata as $select) {
				if ($_resolve_foreign) {
					$id_value_obj[0][] = "'".$select->_value."':".$select->id;
					$id_value_obj[1][] = $select->id.":'".$select->_value."'";
				}			
				$history .= '<option value="'.$select->id.'" '.(($select->id==$_f->d_content)?('selected="selected"'):('')).'>'.$select->_value.'</option>';
			}
		}
		
		if ($history_command=="cloud"||$history_command=="self"||$history_command=="") $addField = true;
		else $addField = false;
				
		if ($threeDee) {
			$threeDeeSlider = ('<div id="threeDee-'.$_f->d_id.'" class="threedslider">
				<span class="minValLeg">IN</span>
				<span class="maxValLeg">OUT</span>
				<div class="knob"><input class="threedVal" type="hidden" value="0"  name="threedVal"></div>
				</div>');
			$this->_domready('rfgui.add3dSlider(\'threeDee-'.$_f->d_id.'\', \'keyword-'.$_f->d_id.'\');');
		}
		$retval .= ('<div class="smallContainer">
			<div id="keyword-'.$_f->d_id.'" class="cloud">	
			<div class="cloud-del-handle">&nbsp;</div>
			'.(($addField)?('<input type="text" class="cloudnewTask" />  <select class="cloudrecTask">'):(($history)?(' <select class="cloudrecTask" style="width:160px">'):(''))).'
				'.$history.'
			'.(($history)?('</select>'):('<div class="cloudrecTask">'.FixedHistory.'</div>')).'		
			'.(($threeDee)?($threeDeeSlider):('')).'
			<div class="cloudContainer">
					<div class="minXLeg">'.$default[0].'</div>
					<div class="maxXLeg">'.$default[1].'</div>
					<div class="minYLeg">'.$default[2].'</div>
					<div class="maxYLeg">'.$default[3].'</div>
			</div>
			<input class="saveVal" type="hidden" value=\''.json_encode($_f->d_content).'\' name="text['.$_f->d_id.']">
			<input class="threeDee" type="hidden" value="0">
			</div></div>');
		$this->_domready('rfgui.addTopic(\'keyword-'.$_f->d_id.'\', '.(($addField)?('true'):('false')).''.(($_resolve_foreign)?(' ,[{'.join(',',$id_value_obj[0]).'},{'.join(',',$id_value_obj[1]).'}]'):('')).');');
		return ('<p class="status">'.$description.'</p><h3>'.$_f->t_fieldname.'</h3>
		'.$retval.'
		');

	}	

	/**
	 * prints the ajax table in the rfPopup Field (big usage only)
	 * used mainly in the system tab
	 *
	 * @param string $_action (action switch)
	 * @param int $_dataid (book, issueid)
	 * @param int $_dataid (chapterid)	
	 * @return void
	 * @author albertina
	 */
	function printAjaxTableeditor($_action,$_dataid=false,$_subdata=false) {
		$col=0;
		switch ($_action) {
			case 'addBook':
				$_header = '<span style="width:100px;">Name</span>';
				$_blank->id = "added";$_blank->_name = "Name";				
				$_projects = $this->rfDB->getProjects()?$this->rfDB->getProjects():array($_blank);
				foreach ($_projects as $_value) {
					$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
					$retval .= '<input type="hidden" name="id" value="'.$_value->id.'">';
					$retval .= '<textarea style="width:500px;">'.htmlspecialchars($_value->_name).'</textarea></li>';
					$col++;
				}
				break;
			case 'addChapter':
				$_header = '<span style="width:100px;">Name</span>';
				$_blank->id = "added";$_blank->_name = "Name";				
				$_chapters = $this->rfDB->getParts($_dataid)?$this->rfDB->getParts($_dataid):array($_blank);
				foreach ($_chapters as $_value) {
					$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
					$retval .= '<input type="hidden" name="id" value="'.$_value->id.'">';
					$retval .= '<textarea style="width:500px;">'.htmlspecialchars($_value->_name).'</textarea></li>';
					$col++;
				}
				break;
			case 'sortChapter':
				$_sort = $this->caller->getOrder($this->rfDB->getProjectNamebyEditionId($_dataid)->id);
				$_header = '<span style="width:100px;">Name</span>';
				$_state = "Open";	// User: Only Open Contribs
				if ($this->rfDB->getUserGroup() == 'admin') $_state = 'Open" || _contributions._status = "Close';	// Admin: Open and Closed
				if ($this->rfDB->getUserGroup() == 'root') $_state = false;	// Root: all (incl. deleted)				
				$_chapters = $this->rfDB->getElements($_subdata, $_state, $_dataid, false, $_sort);
				foreach ($_chapters as $_value) {
					$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div class="rowToggler">&nbsp;</div>';
					$retval .= '<input type="hidden" name="id" value="'.$_value->id.'">';
					$retval .= '<textarea disabled="disabled" style="width:600px;">'.htmlspecialchars($_value->_name).'</textarea></li>';
					$col++;
				}
				break;							
			case 'addIssue':
				$_header = '<span style="width:100px;">Name</span>
				<span style="width:150px;">Helptext</span>
				<span style="width:70px;">Deadline</span>
				<span style="width:100px;">File Zugriff</span>
				<span style="width:95px;">Status</span>
				<span style="width:200px;">Output Plugins</span>';			
			
				$_blank->id = "added";$_blank->_name = "Name";$_blank->_infotext = "Info";
				$_blank->_closedate = time();$_blank->_status = "open";
				$_issues = $this->rfDB->getEditions ($_dataid, false, false)?$this->rfDB->getEditions ($_dataid, false, false):array($_blank);
				foreach ($_issues as $_value) {
					$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 54px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
					$retval .= '<input type="hidden" name="id" value="'.$_value->id.'">';
					$retval .= '<textarea style="width:100px; height: 60px;">'.htmlspecialchars($_value->_name).'</textarea>';
					$retval .= '<textarea style="width:150px; height: 60px;">'.htmlspecialchars($_value->_infotext).'</textarea>';
					$retval .= '<textarea style="width:70px; height: 60px;">'.htmlspecialchars(date("d.m.Y",$_value->_closedate)).'</textarea>';

					/* Preparing Plugin Data */
					$sql = "SELECT * FROM _plugins ORDER BY _name";
					$query = @$this->rfDB->query($sql);
					$_options['s'] = '<optgroup label="'.SinglePlugin.'">';
					$_options['a'] = '<optgroup label="'.AllPlugin.'">';
					$_options['r'] = '<optgroup label="'.RTFPlugin.'">';	
					$_options['x'] = '<optgroup label="'.XMLPlugin.'">';
					$_options['n'] = '<optgroup label="'.NarrationPlugin.'">';
					while ($_i = @$this->rfDB->fetch_object($query)) {
           $_check = $this->rfDB->getCrossRef($_value->id, '_issueid', '_pluginid', 'R_issues_singleplugin');
						$_options['s'] .= '<option value="_singleplugin-'.$_i->id.'" '.((in_array($_i->id,$_check))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';
            $_check = $this->rfDB->getCrossRef($_value->id, '_issueid', '_pluginid', 'R_issues_allplugin');
						$_options['a'] .= '<option value="_allplugin-'.$_i->id.'" '.((in_array($_i->id,$_check))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';
            $_check = $this->rfDB->getCrossRef($_value->id, '_issueid', '_pluginid', 'R_issues_rtfplugin');
						$_options['r'] .= '<option value="_rtfplugin-'.$_i->id.'" '.((in_array($_i->id,$_check))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';
            $_check = $this->rfDB->getCrossRef($_value->id, '_issueid', '_pluginid', 'R_issues_xmlplugin');
						$_options['x'] .= '<option value="_xmlplugin-'.$_i->id.'" '.((in_array($_i->id,$_check))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';
            $_check = $this->rfDB->getCrossRef($_value->id, '_issueid', '_pluginid', 'R_issues_narrationplugin');
						$_options['n'] .= '<option value="_narrationplugin-'.$_i->id.'" '.((in_array($_i->id,$_check))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';
          }
					$_options['s'] .= '</optgroup>';
					$_options['a'] .= '</optgroup>';
					$_options['r'] .= '</optgroup>';	
					$_options['x'] .= '</optgroup>';
					$_options['n'] .= '</optgroup>';
					$retval .= '<select style="width:200px;" class="multiselect" multiple="multiple">'.join('',$_options).'</select>';
					$retval .= '<select style="width:70px;"><option value="open" '.((htmlspecialchars($_value->_status)=="open")?('selected="selected"'):('')).'>'.OpenDocs.'</option><option value="closed" '.((htmlspecialchars($_value->_status)=="closed")?('selected="selected"'):('')).'>'.ClosedDocs.'</option></select>';
					$retval .= '<select style="width:90px;">
						<option value="0" '.((htmlspecialchars($_value->__config__)==0)?('selected="selected"'):('')).'>'.IssuePrivate.'</option>
						<option value="1" '.((htmlspecialchars($_value->__config__)==1)?('selected="selected"'):('')).'>'.IssuePublic.'</option>
						<option value="2" '.((htmlspecialchars($_value->__config__)==2)?('selected="selected"'):('')).'>'.IssueAllowDownload.'</option>
						<option value="3" '.((htmlspecialchars($_value->__config__)==3)?('selected="selected"'):('')).'>'.IssueAllowPreview.'</option>
						<option value="4" '.((htmlspecialchars($_value->__config__)==4)?('selected="selected"'):('')).'>'.IssueAllowFulltext.'</option>												
						<option value="5" '.((htmlspecialchars($_value->__config__)==4)?('selected="selected"'):('')).'>'.IssueAllowAllPdf.'</option>												
						</select>';
					$retval .= '</li>';
					$col++;
				}
				break;
			case 'Listtemplates':
			
			$_header = '<span style="width:100px;">Name</span>
			<span style="width:130px;">Helptext</span>
			<span style="width:70px;">Image Url</span>
			<span style="width:70px;">Category</span>
			<span style="width:90px;">Status</span>
			<span style="width:120px;">Book</span>
			<span style="width:120px;">Chapter</span>';			
			
				$_blank->id = "added";
				$_blank->_name = "Name";
				$_blank->_helptext = "Info Text about the Template";
				$_blank->_category = "Template Group";				
				$_blank->_helpimage = "Url for help image\ni.e. /help/field.png\nLeave empty to disable";
				$_blank->_inchapter = "";
				$_blank->_forbook = "";
				$_blank->_public = "false";
				$_templates = $this->rfDB->getTemplates(false,false, true)?$this->rfDB->getTemplates(false,false, true):array($_blank);			
				foreach ($_templates as $_value) {
					
					/* Preparing Plugin Data */
					$sql = "SELECT * FROM _formats ORDER BY _name";
					$query = @$this->rfDB->query($sql);
					$_options['c'] = "";

          $_active_books = $this->rfDB->getCrossRef($_value->id, '_templateid', '_bookid', 'R_templatenames_forbook');
					if (count($_active_books)==0) {
						$_active_books = false;
					}

          $_active_chapter = $this->rfDB->getCrossRef($_value->id, '_templateid', '_chapterid', 'R_templatenames_inchapter');
					while ($_i = @$this->rfDB->fetch_object($query)) {
						if (!$_active_books || in_array($_i->_forbook, $_active_books)) {
							$_options['c'] .= '<option value="_chapter-'.$_i->id.'" '.((in_array($_i->id,$_active_chapter))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';							
						}
					}

					/* Preparing Plugin Data */
					$sql = "SELECT * FROM _books ORDER BY _name";
					$query = @$this->rfDB->query($sql);
					$_options['b'] = "";
					while ($_i = @$this->rfDB->fetch_object($query)) {
						$_options['b'] .= '<option value="_book-'.$_i->id.'" '.((in_array($_i->id,$_active_books))?('selected="selected"'):('')).'>'.$_i->_name.'</option>';
					}
					$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 54px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
					$retval .= '<input type="hidden" name="id" value="'.$_value->id.'">';
					$retval .= '<textarea style="width:100px; height: 60px;">'.htmlspecialchars($_value->_name).'</textarea>';
					$retval .= '<textarea style="width:130px; height: 60px;">'.htmlspecialchars($_value->_helptext).'</textarea>';
					$retval .= '<textarea style="width:70px; height: 60px;">'.htmlspecialchars($_value->_helpimage).'</textarea>';
					$retval .= '<textarea style="width:70px; height: 60px;">'.htmlspecialchars($_value->_category).'</textarea>';
					$retval .= '<select style="width:120px;" class="multiselect" multiple="multiple">'.$_options['c'].'</select>';
					$retval .= '<select style="width:120px;" class="multiselect" multiple="multiple">'.$_options['b'].'</select>';
					$retval .= '<select style="width:70px;"><option value="true" '.((htmlspecialchars($_value->_public)=="true")?('selected="selected"'):('')).'>true</option><option value="false" '.((htmlspecialchars($_value->_public)=="false")?('selected="selected"'):('')).'>false</option></select>';
					$retval .= '</li>';
					
					$col++;
				}			
				break;
			case 'Listfields':
				$_blank->id = "added";			
				$_blank->_fieldname = "Fieldname";
				$_blank->_helpdescription = "Info Text about the Field";
				$_blank->_helpimage = "Url for help image\ni.e. /help/field.png\nLeave empty to disable";
				$_blank->_fieldtype = "*Ausgeschaltet*";
				$_blank->_maxlines = 0;
				$_blank->_textlength = 0;
				$_blank->_imagewidth = 0;
				$_blank->_imageheight = 0;
				$_blank->_cols = 0;
				$_blank->_colNames = "Col1;Col2, Legends, History";
				$_blank->_history = "Nein";
				$_blank->_growing = "Ja";
				$_blank->_lengthInfluence = 0;
			
				$_fieldtypes = array ("Text","Bild","Tabelle","Zahl","TypologySlider",
				"TypologyMatrix","TypologyKeyword",	"TypologyCloud",
				"TypologySelect","FilePicker","*Ausgeschaltet*");

				$_header = '<span style="width:100px;">Feldname</span>
				<span style="width:100px;">Helptext</span>
				<span style="width:70px;">Image Url</span>
				<span style="width:200px;">Legends</span>
				<span style="width:95px;">Type</span>
				<span style="width:60px;">Growing</span>
				<span style="width:60px;">History</span>';

	
				$_fields = $this->rfDB->getTemplateFields($_dataid)?$this->rfDB->getTemplateFields($_dataid):array($_blank);			
				foreach ($_fields as $_value) {
					$_options['t'] = "";
					foreach ($_fieldtypes as $value) {
						$_options['t'] .= '<option value="'.$value.'" '.(($value==$_value->_fieldtype)?('selected="selected"'):('')).'>'.$value.'</option>';
					}
					$retval .= '<li style="height: 104px; '.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 100px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
					$retval .= '<input type="hidden" name="id" value="'.$_value->id.'">';
					$retval .= '<textarea style="width:100px; height: 60px;">'.htmlspecialchars($_value->_fieldname).'</textarea>';
					$retval .= '<textarea style="width:100px; height: 60px;">'.htmlspecialchars($_value->_helpdescription).'</textarea>';
					$retval .= '<textarea style="width:70px; height: 60px;">'.htmlspecialchars($_value->_helpimage).'</textarea>';
					$retval .= '<textarea style="width:197px; height: 60px; border-right: 1px solid #CCC">'.htmlspecialchars($_value->_colNames).'</textarea>';
					$retval .= '<select style="width:60px;"><option value="Ja" '.((htmlspecialchars($_value->_history)=="Ja")?('selected="selected"'):('')).'>Ja</option><option value="Nein" '.((htmlspecialchars($_value->_history)=="Nein")?('selected="selected"'):('')).'>Nein</option></select>';
					$retval .= '<select style="width:60px;"><option value="Ja" '.((htmlspecialchars($_value->_growing)=="Ja")?('selected="selected"'):('')).'>Ja</option><option value="Nein" '.((htmlspecialchars($_value->_growing)=="Nein")?('selected="selected"'):('')).'>Nein</option></select>';
						$retval .= '<select style="width:80px;">'.$_options['t'].'</select>';
					$retval .= '<label>Maxlines:<input type="text" value="'.$_value->_maxlines.'"></label>';
					$retval .= '<label>Textlength:<input type="text" value="'.$_value->_textlength.'"></label>';
					$retval .= '<label>Imagewidth:<input type="text" value="'.$_value->_imagewidth.'"></label>';
					$retval .= '<label>Imageheight:<input type="text" value="'.$_value->_imageheight.'"></label>';
					$retval .= '<label>Columns:<input type="text" value="'.$_value->_cols.'"></label><br/>';
					$retval .= '<label style="width:250px; border-top: 1px solid #CCC;">Influence (Factor|Fieldname||...):<input type="text" value="'.str_replace(array('%%%%%','<::::::>'),array('|','||'),$_value->_lengthInfluence).'"></label>';
					$retval .= '</li>';
					$col++;					
					
				}				
				break;			
			case 'Listusers':
				$_header = '<span style="width:200px;">Name</span><span style="width:290px;">Password</span><span style="width:200px;">Group</span>';
				$_blank->id = "added";
				$_blank->username = "Name";				
				$_blank->password = "xxx";				
				$_blank->usergroup = "User";				
				$_group  = $this->rfDB->getUserGroup();
				if ($_group=="root") $sql = 'SELECT * FROM users ORDER BY id';
				else $sql = 'SELECT * FROM users WHERE usergroup = "user" || id = '.$this->rfDB->getUserId().' ORDER BY id';
				$query = @$this->rfDB->query($sql);
				$_repeat = true;
				do {
					$_i = @$this->rfDB->fetch_object($query);
					if (!($_repeat=$_i)&&$col==0) $_i=$_blank;
					if ($_i) {
						$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
						$retval .= '<input type="hidden" name="id" value="'.$_i->id.'">';
						$retval .= '<textarea style="width:200px;">'.htmlspecialchars($_i->username).'</textarea>';
						$retval .= '<textarea style="width:290px;">'.htmlspecialchars($_i->password).'</textarea>';										
						$retval .= '<select style="width:200px;">
									<option value="admin" '.((htmlspecialchars($_i->usergroup)=="admin")?('selected="selected"'):('')).'>Admin</option>
									<option value="user" '.((htmlspecialchars($_i->usergroup)=="user")?('selected="selected"'):('')).'>Regular User</option>
									<option value="api" '.((htmlspecialchars($_i->usergroup)=="api")?('selected="selected"'):('')).'>Regular multisession User</option>									
									'.(($_group=="root")?('<option value="root" '.((htmlspecialchars($_i->usergroup)=="root")?('selected="selected"'):('')).'>Super Admin</option>'):('')).'
									</select>';					
						$retval .= '</li>';
					}
					$col++;
				} while ($_repeat);
				break;				
			case 'Listrights':
				$_header = '<span style="width:100px;">Group Name</span><span style="width:150px;">Members</span><span style="width:150px;">Issues</span><span style="width:150px;">Books</span><span style="width:150px;">Templates</span>';
				$_blank->id = "added";
				$_blank->_group = "Group Name";				
				$_group  = $this->rfDB->getUserGroup();
				if ($_group=="root") $sql = 'SELECT * FROM _rights ORDER BY __sort__';
				else $sql = 'SELECT * FROM _rights WHERE __user__ = "'.$this->rfDB->getUserId().'" || __user__ = "'.$this->rfDB->getUserName().'" ORDER BY id';
				$query = @$this->rfDB->query($sql);
				$_repeat = true;
				do {
					$_i = @$this->rfDB->fetch_object($query);
					if (!($_repeat=$_i)&&$col==0) $_i=$_blank;
					if ($_i) {
						$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 54px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
						$retval .= '<input type="hidden" name="id" value="'.$_i->id.'">';
						$retval .= '<textarea style="width:100px; height: 60px;">'.htmlspecialchars($_i->_group).'</textarea>';
						if ($_i->id<>'added') $_rights = $this->rfDB->showGroupRights($_i->id);
						$sql = array(
							"Templates" => "SELECT id,_name FROM _templatenames ORDER BY __sort__",
							"Projects"  => "SELECT id,_name FROM _books ORDER BY __sort__",
							"Editions"  => "SELECT id,_name FROM _issues ORDER BY __sort__",		
							"Users"     => "SELECT id,username as _name FROM users ORDER BY username"
						);
						foreach ($sql as $key=>$_sql) {
							$_right = $_rights[$key];
							$_q = $this->rfDB->query($_sql);
							$retval .= '<select style="width:150px;" class="multiselect" multiple="multiple">';
							while ($_row = $this->rfDB->fetch_object($_q)) {
								$retval .= '<option value="_'.$key.'-'.$_row->id.'" '.(in_array($_row->id,$_right)?('selected="selected"'):('')).'>'.$_row->_name.'</option>';
							}
							$retval .= '</select>';							
						}
						$retval .= '</li>';
					}
					$col++;
				} while ($_repeat);
				break;
			case 'Listplugins':
				$_header = '<span style="width:350px;">Plugin Name</span><span style="width:350px;">Callback / Config / Page</span>';
				$_blank->id = "added";
				$_blank->_name = "Plugin Name";				
				$_group  = $this->rfDB->getUserGroup();
				$sql = 'SELECT * FROM _plugins ORDER BY __sort__';
				$query = @$this->rfDB->query($sql);
				$_repeat = true;
				do {
					$_i = @$this->rfDB->fetch_object($query);
					if (!($_repeat=$_i)&&$col==0) $_i=$_blank;
					if ($_i) {
						$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 200px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
						$retval .= '<input type="hidden" name="id" value="'.$_i->id.'">';
						$retval .= '<textarea style="width:350px; height: 100px;">'.htmlspecialchars($_i->_name).'</textarea>';
						$retval .= '<textarea wrap="off" style="border-right: 1px solid #CCC; width:350px; height: 100px; overflow: auto; ">'.htmlspecialchars($this->_stripslashes($_i->_callback)).'</textarea><br/>';
						$retval .= '<textarea wrap="off" style="border-top: 1px solid #CCC;  width:350px; height: 100px; overflow: auto;">'.htmlspecialchars($this->_stripslashes($_i->_config)).'</textarea>';
						$retval .= '<textarea wrap="off" style="border-top: 1px solid #CCC;  none; width:350px; height: 100px; overflow: auto;">'.htmlspecialchars($this->_stripslashes($_i->_page)).'</textarea>';												
						$retval .= '</li>';
					}
					$col++;
				} while ($_repeat);
				break;
			case 'Listbatchs':
					$_header = '<span style="width:150px;">Batch Name</span><span style="width:367px;">Description</span><span style="width:180px;">For Book / Precode / Postcode</span>';
					$_blank->id = "added";
					$_blank->_name = "Plugin Name";				
					$_blank->_description = "Description";				
					$_blank->_precode = CallbackPreMessage;				
					$_blank->_postcode = CallbackAfterMessage;									

					$_group  = $this->rfDB->getUserGroup();
					$sql = 'SELECT * FROM _batch ORDER BY __sort__';
					$query = @$this->rfDB->query($sql);
					$_repeat = true;
					do {
						$_i = @$this->rfDB->fetch_object($query);
						if (!($_repeat=$_i)&&$col==0) $_i=$_blank;
						if ($_i) {
							
							/* Preparing Plugin Data */
							$sql_b = "SELECT * FROM _books ORDER BY _name";
							$query_b = @$this->rfDB->query($sql_b);
							$_options = "";
              $_check = $this->rfDB->getCrossRef($_i->id, '_batchid', '_bookid', 'R_batch_forbook');
							while ($_b = @$this->rfDB->fetch_object($query_b)) {
								$_options .= '<option value="_book-'.$_b->id.'" '.((in_array($_b->id,$_check))?('selected="selected"'):('')).'>'.$_b->_name.'</option>';
							}

							$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 160px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
							$retval .= '<input type="hidden" name="id" value="'.$_i->id.'">';
							$retval .= '<textarea style="width:150px; height: 60px;">'.htmlspecialchars($_i->_name).'</textarea>';
							$retval .= '<textarea style="width:360px; height: 60px;">'.htmlspecialchars($_i->_description).'</textarea>';
							$retval .= '<select style="width:186px;" class="multiselect" multiple="multiple">'.$_options.'</select><br/>';														
							$retval .= '<textarea wrap="off" style="border-top: 1px solid #CCC;  width:350px; height: 100px; overflow: auto;">'.htmlspecialchars($this->_stripslashes($_i->_precode)).'</textarea>';
							$retval .= '<textarea wrap="off" style="border-top: 1px solid #CCC;  width:350px; height: 100px; overflow: auto;">'.htmlspecialchars($this->_stripslashes($_i->_postcode)).'</textarea>';												
							$retval .= '</li>';
						}
						$col++;
					} while ($_repeat);
					break;				
			case 'Listfieldprocessors':
				$_header = '<span style="width:150px;">Processor Name</span><span style="width:365px;">Code</span><span style="width:200px;">Field</span>';
				$_blank->id = "added";
				$_blank->_name = "Name";							
				$_blank->_code = CallbackFieldAlert;									

				$_group  = $this->rfDB->getUserGroup();
				$sql = 'SELECT * FROM _fieldpostprocessor ORDER BY __sort__';
				$query = @$this->rfDB->query($sql);
				$_repeat = true;
				do {
					$_i = @$this->rfDB->fetch_object($query);
					if (!($_repeat=$_i)&&$col==0) $_i=$_blank;
					if ($_i) {
					
						/* Preparing Plugin Data */
						$sql_f = "SELECT * FROM _templates ORDER BY _fieldname";
						$query_f = @$this->rfDB->query($sql_f);
						$_options = "";
            $_check = $this->rfDB->getCrossRef($_i->id, '_postprocessorid', '_templateid', 'R_fieldpostprocessor_forfield');
            
						while ($_b = @$this->rfDB->fetch_object($query_f)) {
							$_options .= '<option value="_field-'.$_b->id.'" '.((in_array($_b->id,$_check))?('selected="selected"'):('')).'>'.$_b->_fieldname.'</option>';
						}

						$retval .= '<li style="'.(($col%2<>0)?('background: #F1F5FA;'):('background: #FFFFFF;')).'"><div style="height: 160px;" class="rowToggler">&nbsp;</div><div><a href="#" class="t_add button micro green">+</a><a href="#" class="t_del button micro red">-</a></div>';
						$retval .= '<input type="hidden" name="id" value="'.$_i->id.'">';
						$retval .= '<textarea style="width:150px; height: 100px;">'.htmlspecialchars($_i->_name).'</textarea>';
						$retval .= '<select style="width:200px;" class="multiselect" multiple="multiple">'.$_options.'</select>';														
						$retval .= '<textarea wrap="off" style="border-top: 1px solid #CCC;  width:360px; height: 100px; overflow: auto;">'.htmlspecialchars($this->_stripslashes($_i->_code)).'</textarea>';												
						$retval .= '</li>';
					}
					$col++;
				} while ($_repeat);
				break;
		}
		$this->_domready("
		rfgui.initTable('editor".$_action."',".($_action=='sortChapter'?("false"):("$('tblAdd')")).");
		");		
		$this->buffer .= ('
		<p class="header">'.$_header.'</p><div id="rfPopFormContainer"><ul id="editor'.$_action.'">
			'.$retval.'
		</ul>
		<input type="hidden" name="store" value="" id="store'.$_action.'">
		</div>
		'.($_action=='sortChapter'?(''):('<a href="#" id="tblAdd" class="button mini blue medium">'.Insert.'</a>')));
	}


	/**
	 * prints html snippets used in rfAjax - Alert Boxes
	 *
	 * @param string $_action 
	 * @param int $_value 
	 * @return void
	 * @author Urs Hofer
	 */
	function printAjaxForm($_action,$_project=false,$_issue=false,$_chapter=false,$_contribution=false) {
		switch ($_action) {
			case 'Movecontribution':
				$this->buffer .= '<p><label for="issuechapter">'.MoveToTarget.':</label><select name="issuechapter" id="issuechapter">';
				# Shows all possible books/issues for a contribution...
				$_contribInfo = $this->rfDB->getElementsNamebyId($_SESSION['contribid']);
				# Load all books & chapters for the template of this contribution
				$sql = "SELECT * FROM _templatenames WHERE id = ".$_contribInfo->_fortemplate;
				$_t = @$this->rfDB->fetch_object(@$this->rfDB->query($sql));
				$_chapter_allowed = $this->rfDB->getCrossRef($_t->id, '_templateid', '_chapterid', 'R_templatenames_inchapter');
        $_books = $this->rfDB->getCrossRef($_t->id, '_templateid', '_bookid', 'R_templatenames_forbook');
				# cycle thru all issues of a the books
				foreach ($_books as $_bookid) {
          $_issues = @$this->rfDB->getEditions ($_bookid);
          $_chapters = @$this->rfDB->getParts ($_bookid);
          foreach ($_issues as $_issue) {
          	foreach ($_chapters as $_chapter) {
          		if (in_array($_chapter->id, $_chapter_allowed)) $this->buffer .= '<option value="'.$_bookid.'-'.$_issue->id.'-'.$_chapter->id.'">'.$this->rfDB->getProjectName($_bookid)->_name.' &gt; '.$_issue->_name.' &gt; '.$_chapter->_name.'</option>';						
          	}
          }
				}
				$this->buffer .= '</select><br/>';

				
			break;
			case 'Clonelast':
				# Clones the latest contribution
				$_contribId = $this->caller->getLastmodifiedContrib();
				$_contribInfo = $this->rfDB->getElementsNamebyId($_contribId);
				$this->buffer .= '<label for="prompt">'.Name.':</label><input id="prompt" name="prompt" value="'.$_contribInfo->_name.rfstring_CLONEADDITION.'"></p>';
				break;
			case 'Changetemplate':
				# Changes the template of a contribution
				$_tpl = $this->rfDB->getTemplates($_SESSION['bookid'],$_SESSION['chapterid']);
				if (is_array($_tpl)) {
					$this->buffer .= '<p><label for="templatename">'.Template.':</label><select name="templatename" id="templatename">';
					foreach ($_tpl as $_t) {
						$this->buffer .= '<option value="'.$_t->id.'" '.(($_SESSION['lastTemplate']==$_t->id)?(' selected="selected" '):('')).'>'.$_t->_name.'</option>';
					}
					$this->buffer .= '</select><br/>';
				}	
				else $this->buffer .= NoTemplates.'<br/>';
				break;
			case 'Addcontribution':
				# Add Contribution HTML Code
				$_tpl = $this->rfDB->getTemplates($_project,$_chapter);
				if (is_array($_tpl)) {
					$this->buffer .= '<p><label for="templatename">'.Template.':</label><select name="templatename" id="templatename">';
					foreach ($_tpl as $_t) {
						$this->buffer .= '<option value="'.$_t->id.'" '.(($_SESSION['lastTemplate']==$_t->id)?(' selected="selected" '):('')).'>'.$_t->_name.'</option>';
					}
					$this->buffer .= '</select><br/>';
					$this->buffer .= '<label for="contribname">'.Name.':</label><input id="contribname" name="contribname"></p>';
				}	
				else $this->buffer .= NoTemplates.'<br/>';
			
				break;
			case 'Clonedata':
				$_tpl = $this->rfDB->getElements ($_SESSION['chapterid'], 'Open" || _contributions._status = "Close', '_contributions._forissue', $_SESSION['templatename']);
				if (is_array($_tpl)) {
					$this->buffer .= '<p>'.Template." ".$_SESSION['templatename'].'<br/>&nbsp;</p>';
					$this->buffer .= '<p><label for="contribid">'.ExistingContribs.':</label><select name="contribid" id="contribid">';
					foreach ($_tpl as $_t) {
						if ($_t->id <> $_SESSION['contribid']) $this->buffer .= '<option value="'.$_t->id.'">'.$_t->_name.'</option>';
					}
					$this->buffer .= '</select><br/>';
				}	
				else $this->buffer .= NoContribs.'<br/>';
				break;
		}
	}
	
	/**
	 * parses the plugin name and returns the generator-engine
	 *
	 * @param string $_plugin 
	 * @return void
	 * @author Urs Hofer
	 */
	function _getEngine($_plugin) {
		preg_match('/(.+?)-/', $_plugin, $_plugMatch);
		$_engine = false;
		$_engine_from_plugin = $_plugMatch[1];
		foreach (new DirectoryIterator(GENERATORDIR."/engines") as $fileInfo) if($fileInfo->isDir() && $_engine_from_plugin==$fileInfo->getFilename()) {$_engine=$_engine_from_plugin; break;}
		return $_engine;
	}
	
	
	
	/**
	 * shows the forms for the document exporter
	 * $_mode: 'Single' or 'Complete'
	 * $_id: Contribution Id (Single Mode) or Book Id (Complete Mode)
	 * $_default: chosen plugin by default
	 * $_rendering: triggers the generator (true or false)
	 *
	 * @param string $_mode 
	 * @param string $_id 
	 * @return string $pdflink (filename)
	 * @author Urs Hofer
	 */
	function showExporter($_mode, $_id, $_default = false, $_rendering = false) {
		switch ($_mode) {
			case 'Single':
				/* Getting all generators & Plugins into a sorted array */
				$_contrib = $this->rfDB->getElementsNamebyId($_id);
				$_issues = $this->rfDB->getEditionInfo($_contrib->_forissue);
				$_plugins = array();				
				foreach ($this->rfDB->getCrossRef($_issues->id, '_issueid', '_pluginid', 'R_issues_singleplugin') as $_plugid) {
					$sql = "SELECT * FROM _plugins WHERE id = $_plugid";
					$query = @$this->rfDB->query($sql);
					$_plug = @$this->rfDB->fetch_object($query);
					if ($_eName = $this->_getEngine($_plug->_name)) {
						$_plugins[$_eName][] = $_plug;
						if (!$_default || $_default==$_plug->id) {
							$_default = array();
							$_default["plugin"]=$_plug;
							$_default["engine"]=$_eName;
						}
					}
				}	
				if (!$_rendering) {
					/* Printing left hand side plugin choice div */
					$this->buffer .= '<div id="plugchoose">';
					foreach ($_plugins as $_enginename => $_plugs) {
						$this->buffer .= '<p class="enginename">'.mb_convert_case($_enginename, MB_CASE_TITLE, "UTF-8").'</p>';
						$col = 0;
						foreach ($_plugs as $_plug) {					
							$this->buffer .= '<p class="pluginname" style="'.(($col%2<>0)?('background-color: #F1F5FA;'):('background-color: #FFFFFF;')).'"><input type="radio" '.(($_default["plugin"]->id==$_plug->id)?('checked="checked"'):('')).' name="plugradio" id="plugradio'.$_plug->id.'" class="plugchangers" value="'.$_plug->id.'"><label for="plugradio'.$_plug->id.'">'.mb_convert_case(str_replace(array($_enginename,"-"),array(""," "),$_plug->_name), MB_CASE_TITLE, "UTF-8").'</label></p>';
							$col++;
						}
					}
					$this->buffer .= '</div>';
					$this->buffer .= '<div id="renderer">';
					$this->buffer .= '</div>';	
					
					$this->_domready("
					// Render default
					rfgui._ajaxtransport('/Render/".$_mode."/".$_id."/".$_default["plugin"]->id."','renderer');					
					");					
					
									
				}
				else {
					/* Rendering the Page into the right hand side div */

					if ($_default["engine"] && $_contrib->_forissue && $_default["plugin"]->_name) {

					// Choose the right engine according to the plugin name
					if (!$_db) $_db = &$this->rfDB->db; 					// Compatibility
					if (!defined("rootdir")) define("rootdir", DOCROOT);	// Compatibility
					$_plugin = $_default["plugin"]->_name;					// Compatibility
					global $_rokforDB;										// Compatibility
					global $hyphen;											// Compatibility
					$_rokforDB = new rokfor_DB();							// Compatibility
										
					if (!is_dir(REPOSITORYDIR."/previews/")) {
						if (!mkdir(REPOSITORYDIR."/previews/")) die ('Critical: cannot create preview dir: '.(REPOSITORYDIR."/previews/"));
					}
					include (GENERATORDIR."/engines/".$_default["engine"]."/core.inc");

					// Generate and put out HTML w. Thumbnail
						$tex = new TeX($_default["plugin"]->_name, $this->rfDB); 
						if ($tex->error) die($tex->error);
						$tex->finalize($_contrib->id);
						if ($pdflink = $tex->generatePDF(REPOSITORYDIR."/previews/")) {

							/* Post Processor Callback	- Executed after the whole File is generated */
							if (method_exists('postprocessor', 'generic_v2')) 
								call_user_func(array('postprocessor', 'generic_v2'),$tex, false,$pdflink, REPOSITORYDIR."/previews/");			

							$_pages = (int)exec('pdfinfo "'.(REPOSITORYDIR.'/previews/'.$pdflink).'" 2>/dev/null | grep Pages | cut -d ":" -f 2');

							$this->buffer .= ('<div class="previewImage">');
							for ($i=0; $i < ($_pages>5?5:$_pages); $i++) { 
								$this->buffer .= ('<img src="'.SITEPREFIX.'/Ajax/Pdfconverter/'.$i.'/'.$pdflink.'"/>');
							}
							$this->buffer .= ('</div>');
							$this->buffer .= ('<p class="enginename">'.Download.': <a href="'.REPOSITORY.'/previews/'.$pdflink.'" target="_blank">'.$pdflink.'</a> ('.$_pages.' '.Pages.')</p>');

							/* Inserting into _pdf log with __config__ 0 means single page */
							$sql = ("
								INSERT INTO `_pdf` (`id`, `_file`, `_date`, `_issue`, `_plugin`, `_pages`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
								('', '$pdflink', '".time()."', ".$_contrib->id.", ".$_default["plugin"]->id.", $_pages, '".$this->rfDB->getUserId()."', 0, NULL, -1, 0);
							");
							$this->rfDB->query($sql);
							
						}
						else $this->buffer .=  ('<p class="enginename">'.RendererError.'</p>
						<p class="pluginname">'.RendererGeneratorFailure.'</p>');
					}
					else {
						$this->buffer .= NoRendererActivated;					
					}
				}
				break;
			
			case 'Complete':
				/* Getting all generators & Plugins into a sorted array */
				$_issues = $this->rfDB->getEditionInfo($_id);
				$_srcplugs = $_plugins = array();
				$_srcplugs = array_merge(
          $this->rfDB->getCrossRef($_issues->id, '_issueid', '_pluginid', 'R_issues_allplugin'),
          $this->rfDB->getCrossRef($_issues->id, '_issueid', '_pluginid', 'R_issues_xmlplugin'),
          $this->rfDB->getCrossRef($_issues->id, '_issueid', '_pluginid', 'R_issues_rtfplugin'),
          $this->rfDB->getCrossRef($_issues->id, '_issueid', '_pluginid', 'R_issues_narrationplugin')
        );
				foreach ($_srcplugs as $_plugid) if ($_plugid) {
					$sql = "SELECT * FROM _plugins WHERE id = $_plugid";
					$query = @$this->rfDB->query($sql);
					$_plug = @$this->rfDB->fetch_object($query);
					if ($_eName = $this->_getEngine($_plug->_name)) {
						$_plugins[$_eName][] = $_plug;
						if (!$_default || $_default==$_plug->id) {
							$_default = array();
							$_default["plugin"]=$_plug;
							$_default["engine"]=$_eName;
						}
					}
				}
				if (!$_rendering) {
					/* Printing left hand side plugin choice div */
					$this->buffer .= '<div id="plugchoose">';
					foreach ($_plugins as $_enginename => $_plugs) {
						$this->buffer .= '<p class="enginename">'.mb_convert_case($_enginename, MB_CASE_TITLE, "UTF-8").'</p>';
						$col = 0;
						foreach ($_plugs as $_plug) {					
							$this->buffer .= '<p class="pluginname" style="'.(($col%2<>0)?('background-color: #F1F5FA;'):('background-color: #FFFFFF;')).'"><input type="radio" '.(($_default["plugin"]->id==$_plug->id)?('checked="checked"'):('')).' name="plugradio" id="plugradio'.$_plug->id.'" class="plugchangers" value="'.$_plug->id.'"><label for="plugradio'.$_plug->id.'">'.mb_convert_case(str_replace(array($_enginename,"-"),array(""," "),$_plug->_name), MB_CASE_TITLE, "UTF-8").'</label></p>';
							$col++;
						}
					}
					$this->buffer .= '</div>';
					$this->buffer .= '<div id="renderer"></div>';	
					$this->_domready("
					// Render default
					rfgui._ajaxtransport('/Render/".$_mode."/".$_id."/Showlist','renderer');					
					");					
					
				}
				else {
					if ($_default <> 'Showlist') {
						/* Start to distill */
						$distiller_id = uniqid();
						$exec = 'echo "/usr/bin/php '.DOCROOT.SITEPREFIX.'/main.php '.DOCROOT.' '.$_issues->id.' '.$_default["plugin"]->id.' pdf '.$_mode.' '.$this->rfDB->getSessionId().' '.$distiller_id.'"|at -m now';
						exec($exec);
						$sql = ("
							INSERT INTO `_pdf` (`id`, `_file`, `_date`, `_issue`, `_plugin`, `_pages`, `__user__`, `__config__`, `__split__`, `__parentnode__`, `__sort__`) VALUES 
							('', '', '".time()."', '".$_issues->id."', '".$_default["plugin"]->id."', '', '".$this->rfDB->getUserName()."', '$distiller_id', NULL, -1, 1);
						");
						$this->rfDB->query($sql);						
					}
					$sql = ("SELECT _pdf._pages,_pdf._date, _pdf._file,_issues._name FROM  `_pdf` 
						LEFT JOIN _issues ON _issues.id = _pdf._issue 
						WHERE _pdf.__config__ <> 0
						ORDER BY _pdf._date DESC LIMIT 40");
					$_backlist = $this->rfDB->query($sql);
					$this->buffer .= '<div class="previewImage bright">';
					$col=0;
					while ($fields = $this->rfDB->fetch_object($_backlist)) {
						$this->buffer .= '<p class="pluginname" style="'.((($col++)%2<>0)?('background-color: #F1F5FA;'):('background-color: #FFFFFF;')).'"><span style="float:right; width: 65px;"> '.substr($fields->_file,-3).', '.$fields->_pages.' S.</span><a style="float:right; width:150px;" '.(($fields->_file)?('href="/udb/pdfgen/'.$fields->_file.'" target="_blank"'):('href="#"')).'>'.(($fields->_file)?(mb_convert_case(substr($this->_stripslashes($fields->_file),0,22), MB_CASE_TITLE, "UTF-8")):(inProgress)).'</a>'.(date("d-m-Y H:i:s", $fields->_date).': </p>');				
					}
					$this->buffer .= '</div><p class="enginename"><a href="#" class="button mini red medium" id="reloadBacklist">'.ReloadBacklist.'</a></p>';
					$this->_domready("
					// Toggle System Tabs
					$('reloadBacklist').addEvent('click', function(e){
						$('renderer').set('html','');
						rfgui._ajaxtransport('/Render/".$_mode."/".$_id."/Showlist','renderer');					
					});					
					");					
				}
				break;
		}
		if (!$_rendering) $this->_domready("

		// Toggle System Tabs
		$$('.plugchangers').addEvent('click', function(e){
			$('renderer').set('html','');
			rfgui._ajaxtransport('/Render/".$_mode."/".$_id."/' + this.value,'renderer');
		});		
		");
		return ($pdflink);
	}


} // END class 

?>