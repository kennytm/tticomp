/**
 * This JavaScript file is copyright (c) 2002 by Rogier van Dalen.
 * 
 * It will first add a title and then define some helper functions and a
 * beautify() function that adds lines to headers, adds a footer, and
 * shows a navigation window. beautify() should be called after the main text
 * has been loaded as it iterates through the list of tags to beautify the
 * headers and to find the right contents for the navigation window.
 *
 * This file had better be included just after the HTML <BODY> definition
 * to show the title as quickly as possible.
 **/

// Add document title
var title = document.createElement("H1");
title.innerHTML = document.title;
document.body.insertBefore(title, document.body.firstChild);

// Global variables

var pages = [[0, "Home", "index.html"],
	[1, "News", "news.html"],
	[1, "TrueType Viewer", "truetypeviewer.html"],
	[1, "TT Instruction Compiler", "tticomp.html"],
	[2, "Tutorial", "tticomp/introduction.html"],
	[3, "Language", "tticomp/language.html"],
	[3, "Linking", "tticomp/linking.html"],
	[3, "Options", "tticomp/options.html"],
	[3, "Execution", "tticomp/execution.html"],
	[1, "OpenType Compiler", "otcomp.html"],
	[2, "Tutorial", "otcomp/introduction.html"],
	[3, "OpenType Terms", "otcomp/terms.html"],
	[3, "OpenType Concepts", "otcomp/concepts.html"],
	[3, "File Format", "otcomp/format.html"],
	[3, "Lookups", "otcomp/lookups.html"],
	[4, "Single Substitution", "otcomp/singlesubst.html"],
	[4, "Multiple Substitution", "otcomp/multiplesubst.html"],
	[4, "Alternate Substitution", "otcomp/alternatesubst.html"],
	[4, "Ligature Substitution", "otcomp/ligaturesubst.html"],
	[4, "Positioning Parameters", "otcomp/positioning.html"],
	[4, "Single Positioning", "otcomp/singlepos.html"],
	[4, "Pair Positioning", "otcomp/pairpos.html"],
	[4, "Anchors", "otcomp/anchors.html"],
	[4, "Cursive Attachment", "otcomp/cursiveattach.html"],
	[4, "Mark-to-base Attachment", "otcomp/marktobaseattach.html"],
	[4, "Mark-to-ligature Attachment", "otcomp/marktoligaattach.html"],
	[4, "Mark-to-mark Attachment", "otcomp/marktomarkattach.html"],
	[4, "Contextual Lookups", "otcomp/contextual.html"],
	[4, "Reverse Chaining Lookups", "otcomp/reversechainingsubst.html"],
	[1, "OpenType Legacy", "otlegacy.html"],
	[1, "Program Source", "source.html"],
	[1, "Fonts", "fonts.html"]];

var relativePath;
var navigation;
var contents;
var contentsVisible = true;
var contentsHeader;
var tooltip;

// Contents dragging

function getPageMouse(e) {
	if (e && e.pageX)
		return {x: e.pageX, y: e.pageY};
	else
		return {x: window.event.x + document.body.scrollLeft,
				y: window.event.y + document.body.scrollTop};
}

function getScreenMouse(e) {
	if (e && e.screenX)
		return {x: e.screenX, y: e.screenY};
	else
		return {x: window.event.x,
				y: window.event.y};
}

function scrollIE() {
	navigation.style.top = document.body.scrollTop + navigation.dragTop + "px";
	navigation.style.right = navigation.dragRight + "px";
	setTimeout(scrollIE, 100);
}

// Navigation dragging

var dragCurEl = null;
var dragMouse;

function dragMouseMove(e) {
	var newMouse = getScreenMouse(e);
	dragCurEl.dragRight -= (newMouse.x - dragMouse.x);
	dragCurEl.style.right = dragCurEl.dragRight + "px";
	dragCurEl.dragTop += (newMouse.y - dragMouse.y);

	// IE workaround to fix fixed positioning
	if (document.all)
		dragCurEl.style.top = document.body.scrollTop + dragCurEl.dragTop + "px";
	else
		dragCurEl.style.top = dragCurEl.dragTop + "px";
	dragMouse = newMouse;
}

