<?php


/* Login Pop */

if (!defined("Compat"))						define("Compat","Rokfor supports Firefox, Safari, Chrome, Opera and IE 8.");
if (!defined("Benutzer"))					define("Benutzer","Benutzer");
if (!defined("Passwort"))					define("Passwort","Passwort");
if (!defined("Login"))						define("Login","Login");
if (!defined("PleaseLogin"))				define("PleaseLogin","Bitte anmelden");
if (!defined("LoggedOut"))					define("LoggedOut","Abgemeldet");
if (!defined("SessionExpired"))				define("SessionExpired","Session abgelaufen");

/* Karteireiter */

if (!defined("Projekte"))					define("Projekte","Projekte");
if (!defined("Archiv"))						define("Archiv","Archiv");
if (!defined("Suchen"))						define("Suchen","Suchen");
if (!defined("Batch"))						define("Batch","Batch");
if (!defined("System"))						define("System","System");

/* System Tab */
if (!defined("Sortorder"))					define("Sortorder","Sortiereinstellungen");
if (!defined("Adddate"))					define("Adddate","Erfassungsdatum");
if (!defined("Moddate"))					define("Moddate","Modifikationsdatum");
if (!defined("Manualsort"))					define("Manualsort","Manuell");
if (!defined("Id"))							define("Id","Id");
if (!defined("ASC"))						define("ASC","Auf");
if (!defined("DESC"))						define("DESC","Ab");


if (!defined("FieldManagement"))			define("FieldManagement","Felder ändern");
if (!defined("NewTemplate"))				define("NewTemplate","Verfügbare Templates bearbeiten");
if (!defined("TemplateProperties"))			define("TemplateProperties","Templates bearbeiten");
if (!defined("UsersRights"))				define("UsersRights","Benutzer & Rechte");
if (!defined("ModifyUsers"))				define("ModifyUsers","Benutzer bearbeiten");
if (!defined("ModifyRights"))				define("ModifyRights","Rechte bearbeiten");
if (!defined("Plugins"))					define("Plugins","Plugins");
if (!defined("ModifyPlugins"))				define("ModifyPlugins","Plugins verwalten");
if (!defined("ModifyFieldpostprocessor"))	define("ModifyFieldpostprocessor","Feldprozessoren verwalten");
if (!defined("ModifyBatchs"))				define("ModifyBatchs","Stapelverarbeitung verwalten");
if (!defined("DatabaseManagement"))			define("DatabaseManagement","Datenbankverwaltung");
if (!defined("ModifyContributions"))		define("ModifyContributions","Beiträge & Templates synchronisieren");
if (!defined("CleanOrphans"))				define("CleanOrphans", "Verwaiste Beiträge & Daten löschen");
if (!defined("EmptyTrash"))					define("EmptyTrash", "Gelöschte Beiträge vernichten");
if (!defined("PleaseChoose"))				define("PleaseChoose","Bitte wählen...");
if (!defined("Changes"))					define("Changes","Veränderte Datensätze");
if (!defined("Fields"))						define("Fields","Felder");
if (!defined("Contributions"))				define("Contributions","Beiträge");
if (!defined("Deleted"))					define("Deleted","Gelöscht");
if (!defined("CallbackManagement"))			define("CallbackManagement","Plugins, Feld- & Stapelaktionen");


/* Intro */

if (!defined("Quickaccess"))				define("Quickaccess","Direktzugriff");
if (!defined("EditLast"))					define("EditLast","Zum neusten Dokument springen");
if (!defined("CloneLast"))					define("CloneLast","Neustes Dokument klonen &amp; öffnen");
if (!defined("LoadHelp"))					define("LoadHelp","Hilfedatei laden (pdf)");
if (!defined("Copyright"))					define("Copyright","© Rokfor 2008-".date("Y"));
if (!defined("Help"))						define("Help","Hilfe");

/* 
 Javascript: All Constants beginning with rfstring_ are transferred to Javascript Constants
 in the constants.php File in the Javascript Directory.
 */

