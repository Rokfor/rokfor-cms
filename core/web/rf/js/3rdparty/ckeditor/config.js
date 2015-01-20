/*
Copyright (c) 2003-2011, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

CKEDITOR.stylesSet.add( 'my_styles',
[
	// Block-level styles
	{ name : 'Marked 5', element : 'h5'},
	{ name : 'Marked 6' , element : 'h6'}
	
]);

CKEDITOR.editorConfig = function( config )
{
        config.allowedContent = true;
	config.width = '100%';
	config.toolbar = 'MyToolbar';
 	config.resize_enabled = false;
	config.startupShowBorders = false;
	config.toolbar_MyToolbar =
	[
		{ name: 'basicstyles', items : [ 'Bold','Italic','Underline','-','RemoveFormat' ] },	
		{ name: 'document', items : [ 'NewPage','Source' ] },
		{ name: 'clipboard', items : [ /*'Cut','Copy','Paste',*/'PasteText','PasteFromWord'/*,'-','Undo','Redo'*/ ] },
		{ name: 'editing', items : [ 'Find'/*,'Replace'*/] },
		{ name: 'paragraph', items : [ '-', 'NumberedList','BulletedList','Blockquote'] },
	    { name: 'insert', items : ['HorizontalRule','PageBreak' ] },
		{ name: 'links', items : [ 'Link','Unlink','Anchor' ] },
		{ name: 'styles', items : [ 'Styles'] }		
	];	
    config.removePlugins = 'elementspath';
	config.coreStyles_italic = { element : 'i', overrides : 'em' };
	config.coreStyles_bold = { element : 'b', overrides : 'strong' };
	config.tabSpaces = 4;	
	config.stylesSet = 'my_styles';	
	config.toolbarStartupExpanded = false; 
	config.enterMode = CKEDITOR.ENTER_P;	
	config.DefaultLinkTarget = '_blank';

	config.keystrokes =
	[
	    [ CKEDITOR.CTRL + 90 /*Z*/, 'undo' ],
	    [ CKEDITOR.CTRL + 89 /*Y*/, 'redo' ],
	    [ CKEDITOR.CTRL + CKEDITOR.SHIFT + 90 /*Z*/, 'redo' ],
	    [ CKEDITOR.CTRL + 66 /*B*/, 'bold' ],
	    [ CKEDITOR.CTRL + 73 /*I*/, 'italic' ],
	    [ CKEDITOR.CTRL + 85 /*U*/, 'underline' ],
	    [ CKEDITOR.ALT + 109 /*-*/, '' ]
	];	
	
	
};