function dragMouseDown(e) {
	dragCurEl = navigation;

	dragMouse = getScreenMouse(e);
	
	document.onmousemove = dragMouseMove;
	document.onmouseup = dragMouseUp;
	return true;
}

function dragMouseUp(e) {
	dragCurEl = null;
	document.onmousemove = null;
	document.onmouseup = null;
}


// Contents toggling

function toggleContents() {
	while (contentsHeader.firstChild)
		contentsHeader.removeChild(contentsHeader.firstChild);
	if (contentsVisible) {
		contents.style.display = "none";
		var newA = document.createElement("A");
		newA.href = "javascript:toggleContents();";
		newA.innerHTML = "Contents";
		contentsHeader.appendChild(newA);
	} else {
		contents.style.display = "block";
		contentsHeader.innerHTML = "Contents <SPAN id='contentsHide'>(<A href='javascript:toggleContents();'>hide</A>)</SPAN>";
	}
	contentsVisible = !contentsVisible;
}

/*
	Append a line after all "tagName" elements and put the header and the
	line into a new DIV element (to circumvent buggy IE behaviour).
*/

function makeLine(tagName) {
	headers = document.getElementsByTagName(tagName);
	for (i=0; i<headers.length; i++) {
		var container = document.createElement("DIV");
		var line = document.createElement("DIV");
		headers[i].parentNode.insertBefore(container, headers[i]);
		container.className = tagName + "container";
		line.className = tagName + "line";
		container.appendChild(headers[i]);
		container.appendChild(line);
	}
}



