/**
 * @file
 * A JavaScript file for the theme.
 *
 * In order for this JavaScript to be loaded on pages, see the instructions in
 * the README.txt next to this file.
 */

// JavaScript should be made compatible with libraries other than jQuery by
// wrapping it with an "anonymous closure". See:
// - https://drupal.org/node/1446420
// - http://www.adequatelygood.com/2010/3/JavaScript-Module-Pattern-In-Depth
(function ($, Drupal, window, document, undefined) {


// To understand behaviors, see https://drupal.org/node/756722#behaviors
Drupal.behaviors.my_custom_behavior = {
  attach: function(context, settings) {

    // Place your code here.

  }
};


})(jQuery, Drupal, this, this.document);


//jQuery functions
jQuery(document).ready(function($) {
	var eventFired = 0;

	/*
	if ($(window).width() < 1024 && $(window).width() > 640) {
        $("header .widget").prependTo(".site-header");
        $("header .user-block").prependTo(".site-header");
    } else if ($(window).width() < 641) {
    	$(".site-header .user-block").appendTo(".main-navigation .nav-menu");
    	$(".site-header .widget").appendTo(".main-navigation .nav-menu");
    } else {
        $("header .user-block").appendTo(".site-header");
        $("header .widget").appendTo(".site-header");
    }  

	$(window).on('resize', function() {
	//  if (!eventFired) {
	//   if ($(window).width() < 1024 && $(window).width() > 640) {
	//       $("header .widget").prependTo(".site-header");
	//       $("header .user-block").prependTo(".site-header");
	//      } else if ($(window).width() < 641) {
	//      	$(".site-header .user-block").appendTo(".main-navigation .nav-menu");
	//      	$(".site-header .widget").appendTo(".main-navigation .nav-menu");
	//      } else {
	//          $("header .user-block").appendTo(".site-header");
	//          $("header .widget").appendTo(".site-header");
	//      }  
	//  }
	});
	*/

	//Expand power cards
//	$('.power').toggleClass('hide');
//	$('.single-power .power').toggleClass('hide');
	
	//masonry
	var $container_node = $('.field-name-field-powers > .field-items');
	var $container_mitem = $('.field-name-field-character-magic-items > .field-items');
	var $container_term = $('.view-taxonomy-term');
	var $container_srd = $('.view-srd-class-powers');

	$(window).load(function() {
		$container_node.masonry({
		  columnWidth: '.node-power',
		  itemSelector: '.node-power'
		});

		$container_mitem.masonry({
		  columnWidth: '.node-magic-item',
		  itemSelector: '.node-magic-item'
		});

		$container_term.masonry({
		  columnWidth: '.node',
		  itemSelector: '.node'
		});

		$container_srd.masonry({
		  columnWidth: '.node',
		  itemSelector: '.node'
		});
	});

	//back to top
	$('body').append('<a href="#" id="back-to-top">Back to Top</a>');

	$('.node-power').each(function() {
		if ($(this).text().length > 400) {
			$(this).addClass('lengthy');
		}
	});

	//$('.entry-title .toggle').click(function() {
//	$('article').on('click','.entry-title .toggle', function() {
//		$(this).parent().parent().parent().toggleClass('hide');
//		//console.log($(this).parent().parent().parent());
//		$container.masonry({
//		  //columnWidth: 200,
//		  itemSelector: 'article.power'
//		});
//	});
});