/*


	TODOS:

	- Dynamisches Hinzüfügen von Dimensionen.
	- Löschen von Dimensionen.
	- Laden eines leeren Ausgangszustandes.


*/

(function(){

var Cloud = new Class({
	Implements: [Events, Options],

	options: {
		legendleft: 'Differenz: Gleichförmig / Kontinuierlich',
		legendright: 'Differenz: Kontrastreich / Unterschiedlich',		
		legendtop: 'Absolutes Mass: Selten / Wenig',
		legendbottom: 'Absolutes Mass: Oft / Viel',	
		verboseid: false,
		dataid: 'data'
	},
	
	/*
		Constructor
	*/
	
	initialize: function(container, options) {  

		// Options
		this.setOptions(options);		

		// Globals
		this._drawstate = false;		// Linienstatus: true - es wird eine Linie gezeichnet false - Linienzeichnung uaus
		this._cursorpos = Array();		// Cursor Position (x,y)
		this._currentcanvas = false;	// Current Active Canvas to draw on $()-Object
		this._nodestack = Array();
		this._objectstack = {};
		this._datapool = Array();
		this._dataarray = Array();
		this.actualChannel = false;
		this._channels = Array();

		// Grabbing Elements
		this.container = $(container);
		if (this.options.verboseid) this.verb = $(this.options.verboseid);
		else this.verb = new Element('p');
		this.datafield = $(this.options.dataid);

		// Setting up legend
		var bgo = new Element('div', {
			'id': 'move_legend_bg',
			'style': 'display:none'
		});		
		this.container.grab(bgo);
		
		var bgo = new Element('div', {
			'id': 'move_legend',
			'html': '↕vertical<br>↔horizontal',
			'style': 'display:none'
		});		
		this.container.grab(bgo);
		
		this._legend = $('move_legend');	
		this._legend_bg = $('move_legend_bg');	
				
		// Creating Keywords, Objects and Nodes from JSON Data
		this.loadJSON(this.datafield.value);
		
		// Draw the initial setup
		this.setupCanvas();

		// Container Handlers
		this.container.addEvents({
		    'contextmenu': function(event) {						// Cancel Line Drawing 
				event.preventDefault();
				event.stopPropagation();
			}.bind(this),
			'mousemove': function(event) {					// Keeping track of the mouse
				this._cursorpos = Array(event.page.x - this.container.getPosition().x,event.page.y - this.container.getPosition().y);
				if (this._drawstate=="new") this.paintLine();		
			}.bind(this)
		});
				
	},

	/*
		Draw Legends, Initialize Data and Objects
	*/

	setupCanvas: function() {
		// Draw Menu & Legends
		// Draew Channel Selector if more than one channel
		
		if (this._channels.length > 1) {
			var bgm = new Element('div', {'id': 'cloud_menu_bg', 'html': '<p id="link_cont"></p><p>Channels: </p>'});		
			this.container.grab(bgm);		
			for( var d in this._channels ) {	
				if (d<=this._channels.length) new Element('a', {'class': 'changedim' + (this._channels[d]==this.actualChannel?' active':''), 'href': d, 'text': this._channels[d]}).inject($('link_cont'));		
			}
			$$('.changedim').each(function(x){
				x.addEvent('click', function(event) {
					event.preventDefault();
					$$('.changedim').removeClass('active');
					x.addClass('active');
					/* Delete Objects */
					$$('.object1, .object2, .object3, .object4, .object_to, .daggerline').destroy();
					/* Change Channel */
					this.actualChannel = this._channels[x.get('href')];
					/* Redraw */
					this.drawObjects();
				}.bind(this));
			}.bind(this));
		}


		var bgo = new Element('div', {
			'id': 'tp_lg',
			'html': '<p class="top">'+this.options.legendtop+'</p><p class="bottom">'+this.options.legendbottom+'</p>'
		});		
		this.container.grab(bgo);		

		var bgo = new Element('div', {
			'id': 'tp_lg2',
			'html': '<span style="float:right">'+this.options.legendright+'</span>'+this.options.legendleft
		});		
		this.container.grab(bgo);		
		
		this.drawObjects();
		
		this.fireEvent('render', this);			
		
	},
	
	
	drawObjects: function() {
		// Cycle thru the Objects, draw and make them draggable
		for( var k in this._objectstack[this.actualChannel] ) {
			if (k<=this._objectstack[this.actualChannel].length) {
				if (typeof(this._objectstack[this.actualChannel][k]['from'])=="object") 
					this._objectstack[this.actualChannel][k]['fromid'] = this.addObject(this._objectstack[this.actualChannel][k]['from'][0],this._objectstack[this.actualChannel][k]['from'][1],this._objectstack[this.actualChannel][k]['key'],this._objectstack[this.actualChannel][k]['type']);
					$(this._objectstack[this.actualChannel][k]['fromid']).k = k;
				if (typeof(this._objectstack[this.actualChannel][k]['to'])=="object") {
					this._objectstack[this.actualChannel][k]['toid'] = this.addObject(this._objectstack[this.actualChannel][k]['to'][0],this._objectstack[this.actualChannel][k]['to'][1],this._objectstack[this.actualChannel][k]['key']);
					$(this._objectstack[this.actualChannel][k]['fromid']).hasSlave = true;
					// Draw a Linew
					var node = this.drawNode($(this._objectstack[this.actualChannel][k]['fromid']),$(this._objectstack[this.actualChannel][k]['toid']),1)
					$(this._objectstack[this.actualChannel][k]['fromid']).outNodes = $(this._objectstack[this.actualChannel][k]['toid']).inNodes = $(node);
				}
			}
		}		
	},

	/*
		Adds an object to the stage
	*/

	addObject: function(l,t,v,s) {
		_textarray = v.split('|');
		if (s==null) s = false;
		this.verb.set('text',"Add an object");	
		var dt = new Date();
		var rd=Math.floor(Math.random()*1000)
		var id = "obj"+dt.getTime()+rd;			
		var new_e = new Element('div', {
		    'class': 'object'+((s)?(s):('_to')),
			'id' : id,
			'html': _textarray[0],
		    'styles': {
				'left': l,
				'top': t		
		    }
		});
		
		new_e.legendhorizontal = _textarray[1];
		new_e.legendvertical = _textarray[2];
		new_e.inject(this.container);	
		this.initobject(new_e,!(s));	
		
		
		
		new_e.setPosition({x: (this.container.getSize().x-new_e.getSize().x) / 100 * l, y: (this.container.getSize().y-new_e.getSize().y) / 100 * t});
		return id;
	},

	/*
		Drag Methods and Handlers for Objects on Stage
	*/

	initobject: function(elm,slave) {
		// Right Click: Delete word - for slaves only
		if (slave) elm.addEvent('contextmenu', function(event) {
			event.preventDefault();
			if (!this._drawstate) this.deleteKeyword(elm);
		}.bind(this));
		elm.makeDraggable({
			container: this.container,
			onComplete: function(elm){
				this.updateJSON();
				this._legend.set('style', 'visibility:hidden');			
				this._legend_bg.set('style', 'visibility:hidden');				
		    }.bind(this),
			onCancel: function(elm){
				if (!slave && !elm.hasSlave) this.addSlave(elm);
		    }.bind(this),
			onDrag: function(elm) {
				this.updateNodes(elm);
				if (elm.legendvertical && elm.legendhorizontal) {
					var __x = Math.round(100 / (this.container.getSize().x-elm.getSize().x) * elm.getPosition(this.container).x);
					var __y = Math.round(100 / (this.container.getSize().y-elm.getSize().y) * elm.getPosition(this.container).y);
					
					this._legend.set('html','<span style="width:15px; float:left;">↕</span><span style="float:right;">'+__y+'%</span>' + elm.legendvertical + '<br><span style="width:15px; float:left;">↔</span><span style="float:right;">'+__x+'%</span>' + elm.legendhorizontal);
					this._legend.set('style', 'visibility:visible');
					this._legend_bg.set('style', 'visibility:visible');
				}
			}.bind(this)
		});
	},
	
	/*
		JSON Tools - Load
	*/

	loadJSON: function(data) {
		this._dataarray = data.split('<::::::>');
		this._datapool = JSON.decode(this._dataarray[1]);		
		var _obj_length;
		for( var d in this._datapool ) {
				this._channels.push(d);
				if (this._objectstack[d]==null) this._objectstack[d] = Array();
				if (!this.actualChannel) this.actualChannel = d;
				for( var k in this._datapool[d] ) {
					if (k<=this._datapool[d].length) {
						this._datapool[d][k]['from'][0] = this._datapool[d][k]['from'][0];
						this._datapool[d][k]['from'][1] = this._datapool[d][k]['from'][1];
						if (this._datapool[d][k]['to']) {
							this._datapool[d][k]['to'][0] = this._datapool[d][k]['to'][0];
							this._datapool[d][k]['to'][1] = this._datapool[d][k]['to'][1];					
						}
						_obj_length = this._objectstack[d].push(this._datapool[d][k]);			
					}
				}
		}
	},

	/*
		JSON Tools - Save
	*/

	updateJSON: function () {
		// JSONize the Data and save in in the input tag.	Normalize to 100.	

		console.log(this._objectstack);
		
		for( var k in this._objectstack[this.actualChannel] ) {
			if (k<=this._objectstack[this.actualChannel].length) {
				if (this._objectstack[this.actualChannel][k]['to']) {
					this._objectstack[this.actualChannel][k]['to'][0] = Math.round(this._objectstack[this.actualChannel][k]['to'][0]);
					this._objectstack[this.actualChannel][k]['to'][1] = Math.round(this._objectstack[this.actualChannel][k]['to'][1]);				
				}
				this._objectstack[this.actualChannel][k]['from'][0] = Math.round(this._objectstack[this.actualChannel][k]['from'][0]);
				this._objectstack[this.actualChannel][k]['from'][1] = Math.round(this._objectstack[this.actualChannel][k]['from'][1]);
			}
		}

		
		this.datafield.value = this._dataarray[0] + '<::::::>' + JSON.encode(this._objectstack);	
		this.fireEvent('save', this);		
	},
	
	/*
		Adds a slave to an object
	*/
	
	addSlave: function(elm) {
		this._objectstack[this.actualChannel][elm.k]['toid'] = this.addObject(this._objectstack[this.actualChannel][elm.k]['from'][0]+10,this._objectstack[this.actualChannel][elm.k]['from'][1]+10,this._objectstack[this.actualChannel][elm.k]['key']);
		this._objectstack[this.actualChannel][elm.k]['to'] = Array();
		this._objectstack[this.actualChannel][elm.k]['to'][0] = 100 / (this.container.getSize().x-elm.getSize().x) * (elm.getPosition(this.container).x);
		this._objectstack[this.actualChannel][elm.k]['to'][1] = 100 / (this.container.getSize().y-elm.getSize().y) * (elm.getPosition(this.container).y);
		var node = this.drawNode($(this._objectstack[this.actualChannel][elm.k]['fromid']),$(this._objectstack[this.actualChannel][elm.k]['toid']),1)
		$(this._objectstack[this.actualChannel][elm.k]['fromid']).outNodes = $(this._objectstack[this.actualChannel][elm.k]['toid']).inNodes = $(node);	
		elm.hasSlave = true;			
	},

	/*
		delete an Object
	*/

	deleteKeyword: function(elm) {
		// Set Master has Slave to false
		$($(elm.inNodes).from.id).hasSlave = false;
		// Clear from Object Stack
		delete(this._objectstack[this.actualChannel][$($(elm.inNodes).from.id).k]['to']);
		// Clear from OutNodes
		$(this._objectstack[this.actualChannel][$($(elm.inNodes).from.id).k]['fromid']).outNodes = null;
		// Delete Nodes from/to
		if (elm.outNodes) this.deleteNode(elm.outNodes);
		if (elm.inNodes) this.deleteNode(elm.inNodes);
		// Delete from Data
		elm.destroy();
		this.updateJSON();
	},

	/*
		delete a Line
	*/

	deleteNode: function(elm) {
		elm.destroy();
	},

	/*
		Draw a Line
	*/

	drawNode: function(fromelm,toelm) {
		// Draws a node from - to with a certain type
		this.startDraw(fromelm);
		this.paintLine(fromelm,toelm);		 
		var newid = this.finishDraw(toelm);
		return newid;
	},

	/* 
		Update Action - called when Moving an Object
	*/

	updateNodes: function(elm) {
		// Changes the position of an existing Line
		this._drawstate = "update";
		var from = elm;
		var to = false;
		if (elm.outNodes) {
			this._currentcanvas = elm.outNodes;
			this.paintLine(elm,this._currentcanvas.to);
			to = this._currentcanvas.to;
		}
		if (elm.inNodes) {
			this._currentcanvas = elm.inNodes;
			from = this._currentcanvas.from
			to = elm;		
		}
		// Update Coordinates in the Stack

		if (from && to) this.paintLine(from,to);		
		if (to) {
			this._objectstack[this.actualChannel][from.k]['to'][0] = 100 / (this.container.getSize().x-to.getSize().x) * to.getPosition(this.container).x;
			this._objectstack[this.actualChannel][from.k]['to'][1] = 100 / (this.container.getSize().y-to.getSize().y) * to.getPosition(this.container).y;				
		}
		this._objectstack[this.actualChannel][from.k]['from'][0] = 100 / (this.container.getSize().x-from.getSize().x) * from.getPosition(this.container).x;
		this._objectstack[this.actualChannel][from.k]['from'][1] = 100 / (this.container.getSize().y-from.getSize().y) * from.getPosition(this.container).y;				
	
		this._drawstate = false;
	},

	/*
		Paint Line on Canvas
	*/

	paintLine: function(fromelm,toelm) {
		if (!this._drawstate) return;		
		var sz = fromelm.getSize();
		var po = fromelm.getPosition(this.container);
		var tsz = toelm.getSize();
		var tpo = toelm.getPosition(this.container);			
		var right = (tpo.x>po.x)?(tpo.x+tsz.x):(po.x+sz.x);
		var bottom =  (tpo.y>po.y)?(tpo.y+tsz.y):(po.y+sz.y);
		var left = (po.x<tpo.x)?(po.x):(tpo.x);
		var top = (po.y<tpo.y)?(po.y):(tpo.y);
		this._currentcanvas.setStyles({'left': left,'top': top,'display': 'block'});
		this._currentcanvas.width = right-left;
		this._currentcanvas.height = bottom-top;
		this._currentcanvas.context.strokeStyle = "#CCC";		
		this._currentcanvas.context.lineWidth = 20;
		this._currentcanvas.context.beginPath();
		var lfrx = (po.x<tpo.x)?(sz.x/2):(this._currentcanvas.width-sz.x/2);
		var lfry = (po.y<tpo.y)?(sz.y/2):(this._currentcanvas.height-sz.y/2);
		var ltox = (po.x>tpo.x)?(tsz.x/2):(this._currentcanvas.width-tsz.x/2);
		var ltoy = (po.y>tpo.y)?(tsz.y/2):(this._currentcanvas.height-tsz.y/2);
		var dx = ltox - lfrx;
		var dy = ltoy - lfry;
		var d = Math.sqrt((dx*dx)+(dy*dy));
		var fx = dx/d;
		var fy = dy/d;
		for (var i=1; i < dx/fx; i+=5) {
			var sx = lfrx+(i*fx);
			var sy = lfry+(i*fy);
			this._currentcanvas.context.moveTo(sx,sy);
			this._currentcanvas.context.lineTo(lfrx+((i+1)*fx),lfry+((i+1)*fy));
		}; 
		this._currentcanvas.context.stroke();
		this.verb.set('text',"Currently Drawing: "+(right-left)+"/"+(bottom-top));	
	},

	/*
		End Draw Mode
	*/

	finishDraw: function(elm) {
		if (this._drawstate != "new") return false;
		this._drawstate=false;
		this.verb.set('text',"Finish Line");
		this._currentcanvas.to = elm;
		return this._currentcanvas.id;
	},

	/*
		Start Draw Mode
	*/

	startDraw: function(elm) {
		if (this._drawstate!=false) return;
		this.verb.set('text',"Start Line");
		this._drawstate="new";		
		var dt = new Date();
		var rd=Math.floor(Math.random()*1000)		
		var id = "canv_"+dt.getTime()+rd;
		var el = new Element('canvas', {
			'id': id,
			'class': 'daggerline',
		    'styles': {
				'position': 'absolute',
				'display': 'none'
	    	}
		});
		this._currentcanvas = $(el);
		this._currentcanvas.from = elm;
		this._currentcanvas.context = this._currentcanvas.getContext("2d");
		this._currentcanvas.inject(this.container);
	}
});


Element.implement({
	makeCloud: function(options){
		var cloud = new Cloud(this, options);
		return cloud;
	}

});

})();