function beautify() {
	if (!document.getElementById) {
		document.write("Warning: you need a browser supporting CSS and DOM1 for this page, which you don't seem to have. Mozilla should work, IE 5 should work but does only partly.");
		return;
	}

	relativePath = "";

	// Add navigation
	for (var i=0; i<documentPath.length; i++) {
		if (documentPath.charAt(i) == '/')
			relativePath += "../";
	}


	navigation = document.createElement("DIV");
	navigation.className = "navigation";
	navigation.dragRight = 10;
	navigation.dragTop = 10;
	navigation.style.minWidth="8em";


	// Workaround for IE bug concerning fixed positioning
	if (document.all) {
		navigation.style.position="absolute";
		scrollIE();
	} else {
		navigation.style.right = navigation.dragRight + "px";
		navigation.style.top = navigation.dragTop + "px";
	}

	contentsHeader = document.createElement("DIV");
	contentsHeader.className = "contentsHeader";
//	contentsHeader.innerHTML = contentsHeaderText;
	// Mozilla 1.2 bug?
	//alert(contentsHeader.innerHTML = contentsHeaderText);
	//alert(contentsHeader.innerHTML);

	navigation.appendChild(contentsHeader);

	contents = document.createElement("DIV");
	contents.className = "contents";
	contents.id = "contents";
	contents.style.display = "none";		// Initially

	// Get current level
	var thisPage = 0;
	for (var i=0; i<pages.length; i++) {
		if (documentPath == pages[i][2])
			thisPage = i;
	}

	var firstLevel = Array (20);
	var lastLevel = Array (20);
	var curLevel = 0;
	for (var i=0; i <= thisPage; i ++)
		firstLevel [pages[i][0]] = i;
	
	curLevel = pages [thisPage][0];
	for (var i = thisPage; i < pages.length; i ++) {
		if (pages [i][0] <= curLevel) {
			for (var j = curLevel; j >= pages[i][0]; j --)
				lastLevel [j] = i - 1;
			curLevel = pages [i][0];
		}
	}
	for (var j = curLevel; j >= 0; j--)
		lastLevel [j] = i-1;

	/** Get contents from "pages" variable **/
	for (var i=0; i<pages.length; i++) {
		//if (pages[i][0] <= pages[thisPage][0]+1) {
		if (pages[i][0] == 0 ||
			(firstLevel [pages[i][0]-1] <= i && i <=lastLevel [pages [i][0]-1]))
		{
			var contentsItem = document.createElement("DIV");
			contentsItem.className = "contentsItem";
			contentsItem.style.marginLeft = pages[i][0]+"em";
			var contentsRef = document.createElement("A");
			contentsRef.href = relativePath + pages[i][2];
			contentsRef.innerHTML = pages[i][1];
			contentsItem.appendChild(contentsRef);
			contents.appendChild(contentsItem);
			if (documentPath == pages[i][2]) {
				contentsItem.id = "currentItem";

				var tags = document.getElementsByTagName("H2");
				for (var j=0; j<tags.length; j++) {
					var contentsItem = document.createElement("DIV");
					contentsItem.className = "thisPageItem";
					contentsItem.style.marginLeft = (pages[i][0]+1) + "em";
					var contentsRef = document.createElement("A");
					contentsRef.href = "#Header" + j;
					contentsRef.id = "contentsHeader" + j;
					contentsRef.innerHTML = tags[j].innerHTML;
					contentsItem.appendChild(contentsRef);
					contents.appendChild(contentsItem);

					tags[j].innerHTML = "<A name='Header" + j + "'></A>" + tags[j].innerHTML;
				}
			}
		}
	}

	navigation.appendChild(contents);
	document.body.appendChild(navigation);
	toggleContents();

	contentsHeader.onmousedown = dragMouseDown;

	// Make "Next" and "Previous" buttons

	if (document.switcherNext || document.switcherPrevious) {
		var switcherDiv = document.createElement("DIV");
		switcherDiv.className = "pageSwitcher";

		if (document.switcherPrevious) {
			var previousSpan = document.createElement("A");
			previousSpan.href = relativePath + pages[thisPage-1][2];
		} else {
			var previousSpan = document.createElement("SPAN");
		}
		previousSpan.innerHTML = "&lt;&lt; Previous";
		previousSpan.style.marginRight = ".5em";
		switcherDiv.appendChild(previousSpan);

		/*if (document.switcherPrevious)
			switcherDiv.innerHTML = "<A href='"+relativePath+pages[thisPage-1][2]+"'>&lt;&lt; Previous</A>  ";
		else
			switcherDiv.innerHTML = "&lt;&lt; Previous  ";

		if (document.switcherNext)
			switcherDiv.innerHTML += "  <A href='"+relativePath+pages[thisPage+1][2]+"'>Next &gt;&gt;</A>";
		else
			switcherDiv.innerHTML += "  Next &gt;&gt;";*/

		if (document.switcherNext) {
			var nextSpan = document.createElement("A");
			nextSpan.href = relativePath + pages[thisPage+1][2];
		} else {
			var nextSpan = document.createElement("SPAN");
		}
		nextSpan.innerHTML = "Next &gt;&gt;";
		nextSpan.style.marginLeft = ".5em";
		switcherDiv.appendChild(nextSpan);


		document.body.insertBefore(switcherDiv, document.body.firstChild.nextSibling);
		switcherDiv = switcherDiv.cloneNode(true);
		document.body.appendChild(switcherDiv);
	}


	makeLine("H2");
	makeLine("H1");

	// Find tooltips

	/*tooltip = document.createElement("DIV");
	tooltip.onmousedown = delayHideTooltip;
	tooltip.className = "tooltip";
	document.body.appendChild(tooltip);*/

	var ttIndex = 1;

	tags = document.getElementsByTagName("A");
	for (var i=0; i<tags.length; i++) {
		if (tags[i].className == "tooltipLink") {
			tags[i].onclick = showTooltip;

			var reference = document.createElement("SPAN");
			reference.className = "printOnly";
			reference.innerHTML = " ["+ttIndex+"]";
			tags[i].parentNode.insertBefore(reference, tags[i].nextSibling);

			var tooltip = document.createElement("DIV");
			tooltip.onmousedown = delayHideTooltip;
			tooltip.className = "tooltip";
			tooltip.id = "tooltip" + tags[i].id;

			var tip = getTip(tags[i]);
			tooltip.innerHTML = "<SPAN class='printOnly'>["+ttIndex+"] </SPAN>" + tip.text;

			if (tip.closeButton) {
				var closeDiv = document.createElement("DIV");
				closeDiv.innerHTML="<A href='javascript:hideTooltipNow();'>close</A>";
				closeDiv.className = "closeButton";
				tooltip.insertBefore(closeDiv, tooltip.firstChild);
			}

			document.body.appendChild(tooltip);
			ttIndex ++;
		} else {
			if (tags[i].href && tags[i].href != "#") {
				var reference = document.createElement("SPAN");
				reference.className = "printLink";
				reference.innerHTML = " (" + tags[i].href + ")";
				tags[i].parentNode.insertBefore(reference, tags[i].nextSibling);
			}
		}
	}

	// Add footer

	var footer = document.createElement("DIV");
	footer.innerHTML = "This page was last updated on " + lastEdited +
		" by <A href='mailto:R.C.van.Dalen@umail.leidenuniv.nl'>Rogier van Dalen</a>. "+
		"It should be readable on any browser; it should look good and print well on version 5 browsers. "+
		"Please direct any feedback to me.";
	var line = document.createElement("DIV");
	footer.className="footer";
	line.className="footerline";
	document.body.appendChild(line);
	document.body.appendChild(footer);
}