if (!defined("rfstring_NEW"))				define("rfstring_NEW", "Neues Dokument");
if (!defined("rfstring_SORT"))				define("rfstring_SORT", "Sortieren");
if (!defined("rfstring_CLONE"))				define("rfstring_CLONE", "Kopieren");
if (!defined("rfstring_RELEASE"))			define("rfstring_RELEASE", "Sperren");
if (!defined("rfstring_UNRELEASE"))			define("rfstring_UNRELEASE", "Freigeben");
if (!defined("rfstring_DELETE"))			define("rfstring_DELETE", "Löschen");
if (!defined("rfstring_UNDELETE"))			define("rfstring_UNDELETE", "Wiederherstellen");
if (!defined("rfstring_TRASH"))				define("rfstring_TRASH", "Vernichten");
if (!defined("rfstring_CANCEL"))			define("rfstring_CANCEL", "Abbrechen");
if (!defined("rfstring_OK"))				define("rfstring_OK", "OK");
if (!defined("rfstring_EXPORTCONTRIB"))		define("rfstring_EXPORTCONTRIB", 'Exportieren');
if (!defined("rfstring_EXPORTISSUE"))		define("rfstring_EXPORTISSUE", "Exportieren");
if (!defined("rfstring_CLONEISSUE"))		define("rfstring_CLONEISSUE", 'Duplizieren');
if (!defined("rfstring_CLOSEISSUE"))		define("rfstring_CLOSEISSUE", 'Archivieren');
if (!defined("rfstring_REOPENISSUE"))		define("rfstring_REOPENISSUE", 'Reaktivieren');
if (!defined("rfstring_SYSTEM"))			define("rfstring_SYSTEM", PROJECT_NAME);
if (!defined("rfstring_CLONEQUESTION"))		define("rfstring_CLONEQUESTION", 'Beitrag duplizieren?');
if (!defined("rfstring_IMPORTQUESTION"))	define("rfstring_IMPORTQUESTION", 'Anderer Datensatz importieren?');
if (!defined("rfstring_RELEASEQUESTION"))	define("rfstring_RELEASEQUESTION", 'Beitrag sperren? Er kann danach nicht mehr bearbeitet werden.');
if (!defined("rfstring_UNRELEASEQUESTION"))	define("rfstring_UNRELEASEQUESTION", 'Beitrag freigeben? Danach kann er vom Autor wieder editiert werden.');
if (!defined("rfstring_DELETEQUESTION"))	define("rfstring_DELETEQUESTION", 'Beitrag löschen? Er kann notfalls durch den Administrator wiederhergestellt werden.');
if (!defined("rfstring_UNDELETEQUESTION"))	define("rfstring_UNDELETEQUESTION", 'Beitrag wiederherstellen?');
if (!defined("rfstring_TRASHQUESTION"))		define("rfstring_TRASHQUESTION", 'Alle Daten des Beitrags werden gelöscht. Er kann danach nicht mehr wiederhergestellt werden.');
if (!defined("rfstring_NEWQUESTION"))		define("rfstring_NEWQUESTION", 'Bitte Vorlage und Name wählen:');
if (!defined("rfstring_CLONEQUESTION"))		define("rfstring_CLONEQUESTION", 'Beitrag wählen, aus dem die Daten importiert werden sollen:');
if (!defined("rfstring_RENAMEQUESTION"))	define("rfstring_RENAMEQUESTION", 'Beitrag umbenennen in:');
if (!defined("rfstring_CHANGETEMPLATEQUESTION"))	define("rfstring_CHANGETEMPLATEQUESTION", 'Neue Vorlage wählen. Achtung: Es können Daten verloren gehen, falls die neue Vorlage weniger Felder aufweist!');
if (!defined("rfstring_CLOSEISSUEQUESTION"))		define("rfstring_CLOSEISSUEQUESTION", 'Ausgabe schliessen? Normale Benutzer können die Beiträge nicht mehr öffnen.<br>Die Ausgabe ist jedoch weiterhin im Archiv verfügbar.');
if (!defined("rfstring_CLONEISSUEQUESTION"))		define("rfstring_CLONEISSUEQUESTION", 'Ausgabe Duplizieren? Sie müssen danach die Berechtigungen für die neue Ausgabe anpassen.');
if (!defined("rfstring_REOPENISSUEQUESTION"))		define("rfstring_REOPENISSUEQUESTION", 'Ausgabe Reaktivieren? Normale Benutzer können sie dann wieder unter Projekte bearbeiten.');
if (!defined("rfstring_ISSUEEXPORTQUESTION"))		define("rfstring_ISSUEEXPORTQUESTION", 'Ausgabe Exportieren');
if (!defined("rfstring_ADDISSUE"))			define("rfstring_ADDISSUE", 'Ausgaben');
if (!defined("rfstring_ADDCHAPTER"))		define("rfstring_ADDCHAPTER", 'Kapitelstruktur');
if (!defined("rfstring_ADDBOOK"))			define("rfstring_ADDBOOK", 'Bücher verwalten');
if (!defined("rfstring_MAXLENGTH"))			define("rfstring_MAXLENGTH", 'Limit');
if (!defined("rfstring_LENGTH"))			define("rfstring_LENGTH", 'Zeichen');
if (!defined("rfstring_DELETEIMGQUESTION"))	define("rfstring_DELETEIMGQUESTION", 'Datei wirklich löschen?');
if (!defined("rfstring_MISSINGNAME"))		define("rfstring_MISSINGNAME", 'Bitte Namen für Beitrag angeben.');
if (!defined("rfstring_MISSINGCLONE"))		define("rfstring_MISSINGCLONE", 'Fehler beim Duplizieren: Id der Quelle unbekannt.');
if (!defined("rfstring_CLONEADDITION"))		define("rfstring_CLONEADDITION", ' [Kopie]');
if (!defined("rfstring_STORESETTINGS"))		define("rfstring_STORESETTINGS", 'Einstellungen gespeichert');
if (!defined("rfstring_SAVEASQUESTION"))	define("rfstring_SAVEASQUESTION", 'Speichern als:');
if (!defined("rfstring_SORTERROR"))			define("rfstring_SORTERROR", 'Achtung:<br>Weiter & Zurück ist nicht möglich, wenn die Ansicht nach dem Modifikationsdatum sortiert wird.');
if (!defined("rfstring_BOOKEDITOR"))		define("rfstring_BOOKEDITOR", 'Bücherliste bearbeiten');
if (!defined("rfstring_CHAPTEREDITOR"))		define("rfstring_CHAPTEREDITOR", 'Kapitelliste des Buches bearbeiten');
if (!defined("rfstring_ISSUEEDITOR"))		define("rfstring_ISSUEEDITOR", 'Ausgaben (Issues) bearbeiten');
if (!defined("rfstring_SORTCHAPTEREDITOR"))		define("rfstring_SORTCHAPTEREDITOR", 'Beiträge manuell sortieren');
if (!defined("rfstring_CHECKDATACONSISTENCY"))	define("rfstring_CHECKDATACONSISTENCY", 'Beiträge werden den Templates angepasst, Felder erweitert, gelöscht oder die Reihenfolge verändert.<br/>Der Vorgang kann sehr lange dauern.');
if (!defined("rfstring_TEMPLATELISTEDITOR"))	define("rfstring_TEMPLATELISTEDITOR", 'Liste der Vorlagen bearbeiten');
if (!defined("rfstring_CLEANORPHANS"))		define("rfstring_CLEANORPHANS", 'Alle Daten ohne existierenden Beitrag und alle Beiträge ohne existierende Vorlagen werden gelöscht.');
if (!defined("rfstring_EMPTYTRASH"))		define("rfstring_EMPTYTRASH", 'Alle gelöschten Beiträge werden vernichtet.');
if (!defined("rfstring_USERLISTEDITOR"))	define("rfstring_USERLISTEDITOR", 'Benutzer verwalten');
if (!defined("rfstring_RIGHTSLISTEDITOR"))	define("rfstring_RIGHTSLISTEDITOR", 'Rechte verwalten');
if (!defined("rfstring_PLUGINSLISTEDITOR"))	define("rfstring_PLUGINSLISTEDITOR", 'Plugins verwalten');
if (!defined("rfstring_SITEPREFIX"))		define("rfstring_SITEPREFIX", SITEPREFIX);
if (!defined("rfstring_SHOWCLOSEDCONTRIB"))		define("rfstring_SHOWCLOSEDCONTRIB", defined("SHOWCLOSEDCONTRIB")?SHOWCLOSEDCONTRIB:false);
if (!defined("rfstring_SHOWDELETEDCONTRIB"))		define("rfstring_SHOWDELETEDCONTRIB", defined("SHOWDELETEDCONTRIB")?SHOWDELETEDCONTRIB:false);
if (!defined("rfstring_TEMPLATEFIELDEDITOR"))	define("rfstring_TEMPLATEFIELDEDITOR","Felder des Templates bearbeiten");             
if (!defined("rfstring_FIELDPOSTPROCESSOREDITOR"))	define("rfstring_FIELDPOSTPROCESSOREDITOR","Feldprozessoren bearbeiten");             
if (!defined("rfstring_BATCHEDITOR"))		define("rfstring_BATCHEDITOR","Stapelverarbeitung bearbeiten");             
if (!defined("rfstring_MOVEQUESTION"))		define("rfstring_MOVEQUESTION","Beitrag in ein anderes Kapitel/Buch verschieben");             

