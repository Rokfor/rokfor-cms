<?php
// used for direct access protection
define('ROK', true);

/*
   -----------------------------------------------------------------------------------
   General Settings

   - Project Name
   - Email Contact
   - Language
   -----------------------------------------------------------------------------------
*/

define("PROJECT_NAME", 			"xxx.com");
define("USERMAIL", 				"xxx@xxx.com");
define("LANGUAGE", 				"deCH");

/*
   -----------------------------------------------------------------------------------
   Database

   - DB Config and Connection Setup
   -----------------------------------------------------------------------------------
*/

define("DBNAME",				"xxx");
define("DBUSER",				"xxx");
define("DBPASS",				"xxx");
define("DBASE",					"localhost");

/*
   -----------------------------------------------------------------------------------
   Paths

   - DOCROOT: Document Root of Webserver
   - SYSDIR: Path to Rokfor Libraries
   - LOGDIR: Path to the Logfile
   - REPOSITORY: Path to the file repository (only accssible, relative).
   - REPOSITORYDIR: Absolute path to the file repository.
   - GENERATORDIR: Absolute path to the document output generator
   - SITEPREFIX: Prefix of the rokfor installation on the webserver (default: /rf)
   -----------------------------------------------------------------------------------
*/

define("DOCROOT", 				$_SERVER["DOCUMENT_ROOT"]?$_SERVER["DOCUMENT_ROOT"]:die("/enter/path/manually/here"));
define("SYSDIR", 				DOCROOT."/../sys");
define("LOGDIR", 				DOCROOT."/../sys/log");
define("REPOSITORY", 			"/udb");
define("REPOSITORYDIR",			DOCROOT.REPOSITORY);
define("GENERATORDIR",			DOCROOT."/../pdf");
define("SITEPREFIX", 			"/rf");

/*
   -----------------------------------------------------------------------------------
   Repository Access Rights

   - PUBLIC_UDB_ACCESS: whole Repository is accessible (inclusive generated docs)
   - UPLOAD_UDB_ACCESS: Uploads are Downloadable 
   - PDF_PREVIEW_ACCESS: Access to preview pdfs (single page) is allowed by the public
   - PDF_FULLTEXT_ACCESS: Access to complete pdfs (single page) is allowed by the public
   - BY_TYPE_ACCESS: Comma Separated List of File Types which are always downloadable
   - OVERWRITE_UPLOADS_WITH_SAMENAME: Overwrite Uploads with the same name (true) or 
     add timestamp to keep the file (false)
   -----------------------------------------------------------------------------------
*/


define("PUBLIC_UDB_ACCESS", 	false);	
define("BY_TYPE_ACCESS", 		'jpg');		
define("UPLOAD_UDB_ACCESS", 	false);		
define("PDF_PREVIEW_ACCESS", 	false);	
define("PDF_FULLTEXT_ACCESS", 	false);	
define("OVERWRITE_UPLOADS_WITH_SAMENAME", false);

/*
   -----------------------------------------------------------------------------------
   API XML Export

   - BOOKS_ALLOWED_FOR_GUESTS: Comma Seperated List of Book Names which are exportable
     by the API XML Call without a password (i.e. as guest account):
     * Url: /rf/Api/XML/Guest/Bookname
     * Booknames with Commas should be wrapped in ""
   -----------------------------------------------------------------------------------
*/

define("BOOKS_ALLOWED_FOR_GUESTS", false);	


/*
   -----------------------------------------------------------------------------------
   Text editing

   - EDITOR: ckeditor / nicEdit / mooEditable / wymeditor (case sensitive)
     choose of one of the editors. currently, ckeditor is best implemented
   - ENABLE_SPELLCHECK: enable/disable (disabled by default)
	 spell Check only works with rtf enabled text fields.
   - STORE_HTML: enable/disable (disabled by default)
	 either store cleaned up html in the database (default, suitable for print output)
	 or leave the content untouched from the rtf-editor (suitable for web applications)
   - TEXTAREAMAXLINES: Maximum Lines of Textareas - if not limited by the configuration.
     Used only if (textlength/80) exceeds this limitation.
   -----------------------------------------------------------------------------------
*/

define("EDITOR",				"ckeditor");
define("ENABLE_SPELLCHECK",		false);
define("STORE_HTML", 			false);	
define("TEXTAREAMAXLINES",		30);

/*
   -----------------------------------------------------------------------------------
   Image Editing

   - Image size: default value, will be overwritten by the field settings. Used as a 
     fall back value.
   -----------------------------------------------------------------------------------
*/


define("PREVIEW_WIDTH", 		450); 	// Width in Pixels of the Preview Copy of uploaded Images    
define("PREVIEW_HEIGHT", 		300);	// Height in Pixels of the Preview Copy of uploaded Images
define("JPEG_COMPRESSION", 		75);	// Default JPEG Compression

/*
   -----------------------------------------------------------------------------------
   Binary paths

   - ffmpeg: path to ffmpeg on the system
   - ffmpeg2theora: path to ffmpeg2theora, for html5 previews of videos
   -----------------------------------------------------------------------------------
*/

define("ffmpeg",				"/usr/local/bin/ffmpeg");
define("ffmpeg2theora", 		"/usr/bin/ffmpeg2theora");


/*
   -----------------------------------------------------------------------------------
   Tree & Interface Settings

   - HIDEUSER: Hide users and and empty or single issues/chapter in the Tree
   - HIDEBUTTONS: Hide Buttons (admins and super-admins not affected). Comma separated list.
   - SHOWCLOSEDCONTRIB: Show Closed Contributions for regular Users?		
   -----------------------------------------------------------------------------------
*/

define("HIDEUSER", 				true);			
define("HIDEBUTTONS", 			"ChangeTemplate,CopyData,Rename,SaveNew,SaveNext,SavePrev,PageExport");
define("SHOWCLOSEDCONTRIB",		false);
define("SHOWDELETEDCONTRIB", 	false);


?>
