/*


	target.js

	Loads all Keywords of all dimensions into one area
	While keywords are fixed, a Target per dimension can
	be dragged freely

*/
(function(){

var Target = new Class({
	Implements: [Events, Options],

	options: {
		legendleft: '',
		legendright: '',		
		legendtop: '',
		legendbottom: '',	
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
		this._keywordstack = Array();
		this._targetstack = Array();
		this._dimensionstack = Array();		
		this._datapool = Array();
		this._dataarray = Array();
		
		// Grabbing Elements
		this.container = $(container);
		if (this.options.verboseid) this.verb = $(this.options.verboseid);
		else this.verb = new Element('p');
		this.datafield = $(this.options.dataid);
		
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
			}.bind(this)
		});
	},

	/*
		Draw Legends, Initialize Data and Objects
	*/

	setupCanvas: function() {
		// Draw Menu & Legends

		if (this.options.legendtop && this.options.legendbottom && this.options.legendright && this.options.legendleft) {
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
		}
		
		
		// Cycle thru the dimensions, draw the according keywords onto the canvas
		
		for( var k in this._dimensionstack ) {		
			if (k<=this._dimensionstack.length) {
				if (typeof(this._keywordstack[this._dimensionstack[k]])=="object") {
					Object.each(this._keywordstack[this._dimensionstack[k]], function(e, d){this.addObject(k,d,e,'object')},this);
				}
				if (typeof(this._targetstack[this._dimensionstack[k]])=="object")	{
					this.addObject(k,this._dimensionstack[k],this._targetstack[this._dimensionstack[k]], 'target');
				}
			}
		}
	},

	/*
		Adds an object to the stage
	*/

	addObject: function(k,e,coord,type) {
		var dim = this._dimensionstack[k];
		this.verb.set('text',"Add an "+type);	
		var dt = new Date();
		var rd=Math.floor(Math.random()*1000)
		var id = "obj"+dt.getTime()+rd;	
		var new_e = new Element('div', {
		    'class': type+' dim_'+k,		
			'id' : id,
			'text': e,
		    'styles': {
				'left': coord[0],
				'top': coord[1]		
		    }			
		});
		new_e.inject(this.container);	
		new_e.dim = k;
		/* Init if Target */
		if (type=="target") this.initobject(new_e);	
		return id;
	},

	/*
		Drag Methods and Handlers for Objects on Stage
	*/

	initobject: function(elm) {

		var dim = this._dimensionstack[elm.dim];
		elm.setStyle('background',(this._targetstack[dim][2]==0)?('rgba(204,255,255,0.3)'):('rgba(204,255,255,0.8)'));

		elm.addEvent('contextmenu', function(event) {
			event.preventDefault();
			var dim = this._dimensionstack[elm.dim];
			this._targetstack[dim][2] = (this._targetstack[dim][2])?(0):(1);
			if (this._targetstack[dim][2]==0) elm.setStyle('background','rgba(204,255,255,0.3)');
			if (this._targetstack[dim][2]==1) elm.setStyle('background','rgba(204,255,255,0.8)');
			this.updateJSON();			
			event.stopPropagation();
		}.bind(this));
		
		elm.makeDraggable({
			container: this.container,
			onComplete: function(elm){
				/* Remove Hilite */				
				$$('div .dim_'+elm.dim).removeClass('hilite');
				/* Store Position */
				var dim = this._dimensionstack[elm.dim];					
				this._targetstack[dim][0] = elm.getPosition(this.container).x;		
				this._targetstack[dim][1] = elm.getPosition(this.container).y;				
				this.updateJSON();
		    }.bind(this),
			onCancel: function(elm){
				/* Remove Hilite */
				$$('div .dim_'+elm.dim).removeClass('hilite');
		    }.bind(this),
			onBeforeStart: function(elm) {
				$$('div .dim_'+elm.dim).addClass('hilite');
			}.bind(this)
		});
	},

	/*
		JSON Tools - Load
	*/

	loadJSON: function(data) {
		this._dataarray = data.split('<::::::>');
		this._datapool = JSON.decode(this._dataarray[1]);	
		for( var k in this._datapool ) {
			/* Loading Keywords per Dimension */
			if (!this._keywordstack[k]) this._keywordstack[k] = Array();
			for( var e in this._datapool[k]['Keywords'] ) {	
				this._keywordstack[k][e] = Array();	
				this._keywordstack[k][e][0] = this.container.getSize().x / 100 *this._datapool[k]['Keywords'][e][0];
				this._keywordstack[k][e][1] = this.container.getSize().y / 100 *this._datapool[k]['Keywords'][e][1];
			}
			/* Loading Targets per Dimension */			
			if (!this._targetstack[k]) this._targetstack[k] = Array();			
			this._targetstack[k][e] = Array();	
			this._targetstack[k][0] = this.container.getSize().x / 100 * this._datapool[k]['Target'][0];			
			this._targetstack[k][1] = this.container.getSize().y / 100 * this._datapool[k]['Target'][1];			
			this._targetstack[k][2] = (this._datapool[k]['Target'][2]===undefined)?(1):(this._datapool[k]['Target'][2]);			

			/* Storing Dimensions */
			this._dimensionstack.push(k);
		}
	},

	/*
		JSON Tools - Save
	*/

	updateJSON: function () {
		// JSONize the Data and save in in the input tag.	Normalize to 100.	
		this._dimensionstack.each(function(k, d){
			this._datapool[k]['Target'][0] = Math.round(100 / this.container.getSize().x * this._targetstack[k][0]);
			this._datapool[k]['Target'][1] = Math.round(100 / this.container.getSize().y * this._targetstack[k][1]);								
			this._datapool[k]['Target'][2] = this._targetstack[k][2];								

		},this);
		this.datafield.value = this._dataarray[0] + '<::::::>' + JSON.encode(this._datapool);
		this.fireEvent('save', this);		
	}
});

Element.implement({
	makeTarget: function(options){
		var target = new Target(this, options);
		return target;
	}

});
})();