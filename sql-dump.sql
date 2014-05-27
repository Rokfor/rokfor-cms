

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `session`
--

CREATE TABLE IF NOT EXISTS `session` (
  `id` bigint(40) NOT NULL AUTO_INCREMENT,
  `session` bigint(40) NOT NULL DEFAULT '0',
  `userid` tinyint(4) NOT NULL DEFAULT '0',
  `starttime` bigint(40) NOT NULL DEFAULT '0',
  `currenttime` bigint(40) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `username` text NOT NULL,
  `password` text NOT NULL,
  `usergroup` text NOT NULL,
  `filerights` text NOT NULL,
  `pluginrights` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `id` (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

INSERT INTO `users` (`id`, `username`, `password`, `usergroup`, `filerights`, `pluginrights`) VALUES
(1, 'root', '81dc9bdb52d04dc20036dbd8313ed055', 'root', '', '');

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_batch`
--

CREATE TABLE IF NOT EXISTS `_batch` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_name` text,
  `_description` text,
  `_precode` text,
  `_postcode` text,
  `_forbook` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_books`
--

CREATE TABLE IF NOT EXISTS `_books` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_name` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

--
-- Trigger `_books`
--
DROP TRIGGER IF EXISTS `Delete Chapter or Issue after Book Delete`;
DELIMITER //
CREATE TRIGGER `Delete Chapter or Issue after Book Delete` AFTER DELETE ON `_books`
 FOR EACH ROW BEGIN
		DELETE FROM _formats WHERE _forbook = OLD.id;
		DELETE FROM _issues WHERE _forbook = OLD.id;
		END
//
DELIMITER ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_contributions`
--

CREATE TABLE IF NOT EXISTS `_contributions` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_fortemplate` int(32) DEFAULT NULL,
  `_forissue` int(32) DEFAULT NULL,
  `_name` text,
  `_status` text,
  `_newdate` int(40) DEFAULT NULL,
  `_moddate` int(40) DEFAULT NULL,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `_contributionfortemplate_index` (`_fortemplate`),
  KEY `_contributionforissue_index` (`_forissue`),
  KEY `_contributionname_index` (`_name`(10))
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_data`
--

CREATE TABLE IF NOT EXISTS `_data` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_forcontribution` int(32) DEFAULT NULL,
  `_fortemplatefield` int(32) DEFAULT NULL,
  `_datatext` text,
  `_databinary` text,
  `_datainteger` int(32) DEFAULT NULL,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `_datainteger_index` (`_datainteger`),
  KEY `_dataforcontribution_index` (`_forcontribution`),
  KEY `_datafortemplatefield_index` (`_fortemplatefield`),
  FULLTEXT KEY `_datatext_index` (`_datatext`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_fieldpostprocessor`
--

CREATE TABLE IF NOT EXISTS `_fieldpostprocessor` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_name` text,
  `_forfield` text,
  `_code` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_formats`
--

CREATE TABLE IF NOT EXISTS `_formats` (
  `id` bigint(32) NOT NULL AUTO_INCREMENT,
  `_name` text NOT NULL,
  `_forbook` int(32) DEFAULT NULL,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__sort__` int(32) DEFAULT NULL,
  `__parentnode__` int(32) DEFAULT NULL,
  KEY `id` (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_issues`
--

CREATE TABLE IF NOT EXISTS `_issues` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_name` text,
  `_opendate` int(40) DEFAULT NULL,
  `_closedate` int(40) DEFAULT NULL,
  `_status` text,
  `_infotext` text,
  `_forbook` int(32) DEFAULT NULL,
  `_singleplugin` text,
  `_allplugin` text,
  `_rtfplugin` text,
  `_xmlplugin` text,
  `_narrationplugin` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

--
-- Trigger `_issues`
--
DROP TRIGGER IF EXISTS `Delete Contributions after Issues Delete`;
DELIMITER //
CREATE TRIGGER `Delete Contributions after Issues Delete` AFTER DELETE ON `_issues`
 FOR EACH ROW BEGIN
		DELETE FROM _contributions WHERE _forissue = OLD.id;
		END
//
DELIMITER ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_log`
--

CREATE TABLE IF NOT EXISTS `_log` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_ip` text,
  `_agent` text,
  `_user` text,
  `_date` int(40) DEFAULT NULL,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_pdf`
--

CREATE TABLE IF NOT EXISTS `_pdf` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_file` text,
  `_date` int(40) DEFAULT NULL,
  `_issue` int(32) DEFAULT NULL,
  `_plugin` int(32) DEFAULT NULL,
  `_pages` int(32) DEFAULT NULL,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_plugins`
--

CREATE TABLE IF NOT EXISTS `_plugins` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_name` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  `_page` longtext,
  `_config` longtext,
  `_callback` longtext,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_rights`
--

CREATE TABLE IF NOT EXISTS `_rights` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_group` text,
  `_fortemplate` text,
  `_forissue` text,
  `_forbook` text,
  `_foruser` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_templatenames`
--

CREATE TABLE IF NOT EXISTS `_templatenames` (
  `id` bigint(32) NOT NULL AUTO_INCREMENT,
  `_name` text NOT NULL,
  `_helptext` text,
  `_helpimage` text NOT NULL,
  `_inchapter` text NOT NULL,
  `_forbook` text,
  `_category` text,
  `_public` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__sort__` int(32) DEFAULT NULL,
  `__parentnode__` int(32) DEFAULT NULL,
  KEY `id` (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

--
-- Trigger `_templatenames`
--
DROP TRIGGER IF EXISTS `Delete Template Fields after Template Delete`;
DELIMITER //
CREATE TRIGGER `Delete Template Fields after Template Delete` AFTER DELETE ON `_templatenames`
 FOR EACH ROW BEGIN
		DELETE FROM _templates WHERE _fortemplate = OLD.id;
		END
//
DELIMITER ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `_templates`
--

CREATE TABLE IF NOT EXISTS `_templates` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `_fortemplate` int(32) DEFAULT NULL,
  `_fieldname` text,
  `_helpdescription` text,
  `_helpimage` text,
  `_fieldtype` text,
  `_maxlines` int(32) DEFAULT NULL,
  `_textlength` int(32) DEFAULT NULL,
  `_imagewidth` text,
  `_imageheight` text,
  `_cols` int(32) DEFAULT NULL,
  `_colNames` text,
  `_history` text,
  `_growing` text,
  `_lengthInfluence` text,
  `__user__` text,
  `__config__` text,
  `__split__` text,
  `__parentnode__` int(32) DEFAULT NULL,
  `__sort__` int(32) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

--
-- Trigger `_templates`
--
DROP TRIGGER IF EXISTS `Delete Data after Template Delete`;
DELIMITER //
CREATE TRIGGER `Delete Data after Template Delete` AFTER DELETE ON `_templates`
 FOR EACH ROW BEGIN
		DELETE FROM _data WHERE _fortemplatefield = OLD.id;
		END
//
DELIMITER ;