/*                                                   
  Context Menu Buttons                              
*/                                                       

if (!defined("ChangeTemplate"))				define("ChangeTemplate","Vorlage ändern");
if (!defined("CopyData"))					define("CopyData","Daten importieren");
if (!defined("Rename"))						define("Rename","Umbenennen");
if (!defined("Save"))						define("Save","Eingabe speichern");
if (!defined("SaveNew"))					define("SaveNew","Speichern Als");
if (!defined("SaveNext"))					define("SaveNext","Speichern & Weiter");
if (!defined("SavePrev"))					define("SavePrev","Speichern & Zurück");
if (!defined("PageExport"))					define("PageExport","Seite exportieren");
if (!defined("PageMove"))					define("PageMove","Seite verschieben");

/*
Status Info 
*/
if (!defined("StatusTitle"))				define("StatusTitle","Informationen");
if (!defined("Projekt"))					define("Projekt","Projekt");
if (!defined("CreatedBy"))					define("CreatedBy","Erstellt von");
if (!defined("Issue"))						define("Issue","Ausgabe");
if (!defined("Deadline"))					define("Deadline","Abgabe");
if (!defined("Days"))						define("Days","Tage");
if (!defined("HinweisLink"))				define("HinweisLink",'Bemerkungen');


/* 
  Main Form: Text Fields
*/

