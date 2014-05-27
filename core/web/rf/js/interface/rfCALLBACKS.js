var rfCALLBACKS = new Class({
    Implements: [Options, Events],
    options: {
    },

	// Constructor: Load Options, Add Popup Layer
    initialize: function(options){
        this.setOptions(options);
		this.buttons = false;
		this.container = false;
		this.user = false;
    },

	/*
	Tree Callback: Load on doubleclick
	*/
	rfTreeDoubleClickAction: function(node,state,buttons,container) {
		this.buttons = buttons;
		this.container = container;


		/*
			Collapsing a parent node tree always shows the intro and closes the context menu
		*/

		if (!state && rfgui.currentNode != node && rfgui.currentNode.ischildof(node)) {
			this.showIntro();			
			return;
		}

		/* 
		   Load Ajax-Form in the Center if not deleted
		   Fade Out unless Internet Explorer (no gradient Fades) 
		*/
//		if (state && container.id=="tree_container" && node.data.level==5) {
		if (state && node.data.level==5) {
			if (node.data.state!="Deleted") {
				rfgui.currentNode = node;
				/* Hide Intro & Load Context */
				if (rfgui.intro) {
					this.hideIntro();
					if (container.id=="tree_container") rfgui._ajaxtransport('/Context/'+node.data.contribid,$('context'));						
				}
				/* Load Form in all cases - the context menu might be loaded already */
				rfgui._ajaxtransport('/Form/'+node.data.contribid,$('mainform'));						
			}
		}

	},

	/*
	 Tree Callback: Callback Handler der Tree-Class: Generiert die richtigen Buttons passend zum Status des Baums
	*/
	rfTreeAction: function(node,state,buttons,container,userlevel,noload) {
		if (noload==null || !noload) noload = false;
		this.buttons = buttons;
		this.container = container;
		this.user = userlevel;
	
		if (userlevel != "user") var ext = true;
		else var ext =  false;

		/* Preselected? Dann unbedingt im rfgui speichern */
		if (node.preselected) rfgui.currentNode = node;
		
		/* Hidden Level: Nodes mit nur einem Child werden geskipped, ihr Level muss
		   bei der generierung der Buttons aber doch berücksichtig werden */
		var hiddenLevel = Array();
		if (node.data.hiddenlevel) {
			hiddenLevel = node.data.hiddenlevel.split(",");
		}

		/* Aktuell */
		
		var btns = Array(); // Buttons object storage array - used for final fades
		var divider = new Element('p', {html: '' ,'class': 'clearboth'});
		
		if (state) {
			buttons.empty();
			if (container.id=="tree_container") {
			
				/* Ebene: Contribution */

				if (node.data.level==5) {

					/*
						Buttons: Nur Laden, wenn Level geändert od. der Status der Contribution anders ist 
					*/
					if (node.data.state=="Deleted" && (ext || rfstring_SHOWDELETEDCONTRIB)) {
						// Undelete Button (admin)					
						btns.push(buttons.adopt(new Element('a', {href: '#', id: 'undeleteContrib', html: rfstring_UNDELETE,'class': 'button medium green floatright', events: {
							click: this.contributionAction.bind(this,[node,"undelete"])
							}})));
						btns.push(buttons.adopt(new Element('a', {href: '#', id: 'trashContrib', html: rfstring_TRASH,'class': 'button medium red', events: {
							click: this.contributionAction.bind(this,[node,"trash"])
							}})));
							
					}
					else {
						// Clone Button
						btns.push(buttons.adopt(new Element('a', {href: '#', id: 'cloneContrib', html: rfstring_CLONE,'class': 'button small blue', events: {
							click: this.cloneContribution.bind(this,node)
							}})));		
						// Release Button
						if (node.data.state=="Open") btns.push(buttons.adopt(new Element('a', {href: '#', id: 'releaseContrib', html: rfstring_RELEASE,'class': 'button small green', events: {
							click: this.contributionAction.bind(this,[node,"close"])
							}})));					
						// Unrelease Button (admin)
						if (node.data.state=="Close" && (ext || rfstring_SHOWCLOSEDCONTRIB)) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'unreleaseContrib', html: rfstring_UNRELEASE,'class': 'button small dark', events: {
							click: this.contributionAction.bind(this,[node,"unrelease"])
							}})));											
						// Delete Button
						btns.push(buttons.adopt(new Element('a', {href: '#', id: 'deleteContrib', html: rfstring_DELETE,'class': 'button small red', events: {
							click: this.contributionAction.bind(this,[node,"delete"])
							}})));
					}
					buttons.adopt(divider.clone());
				}
				/* Ebene: Kapitel */

				if (node.data.level==4  || hiddenLevel.contains('4')) {
					// Sort Contributions
					btns.push(buttons.adopt(new Element('a', {
						href: '#', id: 'sortChapter', html: rfstring_SORT,'class': 'button medium green floatright', events: {
							click: this.bookchapterissueAction.bind(this,[node,'sortChapter'])
						}
					})));
					// Add Contribution Button				
					btns.push(buttons.adopt(new Element('a', {
						href: '#', id: 'newContrib', html: rfstring_NEW,'class': 'button medium blue', events: {
							click: this.newContribution.bind(this,node)
						}
					})));		
					buttons.adopt(divider.clone());
				}

				/* Ebene: Issue */			

				if (node.data.level==3 || hiddenLevel.contains('3')) {
					// Export Issue (admin)		
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'exportIssue', html: rfstring_EXPORTISSUE,'class': 'button small blue', events: {
							click: this.issueAction.bind(this,[node,'Exportissue'])
						}
					})));
					// Clone Issue (admin)
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'cloneIssue', html: rfstring_CLONEISSUE,'class': 'button small green', events: {
							click: this.issueAction.bind(this,[node,'Cloneissue'])
						}
					})));
					// Archive Issue (admin)
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'closeIssue', html: rfstring_CLOSEISSUE,'class': 'button small red', events: {
							click: this.issueAction.bind(this,[node,'Closeissue'])
						}
					})));
					buttons.adopt(divider.clone());
				}

				/* Ebene: Buch*/			

				if (node.data.level==2) {
					// Manage Chapter (admin)
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'addChapter', html: rfstring_ADDCHAPTER,'class': 'button medium green floatright', events: {
							click: this.bookchapterissueAction.bind(this,[node,"addChapter"])
						}
					})));
					// Manage Issue (admin)		
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'addIssue', html: rfstring_ADDISSUE,'class': 'button medium blue', events: {
							click: this.bookchapterissueAction.bind(this,[node,"addIssue"])
						}
					})));
					buttons.adopt(divider.clone());
				}

				/* Ebene: root*/			
				
				if (node.parent == null) {
					// Manage Books (admin)		
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'addBook', html: rfstring_ADDBOOK,'class': 'button wide blue', events: {
							click: this.bookchapterissueAction.bind(this,[node,"addBook"])
						}
					})));
					buttons.adopt(divider.clone());
				}

				rfgui.previewAjax('/Showinfo',node.data);
				
			}
		
			/* Archiv */
		
			if (container.id=="archive_container") {
			
				/* Ebene: Contribution */
			
				if (node.data.level==5) {
					btns.push(buttons.adopt(new Element('a', {href: '#', id: 'exportContribArchived', html: rfstring_EXPORTCONTRIB,'class': 'button wide blue', events: {click: function(e){}.bind(rfgui)}})));
					buttons.adopt(divider.clone());
				}

				/* Ebene: Issue */

				if (node.data.level==3 || hiddenLevel.contains('3')) {
					
					// Export Issue (admin)		
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'exportIssueArchived', html: rfstring_EXPORTISSUE,'class': 'button medium blue floatright', events: {
							click: this.issueAction.bind(this,[node,'Exportissue'])
						}
					})));
					
					
					if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'reopenIssue', html: rfstring_REOPENISSUE,'class': 'button medium green', events: {
							click: this.issueAction.bind(this,[node,'Reopenissue'])
						}
					})));
					buttons.adopt(divider.clone());
				}
			
			}
		}
		
		/* Fade Buttons unless Internet Explorer (no gradient Fades) on every Level Change */
