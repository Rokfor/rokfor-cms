/*
---
script: nuTabs.js
decription: nuTabs - MooTools-based, transitionified switcherification
license: MIT-style license.
authors:
 - Oskar Krawczyk (http://nouincolor.com/)
requires:
  core:1.2.3:
  - Class.Extras
  - Element.Event
  - Element.Style
  - Element.Dimensions
  - Fx.Tween
  - Fx.Morph
  - String
  - Array
  more:1.2.3.2:
  - Element.Measure
provides: [nuTabs]
...
*/
 
var nuTabs = new Class({
    Implements: [Events, Options],

    options: {
        // transition: $empty,
        navActiveClass: 'selected'
    },
    
    initialize: function(tabsNav, tabsBody, options){
        this.setOptions(options);
        this.tNav = tabsNav;
        this.tBody = tabsBody;
        this.attach();
		this.resize(false,0);
    },
 
    attach: function(){
        this.tNav.each(function(tab, index){
            tab.addEvent('click', this.resize.bindWithEvent(this, index));
        }, this);
    },
 
    alteredHeight: function(index){
        // expose to measurement
        return this.tBody[index].measure(function(){
            return this.getSize().y;
        });
    },
 
    resize: function(e, index){
        if (e) e.stop();
        this.tBodyCont = this.tBody.getParent();
        this.tNavCont = this.tNav.getParent();
        

        // absolutize and hide the content items, set display off to make it compatible with form elements
        this.tBody.set('styles', {
            'opacity': 0,
			'display': 'none'
        }).fade('out');
 
		// Load an Url if href is set
		
		if (ajxLd = this.tNav[index].get('href')) {
			if (ajxLd != "#") {
				var req = new Request.HTML(
					{
					url: ajxLd, 
					noCache: true,
					evalScripts: false,
					onSuccess: function(responseTree, responseElements, responseHTML, responseJavaScript) {
						this.tBody[index].set('text', '');
						this.tBody[index].adopt(responseTree);
						eval(responseJavaScript);						
					}.bind(this)
				});
				req.send();
			}
		}

        // show the active content item
        this.tBody[index].set('styles', {
			'display': 'block',
            'opacity': 0
        }).fade('in');
        
        // add class to the active tab
        this.tNavCont.removeClass(this.options.navActiveClass);
        this.tNavCont[index].addClass(this.options.navActiveClass);
    }
});

Elements.implement({
    tabify: function(options){
        this.tabNav = this.slice(0, this.length/2);
        this.tabBody = this.slice(3, this.length);
        new nuTabs($$(this.tabNav), $$(this.tabBody), $pick(options, {}));
    }
});