if (!defined("Changed"))					define("Changed","Geändert");
if (!defined("Added"))						define("Added","Erfasst");
if (!defined("Template"))					define("Template","Vorlage");
if (!defined("Zeile"))						define("Zeile","Zeile/n");
if (!defined("History"))					define("History","Verlauf");
if (!defined("Loading"))					define("Loading","Wird berechnet...");
if (!defined("Maxlines"))					define("Maxlines","Max. Zeilenzahl");
if (!defined("isSaved"))					define("isSaved","ist gespeichert");
if (!defined("ErrSaveWordlist"))			define("ErrSaveWordlist","Could not save word list.");
if (!defined("ErrAddWordlist"))				define("ErrAddWordlist","Could not add to word list.");
if (!defined("ErrOpenWordlist"))			define("ErrOpenWordlist","Could not open dictionary.");


/*
  Main Form: Typology Select / History / Legends
*/
if (!defined("HistoryDisabled"))	define("HistoryDisabled","— Verlauf ausgeschaltet —");
if (!defined("HistoryOther"))		define("HistoryOther","Quelle des Feldes $1 in $2");
if (!defined("HistoryContributional"))	define("HistoryContributional","Quelle: $1");
if (!defined("HistoryStructural"))	define("HistoryStructural","Templatestruktur: $1");
if (!defined("HistoryOwn"))			define("HistoryOwn","Eigener Verlauf");
if (!defined("HistoryIssue"))		define("HistoryIssue","Verfügbare Issues");
if (!defined("HistoryChapter"))		define("HistoryChapter","Verfügbare Kapitel");
if (!defined("HistoryBooks"))		define("HistoryBooks","Verfügbare Bücher");
if (!defined("FixedHistory"))		define("FixedHistory","Quelle: Fixwerte");
if (!defined("SelectDisabled"))		define("SelectDisabled","— Kein Wert gesetzt —");
if (!defined("FromTemplateErr"))	define("FromTemplateErr","Fromtemplate not defined!");
if (!defined("FromFieldErr"))		define("FromFieldErr","Fromfield not defined!");
if (!defined("FixedValuesErr"))		define("FixedValuesErr","Fixedvalues not defined!");

