Element.implement({
	dragCenter: function(vis,nodrag){
		if (nodrag==null) nodrag = false;
		if (!nodrag) {
			if (this.drag==null) this.drag = this.makeDraggable();
		}
		else if (this.drag) {
			this.drag.detach();
			this.drag = null;
		}
		this.setPosition({
			x: (window.getSize().x/2 - this.getSize().x/2), 
			y: (window.getSize().y/2 - this.getSize().y/2)
		});
		if (vis) this.setStyle('visibility', 'visible');
	}
});

var rfGUI = new Class({
    Implements: [Options, Events],
    options: {
        color: '#fff',
        size: {
            width: 100,
            height: 100
        }
    },

	// Constructor: Load Options, Add Popup Layer
    initialize: function(options){
        this.setOptions(options);
		this.c = new Element('div').addClass('rfAlerts');
		this.c.inject($(document.body));	
		this.intro = true;	
		this.contribid = false;		// Current Contribution
		this.currentNode = false;	// Current open node in tree view
		this.toggleStates = [];
		var other_users = function() {this._ajaxtransport('/OtherUsers','other',false,true);}.bind(this);
		var ou_timer = other_users.periodical(10000, this); //other users online check
    },

	// Ajax Core Transporter
	_ajaxtransport: function(__url__,__target__,__source__,__disablepreloader__,__progressoverlay__,__triggerfunction__) {
		if (!__disablepreloader__ || __disablepreloader__ == null) __disablepreloader__ = false;
		if (!__progressoverlay__ || __progressoverlay__ == null) __progressoverlay__ = false;		
		if (!__source__ || __source__ == null) __source__ = false;

		// Additional function triggered at the end of a successful load
		if (!__triggerfunction__ || __triggerfunction__ == null) __triggerfunction__ = false;		

		if (typeof __source__ == 'string' && $(__source__) != null) __source__ = $(__source__);
		if (typeof __target__ == 'string') __target__ = $(__target__);		
		var req = new Request.HTML(
			{
			url: rfstring_SITEPREFIX + '/Ajax' + __url__, 
			method: "post", 
			noCache: true,
			evalScripts: false,
			onRequest: function() {
				if (!__disablepreloader__ && typeof __target__ == 'object') __target__.set('html', '<div class="preloader">&nbsp;</div>');
				else if (__progressoverlay__ && typeof __target__ == 'object'){
					this.progress = new Element('div').addClass('preloader');
					this.progress.inject(__target__);					
				}
			},
			onSuccess: function(responseTree, responseElements, responseHTML, responseJavaScript) {
				if (typeof __target__ == 'object') {
					if (__progressoverlay__) {
						this.progress.destroy();
					}
					else {
						__target__.set('text', '');
						__target__.adopt(responseTree);
						__target__.setStyle('visibility', 'visible');	
					}
				}
				if (typeof __target__ == 'function') {
					__target__();
				}				
				if (typeof __triggerfunction__ == 'function') {
					__triggerfunction__(responseHTML,responseElements);
				}				
				if (responseJavaScript) eval(responseJavaScript);
			},
			onFailure: function() {
				if (typeof __target__ == 'object') __target__.set('text', 'The request failed.');
			}
		});
		req.send(__source__);
	},

	// Init PopUp
	_init: function(e) {
		this.removeEvents('rfalertclose'); 
		this.call = e;
		this.c.empty();		
		this.inputfield = null;
	},

	// Show PopUp
	_show: function(big) {
		if (big==null) big=false;
		if (big) this.c.addClass('alertlarge');
		else this.c.removeClass('alertlarge');
		this.c.dragCenter(false,big);
		this.c.hider = new Element('div').addClass('rfHider');
		this.c.hider.inject(this.c, 'before');
		this.c.hider.set('opacity', 0.8);
//		this.c.hider.fade(.8);		
		this.c.setStyle('visibility', 'visible');
		$$('#rfPopForm input, #rfPopForm select, #rfPopForm textarea').addEvent('click', function(e){this.focus(); e.stopPropagation();});
		$$('#rfPopForm input, #rfPopForm select, #rfPopForm textarea').addEvent('dblclick', function(e){this.focus();this.select(); e.stopPropagation();});		
		$$('#rfPopForm input, #rfPopForm select, #rfPopForm textarea').addEvent('mousedown', function(e){e.stopPropagation();});		
		$$('#rfPopForm select').addEvent('mousedown', function(e){this.focus(); e.stopPropagation();});		
		if (typeof(this.c.getElement('input, textarea, select'))=='object' && this.c.getElement('input, textarea, select') != null) this.c.getElement('input, textarea, select').focus();
	},
	
	// Close PopUp
	_close: function(ev, buttonvalue) {
		ev.preventDefault();
		ev.stopPropagation();		
		this.c.setStyle('visibility', 'hidden');		
		// Prompt & Ajax Form
		var form = (typeof(this.inputfield)=='object' && this.inputfield != null)?this.inputfield:null;
		this.c.empty();
		this.c.hider.dispose();
		if (buttonvalue) this.fireEvent('rfalertclose',[form,this.call]);
	},
	
	// Show Alert
	rfAlert: function(e,string,big) {
		this._init(e);
		this.c.adopt(new Element('div').set('html',string));		
		this.c.adopt(new Element('a', {href: '#', html: rfstring_OK,'class': 'button middle green', events: {click: function(e){this._close(e,false);}.bind(this)}}));		
		this._show(big);
	},
	
	// Confirm Popup
	rfConfirm: function(e,string) {
		this._init(e);
		this.c.adopt(new Element('div').set('html',string));		
		this.c.adopt(new Element('a', {href: '#', html: rfstring_CANCEL,'class': 'button left small grey', events: {click: function(e){this._close(e,false);}.bind(this)}}));		
		this.c.adopt(new Element('a', {href: '#', html: rfstring_OK,'class': 'button right green', events: {click: function(e){this._close(e,true);}.bind(this)}}));		
		this._show();
	},

	// Prompt Popup	
	rfPrompt: function(e,string,def) {
		if (def==null) def = false;
		this._init(e);
		this.c.adopt(new Element('div').set('html',string));		
		this.inputfield = new Element('form', {
			events: {
					submit: function(e){e.stop(); this._close(e,true); return false;}.bind(this)
			}
		}).set('id','rfPopForm').set('method','POST');
		this.inputfield.inject(this.c);		
		this.promptfield = new Element('input',{'class':'prompt',id: 'prompt', name: 'prompt'});
		if (def) this.promptfield.set('value',def);
		this.promptfield.inject(this.inputfield);		
		this.c.adopt(new Element('a', {href: '#', html: rfstring_CANCEL,'class': 'button left small grey', events: {click: function(e){this._close(e,false);}.bind(this)}}));		
		this.c.adopt(new Element('a', {href: '#', html: rfstring_OK,'class': 'button right green', events: {click: function(e){this._close(e,true);}.bind(this)}}));		
		this._show();
	},

	// Load a form into the popup and pass it
	rfAjax: function(e,string,formurl,big) {
		this._init(e);
		this.c.adopt(new Element('div').set('html',string));		
		this.inputfield = new Element('form', {
			events: {
					submit: function(e){e.stop(); this._close(e,true); return false;}.bind(this)
			}
		}).set('id','rfPopForm').set('method','POST').set('load', {
		onSuccess: function(e){
			this._show(big);		
		}.bind(this)
		}).load(rfstring_SITEPREFIX + formurl);
		this.inputfield.inject(this.c);		
		this.inputfield.addEvent('click', function(){this.focus();});
		this.c.adopt(new Element('a', {href: '#', html: rfstring_CANCEL,'class': 'button left small grey', events: {click: function(e){this._close(e,false);}.bind(this)}}));		
		this.c.adopt(new Element('a', {href: '#', html: rfstring_OK,'class': 'button right green', events: {click: function(e){this._close(e,true);}.bind(this)}}));		
//		this._show(big);		
	},
	
	
	/* 
	 Login Submit Action 
	*/
	
	rfLogin: function(form) {
		var send = true;
		form.getElements('input').each(function(el){
		    if (el.value == "") {
		        el.focus();
				el.setStyle('border', '1px solid #000');
		        send = false;
		    }
		});
		if (send) {
			$('pass').value = $('pass').value.toMD5();
			form.submit();
		}
       	return send;
	},
	
	/* 
	Main Form: Add to Length Calculation
	maxline can be an array if it is a textfield-array
	*/
	addFieldLengthCalc: function(field, editortype, influence, maxlength, maxline, multi) {
		if (typeof($(field))=='object' && $(field) != null) var f = $(field);
		else var f = new Element(field,{id:field});
		f.influence = influence;
		f.maxlength = maxlength;
		f.maxline = maxline;		
		f.multi = (multi>1?multi:false);
		f.grow = 0;	// 1: Text added -1: Text substracted
		f.totallength = [];	// used for multi fields and backup purposes	
		f.editortype = editortype;	
		f.init = false;
		f.state = $('status_' + f.id);
		this.textfields.push(f);
	},

	/* 
	Main Form: Reset the registered fields
	*/
	resetFieldLengthCalc: function() {
		this.textfields = [];
	},
	
	/*
	Enable Toggling of Fields, load default state
	*/
	fieldToggling: function(elm) {
		var vals = elm.id.split("-");
		if (this.toggleStates[vals[1]]==null) this.toggleStates[vals[1]]=false;
		if (this.toggleStates[vals[1]]) {
			elm.getParent('div').setStyles({
				height: '20px',
				overflow: 'hidden'
			});
			elm.setStyle('background-image', 'url(img/fiel_toggle_off.png)');
			elm.getNext('div').setStyle('display','none');				
		}
		else {
			this.toggleStates[vals[1]]=false;
		}

		elm.addEvent('click', function(e) {
			var vals = e.id.split("-");
			if (!this.toggleStates[vals[1]]||this.toggleStates[vals[1]]==null) {
				this.toggleStates[parseInt(vals[1])]=true;				
				e.getParent('div').setStyles({
					height: '20px',
					overflow: 'hidden'
				});
				e.setStyle('background-image', 'url(img/fiel_toggle_off.png)');				
				e.getNext('div').setStyle('display','none');				
			}
			else {
				this.toggleStates[parseInt(vals[1])]=false;
				e.getParent('div').setStyles({
					height: 'auto',
					overflow: 'visible'
				});				
				e.setStyle('background-image', 'url(img/fiel_toggle_on.png)');				
				e.getNext('div').setStyle('display','block');				
			}
		}.bind(this,elm));
	},
	
	/* 
	Main Form: Start the length calculation for all registered fields
	- Create the init values
	- Init the Status Fields
	- Add the events (also for rtf editor fields)
	- change the background colors if limit exceeds
	*/
	startFieldLengthCalc: function() {
		this.textfields.each(function(e) {
			if (e.multi>1) {
				for (var i=0; i < e.multi; i++) {
					var m = $(e.id + "_" + i);
					m.addEvent('keyup', function(e,i) {
						this._textarea_key_callback(e,i);
					}.bind(this,[e,i]));
					m.fireEvent('keyup');
				};

			}
			else {
				switch(e.editortype)
				{
				case "TEXTAREA":
					e.addEvent('keyup', function(e) {
						this._textarea_key_callback(e);
					}.bind(this,e));
					e.fireEvent('keyup');
				  	break;
				case "ckeditor":
				 	CKEDITOR.instances[e.id].on("instanceReady", function(ed) 
				 	{
						CKEDITOR.instances[e.id].ready = true;
						ed.editor.document.on('keyup', function(ev) {this._ckedit_key_callback(ed.editor)}, this, ed);
						this._ckedit_key_callback(ed.editor);
						
						ed.editor.on('doubleclick', function(e) {
						   var elem = e.data.element;
						   if (elem.hasClass("correx")) {
								this.previewAjax('/Savecorrex',"word="+elem.getText());
								elem.removeClass("correx");
						   }
						}, this, ed);
						
						
					}, this);

				  break;
				default:
					alert("Editortype not implemented yet!");
					break;
				}			
			}			
		}, this);
	},

	/* 
	Textarea callback for single fields and field-arrays
	e: textarea-element (bzw. container-element for arrays)
	subId: array id for array elements (empty if single mode)
	*/
	_textarea_key_callback: function(e, subId) {
		var o, subId, subField, multi;
				 
		if (typeof(subId)=="undefined") {
			subId = 0;
			subField = e;
			multi = false;
		}
		else {
			subField = $(e.id+"_"+subId);
			multi = true;
		}
		if (subField==null) return;
		e.grow = subField.value.length-e.totallength[subId];

		e.totallength[subId] = subField.value.length;
		if (multi) e.totallengthSum = e.totallength.sum();
		else  e.totallengthSum = e.totallength[0];
		
		e.state.set('text',rfstring_LENGTH + ": " + e.totallengthSum + " " + rfstring_MAXLENGTH + ": " + e.maxlength);
		if (multi) {
			if (e.totallengthSum>e.maxlength) o = true;	else o = false;
			for (var _i=0; _i < e.multi; _i++) {
				var _m = $(e.id + "_" + _i);
				_m.setStyle('background', (o||(_m.getScrollSize().y>_m.getSize().y && e.maxline[_i]>0))?'#f15922':'#FFF');
			}
		}
		else {
			if (e.totallengthSum>e.maxlength || (e.getScrollSize().y>e.getSize().y && e.maxline>0)) o = true;
			else o = false;
			e.setStyle('background', o?'#f15922':'#FFF');			
		}
		/* Recursively reduce other fields only if content changed */		
		if (e.grow || !e.init) this._reduce_max_limits(e.id);
	},

	/*
	key up callback function for ckedit
	*/
	
	_ckedit_key_callback: function(editor) {
		var o;
		var l = editor.document.getBody().getText().length;
		$(editor.name).grow = l-$(editor.name).totallength[0];
	
		$(editor.name).totallength[0] = l;
		$(editor.name).state.set('text',rfstring_LENGTH + ": " + l + " " + rfstring_MAXLENGTH + ": " + $(editor.name).maxlength);	
		if (l>$(editor.name).maxlength|| (editor.document.getBody().getSize('height',true)>editor.config.height && $(editor.name).maxline)) o = true;
		else o = false;
		editor.document.getBody().setStyle('background', o?'#f15922':'#FFF');		
		/* Recursively reduce other fields only if content changed */
		if ($(editor.name).grow || !$(editor.name).init) this._reduce_max_limits(editor.name);
	},
	
	/*
	Recursively reduce Text Limits of related text fields
	*/
	
	_reduce_max_limits: function(callerId) {
		this.textfields.each(function(e) {
			if (e.id == callerId) {
				e.influence.each(function(i){
					var target = $(i[0].toString());
					if (this.id != i[0].toString() && typeof(target) == "object" && target != null && i[1] != 0) {					
						if (!this.init) {
							target.maxlength -= this.totallength.sum()*i[1];
							this.init = true;
						}
						if (this.grow) {
							target.maxlength -= this.grow*i[1];
						}
						if ($('status_' + i[0].toString()) != null) $('status_' + i[0].toString()).set('text',rfstring_LENGTH + ": " + target.totallength.sum() + " " + rfstring_MAXLENGTH + ": " + target.maxlength);	

						/* update the status of other fields */
						switch(target.editortype)
						{
						case "TEXTAREA":
							target.fireEvent('keyup');
						  	break;
						case "ckeditor":						
						 	if (CKEDITOR.instances[i[0].toString()].ready) {
								var editor = CKEDITOR.instances[i[0].toString()];
								var l = editor.document.getBody().getText().length;								
								if (l>$(target.id).maxlength) o = true;
								else o = false;
								editor.document.getBody().setStyle('background', o?'#f15922':'#FFF');
							}
							break;
						}
					}
				},e);
			}
		},this);
	},
	
	/*
	Pasting from History Drop Down into Editor
	*/
	pasteHistory: function(source, field, multi, editortype) {
		if (multi>1) {
			var _val = decodeURIComponent(source.options[source.selectedIndex].value).split("<::::::>");
			for (var c = 0; c < _val.length; c++) $(field + "_" + c).value = _val[c];
		}
		else {
			switch(editortype)
			{
			case "TEXTAREA":
				$(field).value = decodeURIComponent(source.options[source.selectedIndex].value);
			  	break;
			case "ckeditor":
				CKEDITOR.instances[field].setData(decodeURIComponent(source.options[source.selectedIndex].value), function() {this.checkDirty();});
				CKEDITOR.instances[field].fire('key');
			  break;
			default:
				alert("Editortype not implemented yet!");
				break;
			}			
		}
	},
	
	/* 
	Load HELP (ajax) into Help Div
	*/
	contextHelp: function(f_id) {
		this.previewAjax('/Showinfo','level=field&fieldid='+f_id);
	},


	_tableInsertRow: function(tbl) {
		newItem = tbl.backup.clone();
		var n = newItem.inject(tbl);
		tbl.table.addItems(n);
		tbl.table.fireEvent('complete',false);
		this._tableUpdateEvents(tbl);	
	},


	/*
	Table Editor: update events when adding new fields.
	*/
	_tableUpdateEvents: function(tbl) {
		$$('#' + tbl.id + ' a.t_add').each(function(e,index){
			e.removeEvents('click');
			e.addEvent('click', function(e){
				var clone = "";
				if (tbl.insert_html) clone = new Element('li', {'style':'white-space: nowrap',html:tbl.insert_html});
				else clone = e.getParent('li').clone();
				var n = clone.inject(e.getParent('li'),'after');
				var id = n.getChildren('input[type=hidden]').get('value');
				n.getChildren('input[type=hidden]').set('value','added');				
				new Element('input', {type: 'hidden', value:id}).inject(n);				
				
				tbl.table.addItems(n);
				tbl.table.fireEvent('complete',e);		
				this._tableUpdateEvents(tbl);				
			}.bind(this,e));		
		}, this);
		$$('#' + tbl.id + ' a.t_del').each(function(e,index){
			e.removeEvents('click');
			e.addEvent('click', function(e){


		//		if(tbl.table.elements.length>1) {
					tbl.table.removeItems(e.getParent('li')).destroy();			
		//		}
				
				/* Exception: if no row left, insert one...*/
				if(tbl.table.elements.length==0) this._tableInsertRow(tbl);
				tbl.table.fireEvent('complete',false);		

				
				
			}.bind(this,e));
		}, this);
		tbl.getElements('textarea, input').each(function(e,index){
			if (e.hasClass('has_date_picker')&&!e.picker) e.picker = new Picker.Date(e, {
				timePicker: true,
				positionOffset: {x: 0, y: 0},
				timeWheelStep: 5,
/*				format: "%d/%m/%Y %H:%M",*/
				pickerClass: 'datepicker_dashboard',
				useFadeInOut: !Browser.ie,
				onSelect: function(date){
					tbl.table.fireEvent('complete',this);
			    }
			});
			
			
			e.removeEvents(['keyup','dblclick']);
			e.addEvent('dblclick', function(e){
		        this.select();
			});	
			e.addEvent('keyup', function(e){
				if ((this.getScrollSize().y>this.getSize().y && this.getStyle('overflow')=="hidden") || 
				(this.getProperty('maxchar')>0 && this.value.length > this.getProperty('maxchar'))) o = true;
				else o = false;
				if (!this.oldCol) this.oldCol = this.getStyle('background-color');
				this.setStyle('background-color', o?'#f15922':this.oldCol);
				tbl.table.fireEvent('complete',this);
			});
		});
		tbl.getElements('select').each(function(e,index){
			e.removeEvents('change');
			e.addEvent('change', function(e){
				tbl.table.fireEvent('complete',this);
			});
		});
		tbl.getElements('div.rowToggler').each(function(e,index){
			e.removeEvents('dblclick');
			e.origHeight = e.getStyle('height');
			e.folded=false;
			e.addEvent('dblclick', function(e){
				this.getParent('li').setStyles({
					height: this.folded?(this.origHeight.toInt()+3):'20px',
					overflow: this.folded?'visible':'hidden'
				});
				this.setStyle('height', this.folded?this.origHeight:'17px');
				this.setStyle('background-image', 'url(img/drag_icon'+(this.folded?'':'_fold')+'.png)');
				this.folded = this.folded?false:true;
			}.bind(e));
			e.fireEvent('dblclick',this);
		});

	},


	/*
	Table Editor: Init Table sortable
	tbl: The Table element (id)
	addbutton:  show "add" and "delete" buttons
	serialize: serialize data into rokfor-strings in a hidden input field (following the table)
	copy_delete: if not set: clone the row while inserting, if set: delete the selected elements (css selector)
	copy_empty: if not set: clone the row while inserting, if set: empty the selected elements (css selector)
	
	*/
	initTable: function(tbl,addbutton, serialize, insert_html) {
		if (typeof(tbl)!="object") tbl = $(tbl);
		if (addbutton==null) addbutton=false;
		if (serialize==null) serialize=true;
		if (insert_html==null) insert_html=false;
		else tbl.insert_html = insert_html;

		tbl.table = new Sortables(tbl, {
			dragOptions: {container: tbl.getParent('div')},
			handle: 'div',
			clone: false,
			revert: false,
			opacity: 0.5,
			constrain: true,
			onComplete: function(e){
				if (serialize) {
					tbl.getNext('input').set('value',this.serialize(false, function(element, index){
						var row = element.getElements('textarea, select, input');
						var arr = Array();
						row.each(function(e){
							if (e.getAttribute('multiple')) arr.push(e.getSelected().get('value').join('<:>'));
							else arr.push(e.get('value').replace(/<;;;;;;>/g,"&lt;;;;;;;&gt;").replace(/<::::::>/g,"&lt;::::::&gt;"));	// Exception if somebody actually writes a delimiter in a field.
						});																												// Can sometimes happen in code callback fields
						return (arr.join('<::::::>'));
					}).join('<;;;;;;>'));
					if (tbl.getNext('input').get('value')=='<;;;;;;>'||!tbl.table.elements.length) tbl.getNext('input').set('value','');
				}
			}
		});

		this._tableUpdateEvents(tbl);	

		if (tbl.insert_html) {
			tbl.backup = new Element('li', {'style':'white-space: nowrap',html:tbl.insert_html});
		}
		else {
			tbl.backup = tbl.getLast('li').clone();
			tbl.backup.getChildren('input[type=hidden]').set('value','added');
		}

//		var e = addbutton||tbl.getLast('p').getLast('a');

		if (typeof(addbutton)=="object" && addbutton != null) {
			addbutton.addEvent('click', function(e){
				this._tableInsertRow(tbl);
			}.bind(this,addbutton));	
		}
		tbl.table.fireEvent('complete',addbutton);
		
	},

	/*
	Set Preview Context by Ajax Call
	*/
	previewAjax: function(url,nodeinfo) {
		$('preview').setStyle('opacity',0);
		var callback = function() {$('preview').setStyle('opacity',1)};
		this._ajaxtransport(url,'preview',typeof nodeinfo == "object"?Object.toQueryString(nodeinfo):nodeinfo,false,false,callback);
	},

	/*
	Image Editor: Preview Entry
	*/
	previewImage: function(url) {
		$('preview').setStyle('opacity',0);
		$('preview').set('html','<img src="'+url+'">').setStyle('opacity',1);
	},

	/*
	Filepicker: Preview Video
	*/
	previewVideo: function(e) {
		// Trigger a conversion and Fill in the result in the preview...
		var jsonRequest = new Request.JSON({url: rfstring_SITEPREFIX + '/Ajax/Videoconversion/'+e, onSuccess: function(data){
			// Create a video tag
			var vid = new Element('video', {id: 'videopreview'});
			vid.set('autoplay','true');				
			vid.src = data.file;			
			if (data.mesg=="file_ok") {
				vid.set('controls','controls');
 				vid.inject($('preview').empty());
			}
			else {
				$('preview').set('html','<div id="info"><p>'+data.mesg+'</p></div>');
 				vid.inject($('preview'));
			}
			$('preview').setStyle('opacity',1);				
		}}).send();
	},

	/*
	Filepicker: Preview Gif Animation
	*/
	previewAnimated: function(e) {
		var postdata = "";
		if (typeof  e == 'string')	postdata = 'movieFile=' + e;
		else postdata = 'movieFile=' + e.options[e.selectedIndex].value;
		this.previewAjax('/Animatedpreview',postdata);
	},
	
	/*
	Send custom data for a field, e.g. TimeMatrix or TargetMatrix
	*/
	customAction: function(field, data) {
		var postdata = field.name + '=' + data;
		this._ajaxtransport('/Form/'+this.contribid,$('mainform'),postdata);
	},
	
	
	addTopic: function(cont,input, translation_table) {
		if (translation_table==null) translation_table=false;
		var elm = $(cont);
		var gui = this;
		var updateForm = function() {
			var result = elm.getElement('.cloudContainer').getChildren().get('text');
			var classes = elm.getElement('.cloudContainer').getChildren().get('class');			
			var ids = elm.getElement('.cloudContainer').getChildren().get('id');						
			var coords = elm.getElement('.cloudContainer').getChildren().getPosition(elm.getElement('.cloudContainer'));
			var threeDee = elm.getElement('.threeDee').get('value').toInt();
			var saveTxt = "";
			for (var c = 0; c < result.length; c++)  {				
				if (result[c] && classes[c]=="cloud-drag-handle") {
					// update hash			
					$(ids[c]).retrieve('xHash').set(threeDee, coords[c].x);
					$(ids[c]).retrieve('yHash').set(threeDee, coords[c].y);					
					// create string from hashes
					coordStr = gui.createVals($(ids[c]).retrieve('xHash'),$(ids[c]).retrieve('yHash'));					
					// save in string
					saveTxt += (translation_table?translation_table[0][result[c]]:result[c])+'<::::::>'+(coordStr[0])+'<::::::>'+(coordStr[1])+'<;;;;;;>';					
				}
			}
			elm.getElement('.saveVal').set('value',saveTxt);
		}
		
		if (input) {
			elm.getElement('.cloudnewTask').addEvent('keydown', function(e) {
				if (e.key != "enter") {return;}
				e.stop();
				var val = elm.getElement('.cloudnewTask').get('value');
				if (!val) return;
				elm.getElement('.cloudnewTask').set('value', '');
				updateForm(createNewEntry(val,10,10));			
			});
		}
		elm.getElement('.cloudrecTask').addEvent('change', function(e) {
			e.stop();
			var val = elm.getElement('.cloudrecTask').get('value');
			if (!val) return;
			var vals = val.split("<;;;;;;>");
			for (var c = 0; c < vals.length; c++) if (vals[c] != "") {
				newVal = vals[c].split("<::::::>");
				createNewEntry((translation_table?translation_table[1][newVal[0]]:newVal[0]),newVal[1]?newVal[1]:10,newVal[2]?newVal[2]:10);	
			} 
			updateForm();    
		});	

		// Fügt einen neuen Wert in die Cloud
		//

		var createNewEntry = function(val,x,y) {
			var div = new Element('div', {id: 'item-'+cont+'-'+val, text:val, 'class':'cloud-drag-handle'});
			var threeDee = elm.getElement('.threeDee').get('value').toInt();
			elm.getElement('.cloudContainer').adopt(div);

			// make the coordinate hash, save actual value at actual 3d-position
			$(div).store('xHash', new Hash()); 
			$(div).store('yHash', new Hash()); 
			$(div).retrieve('xHash').set(threeDee, Math.round(x));
			$(div).retrieve('yHash').set(threeDee, Math.round(y));			

			$(div).setStyles({
			    left: Math.round(x) + elm.getElement('.cloudContainer').getPosition(elm.getElement('.cloudContainer').getOffsetParent()).x + elm.getElement('.cloudContainer').getOffsetParent().getScroll().x,
			    top:  Math.round(y) + elm.getElement('.cloudContainer').getPosition(elm.getElement('.cloudContainer').getOffsetParent()).y + elm.getElement('.cloudContainer').getOffsetParent().getScroll().y,
				visibility: 'visible'
			});
			$(div).makeDraggable(
				{
				includeMargins: true,
				container: elm.getElement('.cloudContainer'),	
				droppables: elm.getElement('.cloud-del-handle'),	
				onStart: function(){
					updateForm();
			    },
				onDrag: function() {
					$(div).setStyles({
					    color: '#CCC'
					});
				},
			    onComplete: function(elso){
					$(div).setStyles({
					    color: '#000'
					});				
					updateForm();
			    },
				onDrop: function(el,over) {
					if (over) {
						over.setStyle('background', "url('" + rfstring_SITEPREFIX + "/img/so_tr.png') no-repeat");											
						el.dispose();
						updateForm();
					}
				},
				onEnter: function(el,over) {
					over.setStyle('background', "url('" + rfstring_SITEPREFIX + "/img/so_tr_over.png') no-repeat");
				},
				onLeave: function(el,over) {
					over.setStyle('background', "url('" + rfstring_SITEPREFIX + "/img/so_tr.png') no-repeat");					
				}
			});
			return(div);
		}

		// Create List from Saved Values:
		var vals = (elm.getElement('.saveVal').get('value')).split("<;;;;;;>");
		for (var c = 0; c < vals.length; c++) if (vals[c] != "") {
			newVal = vals[c].split("<::::::>");
			var splitData = this.splitVals (newVal[1],newVal[2],elm);
			newElement = createNewEntry((translation_table?translation_table[1][newVal[0]]:newVal[0]),(splitData[0].get(0))?(splitData[0].get(0)):(0),(splitData[1].get(0))?(splitData[1].get(0)):(0));	
			$(newElement).store('xHash', splitData[0]);	
			$(newElement).store('yHash', splitData[1]);			
		}
	},

	// Combine Hashes with X and Y coordinates and the according 3d-Depth into a serialized string
	createVals: function(x,y) {
		var xStr, yStr;
		xStr = yStr = "";
		x.each(function(value, key){xStr += key+'_'+value+'<>';});
		y.each(function(value, key){yStr += key+'_'+value+'<>';});	
		return Array(xStr,yStr);
	},

	// Split Coordinates-Hashes with 3d-Depth into array of hashes
	splitVals: function(x,y,elm) {
		xA = x.split("<>");	
		yA = y.split("<>");		
		var retX = new Hash();
		var retY = new Hash();	
		xA.each(function(item,ind){
			if (item.split("_")[0]!="" && item.split("_")[1]!="") this.set(item.split("_")[0], item.split("_")[1]);	
		},retX);
		yA.each(function(item,ind){
			if (item.split("_")[0]!="" && item.split("_")[1]!="") this.set(item.split("_")[0], item.split("_")[1]);	
		},retY);
		return Array(retX,retY);
	},

	// interpolate value

	nearestNeighbour: function(x,y,elm) {
		var threeDee = parseInt($(elm).getElement('.threeDee').get('value'));
		var x_bigger = false;
		var x_smaller = false;
		var y_bigger = false;
		var y_smaller = false;	
		if (x.has(threeDee)) xRet=x.get(threeDee); 
		else {
			x_bigger = this.getBiggerValue(x,threeDee);
			x_smaller = this.getSmallerValue(x,threeDee);
			if (x_smaller[0]===false) x_smaller = x_bigger;		
			if (x_bigger[0]===false) x_bigger = x_smaller;				
			xRet = x_smaller[0] + (((x_bigger[1]-x_smaller[1]!=0)?((x_bigger[0]-x_smaller[0]) / (x_bigger[1]-x_smaller[1])):(0)) *  (threeDee-x_smaller[1]));
		};
		if (y.has(threeDee)) yRet=y.get(threeDee); 
		else {
			y_bigger = this.getBiggerValue(y,threeDee);
			y_smaller = this.getSmallerValue(y,threeDee);		
			if (y_smaller[0]===false) y_smaller = y_bigger;		
			if (y_bigger[0]===false) y_bigger = y_smaller;				
			yRet = y_smaller[0] + (((y_bigger[1]-y_smaller[1]!=0)?((y_bigger[0]-y_smaller[0]) / (y_bigger[1]-y_smaller[1])):(0)) *  (threeDee-y_smaller[1]));
		}
		return Array(xRet,yRet);
	},

	getBiggerValue: function(hash,actual_key) {
		var ret_value = 0; 
		ret_key = 0; 
		var diff = 101; 

		hash.each(function(value, key) {
			if (key>actual_key && (Math.abs(key-actual_key)<diff)) {
				diff = Math.abs(key-actual_key);
				ret_value = value;
				ret_key = key;					
			}
		});
		return Array(ret_value.toInt(),ret_key.toInt());
	},

	getSmallerValue: function(hash,actual_key) {
		var ret_value = 0; ret_key = 0; var diff = 101;
		var state = "";
		hash.each(function(value, key) {
			if (key<actual_key && (Math.abs(key-actual_key)<diff)) {
				diff = Math.abs(key-actual_key);
				ret_value = value;
				ret_key = key;			
			}
		});
		return Array(ret_value.toInt(),ret_key.toInt());
	},
	
	add3dSlider: function(cont,target) {
		var el = $(cont), tg = $(target), saveVal = tg.getElement('.threeDee');
		new Slider(el, el.getElement('.knob'), {
			steps: 100, 
			range: [0],
			onChange: function(value){
				saveVal.set('value',value);
				this.cloudUpdate3d(target);
			}.bind(this)
		});
	},

	cloudUpdate3d: function(target) {
		var elm = $(target)

		// Cycle thru all Element in Cloud
		var result = elm.getElement('.cloudContainer').getChildren().get('text');
		var classes = elm.getElement('.cloudContainer').getChildren().get('class');	
		var coords = elm.getElement('.cloudContainer').getChildren().getPosition(elm.getElement('.cloudContainer'));			
		var ids = elm.getElement('.cloudContainer').getChildren().get('id');						
		for (var c = 0; c < result.length; c++)  {
			if (result[c] && classes[c]=="cloud-drag-handle") {
				// Interpolate Coordinates
				coordInterpol = this.nearestNeighbour ($(ids[c]).retrieve('xHash'),$(ids[c]).retrieve('yHash'),target);
				// Set the Values
				$(ids[c]).setStyles({
						// Set the Position
				    left: Math.round(coordInterpol[0]) + elm.getElement('.cloudContainer').getPosition().x,
				    top:  Math.round(coordInterpol[1]) + elm.getElement('.cloudContainer').getPosition().y,
					visibility: 'visible'
				});		
			}
		}
	},

	addSort: function(cont, input, translation_table) {
		if (translation_table==null) translation_table=false;
		var elm_sort = $(cont);
		elm_sort.getElement('.recTask').selectedIndex = -1;
		var updateForm = function(el) {
			var result = "";
			if (translation_table) {
				var arr = [];
				el.getParent().getChildren().each(function(e){
					arr.push(translation_table[0][e.get('text')]);
				});
				result = arr.join("<;;;;;;>");
			}
			else {
				result = el.getParent().getChildren().get('text').join("<;;;;;;>");
			}
			elm_sort.getElement('.saveVal').set('value',result);
		}
		var sort = new Sortables(elm_sort.getElement('.todo'), {
			handle: elm_sort.getElement('.drag-handle'),
			constrain: true,
			clone: true,
			onComplete: updateForm
		});
		if (input) {
			elm_sort.getElement('.newTask').addEvent('keydown', function(e) {
				if (e.key != "enter") {return;}
				e.stop();
				var val = this.get('value');
				if (!val) return;
				this.set('value', '');
				updateForm(createNewEntry(val));			
			});
		}

		elm_sort.getElement('.recTask').addEvent('change', function(e) {
			e.stop();
			var val = this.get('value');
			if (!val) return;
			if (translation_table) {
				val = translation_table[1][val];
			}
			this.selectedIndex = -1;
			updateForm(createNewEntry(val));
		});	

		var createNewEntry = function(val) {
			var li = new Element('li', {id: 'item-'+cont+'-'+val, text:val});
			var handle = new Element('div', {id:'handle-'+cont+'-'+val, 'class':'drag-handle'});
			var del = new Element('div', {id:'delete-'+cont+'-'+val, 'class':'del-handle'});			
			del.addEvent('click', function(e) {
				e.stop();
				var list = this.getParent().getParent();
				sort.removeItems(this.getParent()).destroy();
				

				var result = "";
				if (translation_table) {
					var arr = [];
					list.getChildren().each(function(e){
						arr.push(translation_table[0][e.get('text')]);
					});
					result = arr.join("<;;;;;;>");
				}
				else {
					result = list.getChildren().get('text').join("<;;;;;;>");
				}
				elm_sort.getElement('.saveVal').set('value',result);								
//				elm_sort.getElement('.saveVal').set('value',list.getChildren().get('text').join("<;;;;;;>"));
			});
			handle.inject(li, 'top');
			del.inject(li, 'top');			
			elm_sort.getElement('.sortlist').adopt(li);
			sort.addItems(li);
			return(li);
		}

		// Create List from Saved Values:
		var vals = (elm_sort.getElement('.saveVal').get('value')).split("<;;;;;;>");
		for (var c = 0; c < vals.length; c++) if (vals[c] != "") {
			createNewEntry(translation_table?translation_table[1][vals[c]]:vals[c]);	
		}
	},
	
	addSlider: function(cont) {
		var el = $(cont), saveVal = el.getElement('.saveVal');
		var initval = saveVal.get('value');
		new Slider(el, el.getElement('.knob'), {
			steps: 100, 
			range: [0],
			onChange: function(value){saveVal.set('value',value);}
		}).set(initval);
	},	

	addSquare: function(cont) {
		var el = $(cont).getElement('.knob');
		var vals = (el.getElement('.saveVal').get('value')).split("<::::::>");

		$(el).setStyle('visibility', 'visible');
		$(el).position({
			relativeTo: cont,
			x: parseFloat(vals[0])*3.32,
	    	y: parseFloat(vals[1])*3.12
		});


		el.makeDraggable({
			container: cont,
			onComplete: function(elm){
				var coords = elm.getPosition(cont);
				elm.getElement('.saveVal').set('value',(100/332*coords.x)+'<::::::>'+(100/312*coords.y));
		    }
		});
	}	
	
});