/*		if (!Browser.ie && container.treeLevel != node.data.level) {
			btns.each(function(el){
				if (typeof(el)=='object' && el != null) el.setStyle('opacity',0).fade(1);
			});
		}
*/		
		/* Adjust height of button container to its content */
		var size = buttons.getComputedSize()
		container.setStyle('bottom',size.totalHeight+'px');
		
		
		container.treeLevel = node.data.level;
		
	},

	/*
	 Tree Callback: Multiselection Callback Handler der Tree-Class
	*/
	rfTreeMultiSelectAction: function(nodes,state,buttons,container,userlevel) {
		this.buttons = buttons;
		this.container = container;
		this.user = userlevel;
		var ext = userlevel!="user"?true:false;

		var btns = Array(); // Buttons object storage array - used for final fades
		var divider = new Element('p', {html: '' ,'class': 'clearboth'});

		/* Ebene: Contribution, Tree Container */
		if (state && container.id=="tree_container" && nodes.pick().data.level==5) {
			buttons.empty();
			/*
				Buttons: Nur Laden, wenn Level geändert od. der Status der Contribution anders ist 
			*/
			if (ext) {
				// Undelete Button (admin)					
				btns.push(buttons.adopt(new Element('a', {href: '#', id: 'undeleteContrib', html: rfstring_UNDELETE,'class': 'button medium green floatright', events: {
					click: this.contributionAction.bind(this,[nodes,"undelete"])
					}})));
				// Trash Button (admin)					
				btns.push(buttons.adopt(new Element('a', {href: '#', id: 'trashContrib', html: rfstring_TRASH,'class': 'button medium red', events: {
					click: this.contributionAction.bind(this,[nodes,"trash"])
					}})));
			}
			buttons.adopt(divider.clone());
			// Release Button
			btns.push(buttons.adopt(new Element('a', {href: '#', id: 'releaseContrib', html: rfstring_RELEASE,'class': 'button small green', events: {
				click: this.contributionAction.bind(this,[nodes,"close"])
				}})));					
			// Unrelease Button (admin)
			if (ext) btns.push(buttons.adopt(new Element('a', {href: '#', id: 'unreleaseContrib', html: rfstring_UNRELEASE,'class': 'button small dark', events: {
				click: this.contributionAction.bind(this,[nodes,"unrelease"])
				}})));
			// Delete Button
			btns.push(buttons.adopt(new Element('a', {href: '#', id: 'deleteContrib', html: rfstring_DELETE,'class': 'button small red', events: {
				click: this.contributionAction.bind(this,[nodes,"delete"])
				}})));
			buttons.adopt(divider.clone());

			var size = buttons.getComputedSize()
			container.setStyle('bottom',size.totalHeight+'px');		
			container.treeLevel = nodes.pick().data.level;
		}
	},

	/*
	New Contribution Callback - triggered by Add Contribution Button on Click
	*/
	newContribution: function(node) {
		rfgui.rfAjax($('newContrib'),rfstring_NEWQUESTION,'/Ajax/Forms/Addcontribution/'+node.data.projectid+'/'+node.data.editionid+'/'+node.data.chapterid);
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.contribname.value) {
					// Send New Contribution Info and reload tree.
					var callback = function(rfgui) {
						// Tree Reload Callback
						if (rfgui.intro) {
							this.hideIntro();
							rfgui._ajaxtransport('/Context/Latest',$('context'));							
						}
						node.reload(rfgui.intro);							
						rfgui._ajaxtransport('/Form/Latest',$('mainform'));						
					}.bind(this,rfgui);
					rfgui._ajaxtransport('/Newcontribution/'+node.data.editionid+'/'+node.data.chapterid,callback,e);
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGNAME);
				}
			}
		}.bind(this));		
	},
	
	/*
	Close / Delete / Undelete / Unrelease Contribution, return to intro page
	*/
	
	contributionAction: function(node,action) {
		var addClass, removeClass, newState;
		
		if (this.user != "user") var ext = true;
		else var ext =  false;
				
		switch(action)
		{
		case "close":
			action = 'Closecontribution';
			newState = addClass = 'Close';
			rfgui.rfConfirm($('releaseContrib'),rfstring_RELEASEQUESTION);
		  	break;
		case "delete":
			action = 'Deletecontribution';
			newState = addClass = 'Deleted';
			rfgui.rfConfirm($('deleteContrib'),rfstring_DELETEQUESTION);
			break;
		case "unrelease":
			action = 'Unreleasecontribution';
			removeClass = 'Close';
			newState = 'Open';
			rfgui.rfConfirm($('unreleaseContrib'),rfstring_UNRELEASEQUESTION);
		  	break;
		case "undelete":
			action = 'Undeletecontribution';
			removeClass = new Array('Deleted','Close');
			newState = 'Open';
			rfgui.rfConfirm($('undeleteContrib'),rfstring_UNDELETEQUESTION);
			break;
		case "trash":
			action = 'Trashcontribution';
			rfgui.rfConfirm($('trashContrib'),rfstring_TRASHQUESTION);
			break;
		}		
		
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				var callback = function() {
					if (addClass) {
						if (node.length>0) {
							node.each(function(n){
								n.div.text.addClass(addClass);
								n.data.state = newState;
								n.cssclass = addClass;
							});
						}
						else {
							node.div.text.addClass(addClass);
							node.data.state = newState;
							node.cssclass = addClass;
						}
					}
					if (removeClass) {
						if (node.length>0) {
							node.each(function(n){
								if (typeof removeClass =="object") {
									removeClass.each(function(el){
									    n.div.text.removeClass(el);
									});
								}
								else n.div.text.removeClass(removeClass);
								n.data.state = newState;
								n.cssclass = "";
							});
						}
						else {						
							if (typeof removeClass =="object") {
								removeClass.each(function(el){
								    node.div.text.removeClass(el);
								});
							}
							else node.div.text.removeClass(removeClass);
							node.data.state = newState;
							node.cssclass = "";						
						}
					}
					// Remove Node in three cases:
						// always if finally thrashed
					if (action=="Trashcontribution" || 			 
						// If regular user and showclosedcontrib is false
						(!ext && !rfstring_SHOWCLOSEDCONTRIB) || 
						// If regular user, showclosedcontrib is true and action is not close or unrelease
						(!ext && rfstring_SHOWCLOSEDCONTRIB && (action!="Closecontribution"&&action!="Unreleasecontribution"))	// and action is not closecontrib and showclose is false
						)
					 	{
						if (node.length>0) {
							_node = node.pick().parent;
							node.each(function(n){
								n.remove();
							});						
							node = _node;
						}
						else {						
							_node = node.parent;					// Then remove item from tree
							node.remove();
							node = _node;
						}
					}
					// Close Contribution Window - reset to restart - if Trashed or Deleted or Regular User
					if (action=="Trashcontribution" || action=="Deletecontribution" || !ext) {
						this.showIntro();						
					}
					// Update Buttons
					if (node.length>0) this.rfTreeAction(node.pick(), true, this.buttons, this.container,this.user,true);
					else this.rfTreeAction(node, true, this.buttons, this.container,this.user,true);
				}.bind(this,node);
				if (node.length>0) {
					
					var string = [];
					node.each(function(n){
						string.push("data[]=" + n.data.contribid);
					});
					
					rfgui._ajaxtransport('/'+action+'/multi',callback, string.join('&'));				
				}
				else rfgui._ajaxtransport('/'+action+'/'+node.data.contribid,callback);				
			}
		}.bind(this));		
	},
	
	/* 
	Stores the contribution under a new name
	*/
	
	storeAs: function(button,form,container,body) {
		rfgui.rfPrompt(button,rfstring_SAVEASQUESTION,rfgui.currentNode.div.text.get('text'));
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.prompt.value) {
					$(form).set('action',rfstring_SITEPREFIX + "/Ajax/Store/New");
					
					var newname = new Element('input', {
					    type: 'hidden',
					    name: 'newname',
					    value: e.prompt.value
					});
					newname.inject($(form));
					this.storeForm(form,container,body, 'Latest');
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGNAME);
				}
			}
		}.bind(this));		
	},
	

	/*
	Store Form Content, show progress bar, reopen the mainform
	*/
	
	storeForm: function(form,container,body,loadnew) {
		if (typeof form == 'string') form = $(form);
		if (typeof container == 'string') container = $(container);		
		if (typeof body == 'string') body = $(body);				
		if (loadnew == null) loadnew = false;

		var progress = new Element('div').addClass('progress');
		progress.inject(body);			
		var preload = new Element('div').addClass('preloader');
		preload.inject(container,'top');
		var progress_box = new Element('div').addClass('box');
		progress_box.inject(preload);
		var progress_marker = new Element('div').addClass('marker');
		progress_marker.inject(progress_box);

		var update_progress = function(){
			
			/* ugly onload workaround for ie! setting the global force_upload_stop by the onload attribute in the iframe... */
			
			if (force_upload_stop && Browser.ie) {
				force_upload_stop = false;
				clearInterval(timer);
				progress.destroy();
				preload.destroy();			
				// Reloading Main Form: Updating States of some Fields
				if (loadnew) {
					if (loadnew == "Latest") {
						// Tree Reload Callback
						rfgui.currentNode.parent.reload(true);
						rfgui._ajaxtransport('/Context/Latest',$('context'));	
						rfgui._ajaxtransport('/Form/Latest',$('mainform'));								
					}
					if (loadnew == "Previous" || loadnew == "Next") rfgui.currentNode = rfgui.currentNode.selectSibling(loadnew);
				}
				else rfgui._ajaxtransport('/Form/'+rfgui.contribid,'mainform');
			}
			var jsonRequest = new Request.JSON({url: rfstring_SITEPREFIX + '/Ajax/Progress/'+$('progress_key').value, onSuccess: function(w){
				progress_marker.setStyle('width',w);
			}}).send();
		};
		var timer = update_progress.periodical(1000, this); //adds the number of seconds at the Site.

		/* iframe onload is not fired by ie... therefore the workaround above */

		$(form.target).onload = function() {
			clearInterval(timer);
			progress.destroy();
			preload.destroy();			
			// Reloading Main Form: Updating States of some Fields
			if (loadnew) {
				if (loadnew == "Latest") {
					// Tree Reload Callback
					rfgui.currentNode.parent.reload(true);
					rfgui._ajaxtransport('/Context/Latest',$('context'));	
					rfgui._ajaxtransport('/Form/Latest',$('mainform'));								
				}
				if (loadnew == "Previous" || loadnew == "Next") {
					rfgui.currentNode = rfgui.currentNode.selectSibling(loadnew);
//					rfgui._ajaxtransport('/Form/'+loadnew+'/'+rfgui.contribid,$('mainform'));
				}
			}
			else rfgui._ajaxtransport('/Form/'+rfgui.contribid,'mainform');
		}.bind(this);
		
		form.submit();	
	},
	
	/*
	New Contribution Callback - triggered by Add Contribution Button on Click
	*/
	cloneData: function(button) {
		rfgui.rfAjax(button,rfstring_IMPORTQUESTION,'/Ajax/Forms/Clonedata');
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.contribid.value) {
					// Send New Contribution Info and reload tree.
					var callback = function() {
						this._ajaxtransport('/Form/'+rfgui.contribid,'mainform');
					}.bind(rfgui);
					rfgui._ajaxtransport('/Clonedata/'+rfgui.contribid+'/'+e.contribid.value,'mainform',false,false,false,callback);
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGCLONE);
				}
			}
		});		
	},	

	/*
	New Contribution Callback - triggered by Add Contribution Button on Click
	*/
	changeTemplate: function(button) {
		rfgui.rfAjax(button,rfstring_CHANGETEMPLATEQUESTION,'/Ajax/Forms/Changetemplate');
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.templatename.value) {
					// Send New Contribution Info and reload tree.
					var callback = function() {
						this._ajaxtransport('/Form/'+rfgui.contribid,'mainform');
					}.bind(rfgui);
					rfgui._ajaxtransport('/Changetemplate/'+rfgui.contribid+'/'+e.templatename.value,'mainform',false,false,false,callback);					
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGCLONE);
				}
			}
		});		
	},

	/*
	New Contribution Callback - triggered by Add Contribution Button on Click
	*/
	renameContribution: function(button) {
		rfgui.rfPrompt(button,rfstring_RENAMEQUESTION,rfgui.currentNode.div.text.get('text'));
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.prompt.value) {
					var callback = function() {
						// Tree Reload Callback
						var reload = function() {this.currentNode.rename(e.prompt.value);}.bind(this);
						this._ajaxtransport('/Form/'+this.contribid,'mainform',false,false,false,reload);
					}.bind(rfgui);
					rfgui._ajaxtransport('/Renamecontribution/'+rfgui.contribid,callback,e);
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGNAME);
				}
			}
		});		
	},
	
	/*
	Move Contribution Callback - triggered by Move Contribution Button on Click
	*/
	moveContribution: function(button) {
		rfgui.rfAjax(button,rfstring_MOVEQUESTION,'/Ajax/Forms/Movecontribution');
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				var callback = function() {
					// Reset everything
					this.showIntro();					
					rfgui.currentNode.control.root.load(rfstring_SITEPREFIX + '/Ajax/Tree/Current',true);
				}.bind(this,rfgui);
				rfgui._ajaxtransport('/Movecontribution/'+rfgui.contribid,callback,e);
			}
		}.bind(this));				
	},	
	
	
	
	cloneContribution: function(node) {
		rfgui.rfPrompt($('cloneContrib'),rfstring_CLONEQUESTION,node.div.text.get('text')+rfstring_CLONEADDITION);
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.prompt.value) {
					var callback = function() {
						// Tree Reload Callback
						node.parent.reload(true);
						if (rfgui.intro) {
							this.hideIntro();						
							rfgui._ajaxtransport('/Context/Latest',$('context'));	
						}
						rfgui._ajaxtransport('/Form/Latest',$('mainform'));						
					}.bind(this,rfgui);
					rfgui._ajaxtransport('/Clonecontribution/'+node.data.contribid,callback,e);
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGNAME);
				}
			}
		}.bind(this));			
	},
	
	cloneLatestContribution: function(button) {
		rfgui.rfAjax(button,rfstring_CLONEQUESTION,'/Ajax/Forms/Clonelast');
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				if (e.prompt.value) {
					var callback = function() {
						this.hideIntro();
						rfgui._ajaxtransport('/Form/Latest',$('mainform'));						
						rfgui._ajaxtransport('/Context/Latest',$('context'));	
						// Tree Reload Callback
						// Todo: Opn tree at the right position! this.currentNode.parent.parent.reload();
					}.bind(this);
					rfgui._ajaxtransport('/Clonecontribution/Lastmodified',callback,e);
				}
				else {
					rfgui.rfAlert(this,rfstring_MISSINGNAME);
				}
			}
		}.bind(this));			
	},	
	
	/*
	Opens last modified contribution
	*/
	openLastmodified: function(button) {
		this.hideIntro();
		rfgui._ajaxtransport('/Form/Lastmodified',$('mainform'));						
		rfgui._ajaxtransport('/Context/Lastmodified',$('context'));	
		// Tree Reload Callback
		// Todo: Opn tree at the right position! this.currentNode.parent.parent.reload();									
	},
	
	issueAction: function(node,action) {
		
		
		
		
		switch(action)
		{
 		case 'Cloneissue':
			rfgui.rfPrompt($('cloneIssue'),rfstring_CLONEISSUEQUESTION,node.div.text.get('text')+rfstring_CLONEADDITION);
		  	break;
 		case 'Closeissue':
			rfgui.rfConfirm($('closeIssue'),rfstring_CLOSEISSUEQUESTION);
		  	break;
		case 'Reopenissue':
			rfgui.rfConfirm($('reopenIssue'),rfstring_REOPENISSUEQUESTION);
			break;
		case 'Exportissue':
			rfgui.rfAjax($('exportIssue'),rfstring_ISSUEEXPORTQUESTION,'/Ajax/Export/Complete/'+node.data.editionid,true);
			return;
		}		
		
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				var callback = function() {
					this.showIntro();					
					this.buttons.empty();	
					node.parent.reload(rfgui.intro);
				}.bind(this,node);
				rfgui._ajaxtransport('/'+action+'/'+node.data.editionid,callback,((e!=null&&e.prompt.value)?(e):(false)));
					
			}
		}.bind(this));		
	},	
	
	/* Showing the Book list popup */
	bookchapterissueAction: function(node,action) {
		switch(action)
		{
 		case 'addBook':
			rfgui.rfAjax($(action),rfstring_BOOKEDITOR,'/Ajax/Table/'+action,true);	
		  	break;
 		case 'addChapter':
			rfgui.rfAjax($(action),rfstring_CHAPTEREDITOR,'/Ajax/Table/'+action+'/'+node.data.projectid,true);	
		  	break;
		case 'addIssue':
			rfgui.rfAjax($(action),rfstring_ISSUEEDITOR,'/Ajax/Table/'+action+'/'+node.data.projectid,true);	
			break;
 		case 'sortChapter':
			rfgui.rfAjax($(action),rfstring_SORTCHAPTEREDITOR,'/Ajax/Table/'+action+'/'+node.data.editionid+'/'+node.data.chapterid,true);	
		  	break;
		}
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				var callback = function() {
					this.showIntro();					
					if (action=='addBook') node.control.root.load(rfstring_SITEPREFIX + '/Ajax/Tree/Current',true);
					else node.reload(false);					
				}.bind(this,node);
				rfgui._ajaxtransport('/System/'+action+((node.data.projectid)?('/'+node.data.projectid):('')),'index',e,true,true,callback);
			}
		}.bind(this));			
	},
	
	/*
	Sends the Book-Order Settings Form to the Server (in the System Tab)
	*/
	changeOrder: function(form) {
		if (typeof form == 'string') form = $(form);
		rfgui._ajaxtransport('/System/Storesort',false,form);
	},
	
	/*
	Toggles the visibility of an entry in the system tab
	*/
	toggleSytemtab: function(e) {
		if (e.getNext('.toggle').getStyle('display')=="none") {
			e.getNext('.toggle').setStyle('opacity',1).setStyle('display','block');
		}
		else  e.getNext('.toggle').setStyle('display','none');
	},
	
	/*
	Checks the consistency of the Data: Updating Field Counts/Order in Contributions,
	Deleting orphanized Data
	*/
	checkDataConsistency: function(button) {
		switch(button.id)
		{
 		case 'DataConsistency':
			rfgui.rfConfirm(button,rfstring_CHECKDATACONSISTENCY);
		  	break;
 		case 'CleanOrphans':
			rfgui.rfConfirm(button,rfstring_CLEANORPHANS);
		  	break;
		case 'EmptyTrash':
			rfgui.rfConfirm(button,rfstring_EMPTYTRASH);
			break;
		}		
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				var callback = function(html) {
					rfgui.rfAlert(button,html);
				};
				rfgui._ajaxtransport('/System/'+button.id,'index',false,true,true,callback);
			}
		}.bind(this));		
	},
	

	/* Showing the list popup */
	showListalert: function(button,callback) {
		if (callback == null) callback = false;		
		switch(button.id)
		{
 		case 'modifyUsers':
			var quest = rfstring_USERLISTEDITOR;
 			var command = 'Listusers';
		  	break;
 		case 'modifyRights':
			var quest = rfstring_RIGHTSLISTEDITOR;
			var command = 'Listrights';
		  	break;
		case 'modifyPlugins':
			var quest = rfstring_PLUGINSLISTEDITOR;
			var command = 'Listplugins';
		  	break;	
		case 'modifyFieldpostprocessor':
			var quest = rfstring_FIELDPOSTPROCESSOREDITOR;
			var command = 'Listfieldprocessors';
		  	break;
		case 'modifyBatchs':
			var quest = rfstring_BATCHEDITOR;
			var command = 'Listbatchs';
		  	break;			
		case 'modifyTemplates':
			var quest = rfstring_TEMPLATELISTEDITOR;
			var command = 'Listtemplates';
		  	break;		
		case 'changeTemplateFields':
			if (parseInt(button.options[button.selectedIndex].value)==-1) return;
			var quest = rfstring_TEMPLATEFIELDEDITOR;
			var command = 'Listfields/'+button.options[button.selectedIndex].value;
			break;
		}		
		rfgui.rfAjax(button,quest,'/Ajax/Table/'+command,true);	
		rfgui.addEvent('rfalertclose', function(e,v) {
			if (v) {
				rfgui._ajaxtransport('/System/'+command,'index',e,true,true,(typeof callback==='function')?(callback):(false));
			}
		}.bind(this));			
	},

	
	/*
	Hide Intro Page
	*/
	hideIntro: function() {
		$('intro').setStyle('visibility','hidden');
		$('mainform').setStyle('visibility','visible');
		rfgui.intro = false;	
	},
	
	/*
	Show Intro Page
	*/
	showIntro: function() {
		$('mainform').setStyle('visibility','hidden').empty();
		$('contextbuttons').empty();
		$('preview').empty();		
		$('intro').setStyle('visibility','visible');		
/*		if (!Browser.ie) $('intro').fade('in');		*/
		rfgui.intro = true;				
	}

});