/*
  Main Form: Date / integer
*/
if (!defined("Datefield"))		define("Datefield","Datum");
if (!defined("Timefield"))		define("Timefield","Uhr");

/*
  Main Form: Image
*/
if (!defined("Printsize"))		define("Printsize","Druckgrösse");
if (!defined("Unit"))			define("Unit","mm");
if (!defined("Insert"))			define("Insert","Einfügen");
if (!defined("Preview"))		define("Preview","View");
if (!defined("Download"))		define("Download","Download");
if (!defined("Delete"))			define("Delete","X");

/*
  Main Form: Table
*/
if (!defined("MaxlinePerField"))	define("MaxlinePerField","Zeilen pro Feld");
if (!defined("MaxcharPerField"))	define("MaxcharPerField","Zeichen pro Feld");

/*
  Main Form: File Picker
*/
if (!defined("File"))				define("File","Datei");
if (!defined("conversion_started"))	define("conversion_started","Konvertierung gestartet");
if (!defined("conversion_running"))	define("conversion_running","Laufende Konvertierung");

/*
 Ajax Forms: Add Contribution
*/
if (!defined("Name"))				define("Name","Name");
if (!defined("NoTemplates"))		define("NoTemplates","Keine Vorlagen verfügbar");

/*
 Ajax Forms: Clone Contribution
*/
if (!defined("ExistingContribs"))	define("ExistingContribs","Beiträge");
if (!defined("NoContribs"))			define("NoContribs","Keine Beiträge verfügbar");

/*
 Ajax Forms: Move Contribution
*/
if (!defined("MoveToTarget"))		define("MoveToTarget","Ziel");


/*
 Issue & Chapter & Book Popup
*/
if (!defined("ClosedDocs"))			define("ClosedDocs","Geschlossen");
if (!defined("OpenDocs"))			define("OpenDocs","Offen");

if (!defined("SinglePlugin"))			define("SinglePlugin","Einzelseiten (PDF)");
if (!defined("AllPlugin"))				define("AllPlugin","Gesamtausgaben (PDF)");
if (!defined("RTFPlugin"))				define("RTFPlugin","Gesamtausgaben (RTF/DOC)");
if (!defined("XMLPlugin"))				define("XMLPlugin","Gesamtausgaben (XML)");
if (!defined("NarrationPlugin"))		define("NarrationPlugin","Spezialvarianten");

if (!defined("IssuePrivate"))			define("IssuePrivate","Privat");					// 0
if (!defined("IssuePublic"))			define("IssuePublic","Öffentlich");					// 1
if (!defined("IssueAllowDownload"))		define("IssueAllowDownload","Nur Downloads");		// 2
if (!defined("IssueAllowPreview"))		define("IssueAllowPreview","Nur Einzelseiten");		// 3
if (!defined("IssueAllowFulltext"))		define("IssueAllowFulltext","Nur Gesamtausgaben");	// 4
if (!defined("IssueAllowAllPdf"))		define("IssueAllowAllPdf","Alle PDFs");				// 5


/* 
  Callbacks 
*/