// Tooltips

var tooltipAnchor = null;
var delayTooltipHiding = false;

function hideTooltipNow() {
	tooltip.style.display = null;
	tooltipAnchor.onblur = null;
	delayTooltipHiding = false;
}

function hideTooltip() {
	if (delayTooltipHiding) {
		setTimeout("tooltip.style.visibility = 'hidden';", 1500);
		delayTooltipHiding = false;
	} else
		tooltip.style.display = "";
	tooltipAnchor.onblur = null;
}

function hideTooltipClicked() {
	hideTooltip();
	document.body.onmousedown = null;
}

function delayHideTooltip() {
	delayTooltipHiding = true;
}

function showTooltip(e) {
	if (window.event)
		tooltipAnchor = window.event.srcElement;
	else
		tooltipAnchor = this;
	tooltip = document.getElementById("tooltip" + tooltipAnchor.id);
	var mouse = getPageMouse(e);
	// getTip() should be defined in the HTML document
	var tip = getTip(tooltipAnchor);

	if (!tip.closeButton)
		tooltipAnchor.onblur = hideTooltip;

	tooltip.style.left = mouse.x+"px";
	tooltip.style.top = mouse.y+10+"px";
	tooltip.style.display = "block";

	return false;
}

function replaceInString(oldString, seq1, seq2, auto) {
	var newString = "";
	while (oldString.indexOf(seq1)!= -1) {
		var index = oldString.indexOf(seq1);
		if (auto || (index==0 || ((oldString.charAt(index-1)<'a' || oldString.charAt(index-1)>'z') &&
			(oldString.charAt(index + seq1.length)<'a' || oldString.charAt(index + seq1.length)>'z'))))
			newString += oldString.substring(0, index) + seq2;
		else {
			newString += oldString.substring(0, index + seq1.length);
		}
		oldString = oldString.substring(index + seq1.length, oldString.length);
	}
	newString += oldString;
	return newString;
}

var TTIReservedWords = ["uint", "int", "const", "fixed", "bool", "void", "false", "true",
	"#input", "#output", "#gasp", "#twilight", "#stack", "#cvt", "if", "else", "while", "return"];

var OTReservedWords = ["#input", "#output", "script", "language", "required","feature",
	"lookup", "sub", "pos", "mark", "ignore", "base", "ligature", "component", "group"];

var CReservedWords = ["#include", "#pragma", "pack"];

function replaceReservedWords (oldString, reservedWords) {
	for (var i = 0; i < reservedWords.length; i++)
		oldString = replaceInString(oldString, reservedWords[i],
			"<SPAN class='reservedWord'>" + reservedWords[i] + "</SPAN>");
	return oldString;
}

function showReservedWords (reservedWords) {
	var tags = document.getElementsByTagName("CODE");
	for (var i=0; i<tags.length; i++) {
		tags[i].innerHTML = replaceReservedWords(tags[i].innerHTML, reservedWords);
	}
}