if (!defined("CallbackPreMessage"))	define("CallbackPreMessage", '/*
batch preprocessing function. Passed Parameters:
&$_db		(mysql class)
&$_rokforDB (rokfor db)
&$issueid	(int)
&$bookid	(int)

form elements in the return string are passed to the postprocessor.

*/
return ("HTML String");');

if (!defined("CallbackAfterMessage")) define("CallbackAfterMessage", '/*
batch postprocessing function. Passed Parameters:
&$_db
&$_rokforDB
&$issueid
&$bookid
&$batchid
&$batchpost
&$contribs
&$templates
&$_caller

$batchpost contains the whole $_POST array.

*/
return ("HTML Code shown in popup.");');

if (!defined("CallbackFieldAlert")) define('CallbackFieldAlert', '
/* 
  Field Processor Parameter:

  &$dbMySQL Database Class
  &$data: Field Data
  &$_field_info: Field Specifications as Arraay
  &$rfDB: Rokfor Database Class
  &$_field_info_as_object: Field Specifications as Object (from rfDB->getFieldData)

  - Modify $data in this function
  - If your result is stored in a different variable than $data, add return($yourvar);
  - return($data) is added automatically
*/
');

if (!defined("CallbackExecutionError")) define('CallbackExecutionError', 'Bei der Ausführung des Feldprozessors ist ein Fehler aufgetreten. Bitte den Code überprüfen.');
                                                                       
/*
  Batches
*/
if (!defined("Run"))				define("Run","Ausführen");
if (!defined("BatchPreError"))	    define("BatchPreError","Kein Pre Code vorhanden.");
if (!defined("BatchPostError"))	    define("BatchPostError","Post Code fehlerhaft.");
if (!defined("RunBatch"))			define("RunBatch","Running Batch Id");
if (!defined("NoBatch"))			define("NoBatch","Keine Stapelverarbeitung");
if (!defined("NoBatchText"))		define("NoBatchText", "Keine Stapelverarbeitungen installiert. Bitte fragen sie ihren <a href='mailto:".USERMAIL."'>Administrator</a>.");

/*
  Rendering
*/
if (!defined("NoRendererActivated")) 		define("NoRendererActivated","Für diesen Beitrag steht keine PDF Funktion zur Verfügung.");
if (!defined("RendererError")) 				define("RendererError","Fehler!");
if (!defined("RendererGeneratorFailure")) 	define("RendererGeneratorFailure","Fehler beim Produzieren der Datei. Logfiles überprüfen.");
if (!defined("Pages")) 						define("Pages","Seite(n)");
if (!defined("ReloadBacklist"))				define("ReloadBacklist","Liste neu laden");
if (!defined("inProgress"))					define("inProgress","in Bearbeitung...");


/*
  Search Replace 
*/

if (!defined("SearchReplace"))		define("SearchReplace","Suchen &amp; Ersetzen");
if (!defined("Suchbegriff"))		define("Suchbegriff","Suchbegriff");
if (!defined("CaseSensitive"))		define("CaseSensitive","Gr./Kl. berücksichtigen");
if (!defined("UseRegex"))			define("UseRegex","Platzhalter erlauben");
if (!defined("Suchhelp"))			define("Suchhelp","Erlaubte Platzhalter: ? = 1 Z. * = 2+ Z.");
if (!defined("Ersetzen"))			define("Ersetzen","Ersetzen mit");
if (!defined("Fieldlist"))			define("Fieldlist","Feldliste");
if (!defined("ExistingIssues"))		define("ExistingIssues","Ausgaben");
if (!defined("Range"))				define("Range","Suchbereich");
if (!defined("LimitSearch"))		define("LimitSearch",  "Geöffneter Datensatz");
if (!defined("NoLimitSearch"))		define("NoLimitSearch","Ausgewählte Ausgaben");
if (!defined("AllSearch"))			define("AllSearch",    "Gesamte Datenbank");
if (!defined("NotValidRegexp"))		define("NotValidRegexp","Kein Gültiger Suchterm.");
if (!defined("CheckSyntax"))		define("CheckSyntax","Check the Syntax");
if (!defined("Resultate"))			define("Resultate","Resultate");
if (!defined("Substitutes"))		define("Substitutes","Ersetzungen");
if (!defined("Hits"))				define("Hits","Treffer");
if (!defined("NoResults"))			define("NoResults","Keine Treffer gefunden.");


/* 
  Logged In / Other Users
*/

if (!defined("OtherUsers"))			define("OtherUsers","Andere Benutzer");
if (!defined("LoggedInAs"))			define("LoggedInAs","Angemeldet als");
if (!defined("Since"))				define("Since","Letzte Aktivität vor");		
if (!defined("Minutes"))			define("Minutes","Minuten");		



?>