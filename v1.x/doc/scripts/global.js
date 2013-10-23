<script language="javascript1.2"><!--
// we dont see any frame here
if (top.frames.length==0) {
	top.location='system_session.so';
}

// php strpos style
function strpos(str, ch) {
	var c=''; 
	for(i = 0; i < str.length; ++i) {
		c = str.charAt(i);
		if(c==ch) return 1;
	}	
	return 0;
}

function load(file) {
	if(strpos(file,'?')==0) {
		location.href=file;
	} else {
		location.href=file;
	}
}

function delspace(s) {
	var c;
	var p='';
	for(i = 0; i < s.length; ++i) {
		c = s.charAt(i);
		if(c==' ') c='.'
		p +=c
	}	
	return p;
}


var menuCount = 0;
var itemCount = 0;

if(disallowedTags == undefined) {
	//var disallowedTags = ["A", "BUTTON", "IMG", "INPUT", "OBJECT", "OPTION", "SELECT", "TEXTAREA"];
	var disallowedTags = ["IMG", "INPUT", "TEXTAREA"];
}

if(menuClass == undefined) {
	var menuClass = "jsdomenudiv";
}

if(itemClass == undefined) {
	var itemClass = "jsdomenuitem";
}

if(itemClassOver == undefined) {
	var itemClassOver = "jsdomenuitemover";
}

if(hrClass == undefined) {
	var hrClass = "jsdomenuhr";
}

if(arrowClass == undefined) {
	var arrowClass = "jsdomenuarrow";
}

if(imgSrc == undefined) {
	var imgSrc = "image/rightclick/arrow.png";
}

if(imgSrcOver == undefined) {
	var imgSrcOver = "image/rightclick/arrow_o.png";
}

if(webPageMode == undefined) {
	var webPageMode = 0;
}

function getElm(id) {
	return document.getElementById(id);
}

function getTags(tagName) {
	return document.getElementsByTagName(tagName);
}

function createElm(tagName) {
	return document.createElement(tagName);
}

function appendChild(id, elm) {
	var divElm = getElm(id);
	divElm.appendChild(elm);
}

function isIE() {
	return (navigator.userAgent.indexOf("MSIE") > -1);
}

function isGecko() {
	return (navigator.userAgent.indexOf("Gecko") > -1);
}

function getX(e) {
	if(isGecko()) {
		return e.clientX;
	} else {
		return window.event.x;
	}
}

function getY(e) {
	if(isGecko()) {
		return e.clientY;
	} else {
		return window.event.y;
	}
}

function checkTag(tagName) {
	for(i = 0; i < disallowedTags.length; i++) {
		if(tagName == disallowedTags[i]) {
			return true;
		}
	}
		return false;
}

function getBorderOffset(elm) {
	return (parseInt(elm.style.borderTopWidth.slice(0, -2)) + parseInt(elm.style.borderBottomWidth.slice(0, -2)))
}

function getScrollLeft() {
	switch (webPageMode) {
		case 0:
		return document.body.scrollLeft;
		case 1:
		if(isIE()) {
			return document.documentElement.scrollLeft;
		} else {
			return document.body.scrollLeft;
		}
		case 2:
		return document.body.scrollLeft;
	}
}

function getScrollTop() {
	switch (webPageMode) {
		case 0:
		return document.body.scrollTop;
		case 1:
		if (isIE()) {
			return document.documentElement.scrollTop;
		} else {
			return document.body.scrollTop;
		}
		case 2:
		return document.body.scrollTop;
	}
}

function getClientHeight() {
	switch (webPageMode) {
		case 0:
		return document.body.clientHeight;
		case 1:
		if (isIE()) {
			return document.documentElement.clientHeight;
		} else {
			return document.body.clientHeight;
		}
		case 2:
		return document.body.clientHeight;
	}
}

function getClientWidth() {
	switch (webPageMode) {
		case 0:
		return document.body.clientWidth;
		case 1:
		if(isIE()) {
			return document.documentElement.clientWidth;
		} else {
			return document.body.clientWidth;
		}
		case 2:
		return document.body.clientWidth;
	}
}

function getMainLeftPos(divElm, x) {
	if ((x + divElm.offsetWidth) <= getClientWidth()) {
		return x;
	} else {
		if (x <= getClientWidth()) {
			return (x - divElm.offsetWidth);
		} else {
			return (getClientWidth() - divElm.offsetWidth);
		}
	}
}

function getMainTopPos(divElm, y) {
	if ((y + divElm.offsetHeight) <= getClientHeight()) {
		return y;
	} else {
		if (y <= getClientHeight()) {
			return (y - divElm.offsetHeight);
		} else {
			return (getClientHeight() - divElm.offsetHeight);
		}
	}
}

function getSubLeftPos(divElm, x, offset) {
	if ((x + divElm.offsetWidth - 3) <= getClientWidth()) {
		return (x - 3);
	} else {
		if (x <= getClientWidth()) {
			return (x - divElm.offsetWidth - offset);
		} else {
			return (getClientWidth() - divElm.offsetWidth);
		}
	}
}

function getSubTopPos(divElm, y, offset) {
	if ((y + divElm.offsetHeight) <= getClientHeight()) {
		return y;
	} else {
		if (y <= getClientHeight()) {
			return (y - divElm.offsetHeight + offset);
		} else {
			return (getClientHeight() - divElm.offsetHeight + offset);
		}
	}
}

function popUpMainMenu(e, divElm) {
	divElm.style.left = (getMainLeftPos(divElm, getX(e)) + getScrollLeft()) + "px";
	divElm.style.top = (getMainTopPos(divElm, getY(e)) + getScrollTop()) + "px";
}

function popUpSubMenu(parent, menuItem, divElm) {
	divElm.style.left = (getSubLeftPos(divElm, (parent.offsetLeft + parent.offsetWidth - getScrollLeft()), menuItem.offsetWidth) + getScrollLeft()) + "px";
	divElm.style.top = (getSubTopPos(divElm, (parent.offsetTop + menuItem.offsetTop - getScrollTop()), (getBorderOffset(divElm) + menuItem.offsetHeight)) + getScrollTop()) + "px";
}

function showSubMenu() {
	popUpSubMenu(arguments[0].parent.properties, arguments[0], arguments[0].subMenu.properties);
	arguments[0].subMenu.properties.style.visibility = "visible";
}

function menuItemOver() {
	if (this.parent.previousItem) {
		if (this.parent.previousItem.className == this.parent.previousItem.classNameOver) {
			this.parent.previousItem.className = this.parent.previousItem.itemClass;
		}
		var divAll = getTags("div");    
		if (this.parent.previousItem.hasSubMenu) {
			this.parent.previousItem.className = this.parent.previousItem.itemClass;
			var imgElm = getElm(this.parent.previousItem.id + "Arrow");
			imgElm.src = this.parent.previousItem.imgSrc;
		}
		for (i = 0; i < divAll.length; i++) {
			if(divAll[i].id.indexOf("DOMenu") > -1) {
				if(divAll[i].level > this.parent.level) {
					with (divAll[i].style) {
						left = "0px";
						top = "0px";
						visibility = "hidden";
					}
					for (j = 0; j < divAll[i].childNodes.length; j++) {
						if (divAll[i].childNodes[j].enabled) {
							divAll[i].childNodes[j].className = divAll[i].childNodes[j].itemClass;
							if (divAll[i].childNodes[j].hasSubMenu) {
								var imgElm = getElm(divAll[i].childNodes[j].id + "Arrow");
								imgElm.src = divAll[i].childNodes[j].imgSrc;
							}
						}
					}
				}
			}
		}
	}
	if (this.enabled) {
		this.className = this.classNameOver;
		if (this.hasSubMenu) {
			var imgElm = getElm(this.id + "Arrow");
			imgElm.src = this.imgSrcOver;
			showSubMenu(this);
		}
	}
	this.parent.previousItem = this;
}

function menuItemOut() {
	if (this.enabled) {
		if (!((this.hasSubMenu) && (this.subMenu.properties.style.visibility == "visible"))) {
			this.className = this.itemClass;
		}
		if (this.hasSubMenu) {
			var imgElm = getElm(this.id + "Arrow");
			if (this.subMenu.properties.style.visibility == "visible") {
				imgElm.src = this.imgSrcOver;
			} else {
				imgElm.src = this.imgSrc;
			}
		}
	}
}

function menuItemClick() {
	if ((!this.hasSubMenu) && (this.enabled) && (this.target)) {
		location.href = this.target;
	}
}

function setClassName(className) {
	if (this.itemClass) {
		this.itemClass = className;
		this.className = this.itemClass;
		return;
  	}
	if (this.menuClass) {
		this.menuClass = className;
		this.properties.className = this.menuClass;
		return;
	}
}

function setArrowClassName(className) {
	if (this.subMenuItem) {
		this.arrowClass = className;
		this.subMenuItem.className = this.arrowClass;
		return
	}
}

function setImgSrc(imgSrc) {
	if (this.arrowItem) {
		this.imgSrc = imgSrc;
		this.arrowItem.src = this.imgSrc
	}
}

function setSubMenu() {
	var imgElm = createElm("img");
	imgElm.src = this.imgSrc;
	imgElm.id = this.id + "Arrow";
	var divElm = createElm("div");
	divElm.className = this.arrowClass;
	divElm.appendChild(imgElm);
	this.appendChild(divElm);
	this.hasSubMenu = true;
	this.subMenu = arguments[0];
	this.subMenuItem = divElm;
	this.arrowItem = imgElm
	this.setArrowClassName = setArrowClassName;
	this.setImgSrc = setImgSrc;
	arguments[0].properties.style.zIndex = this.parent.level + 1;
	arguments[0].level = this.parent.level + 1;
	arguments[0].properties.level = arguments[0].level;
}

function addMenuItem() {
	if (arguments[0].displayText == "-") {
		var hrElm = createElm("hr");
		hrElm.id = arguments[0].id;
		if (arguments[0].className.length > 0) {
			hrElm.hrClass = arguments[0].className;
		} else if (arguments[0].hrClass.length > 0) {
			hrElm.hrClass = arguments[0].hrClass;
		} else {
			hrElm.hrClass = hrClass;
		}
		hrElm.className = hrElm.hrClass;
		appendChild(this.id, hrElm);
		hrElm.parent = this;
		hrElm.onmouseover = menuItemOver;
		if (arguments[0].itemName.length > 0) {
			this.items[arguments[0].itemName] = hrElm;
		} else {
			this.items[this.items.length] = hrElm;
		}
	} else {
		var divElm = createElm("div");
		divElm.id = arguments[0].id;
		divElm.target = arguments[0].target;
		divElm.enabled = arguments[0].enabled;
		divElm.itemClass = arguments[0].className;
		divElm.classNameOver = arguments[0].classNameOver;
		divElm.className = divElm.itemClass;
		divElm.hasSubMenu = false;
		divElm.subMenu = null;
		divElm.arrowClass = arrowClass;
		divElm.imgSrc = imgSrc;
		divElm.imgSrcOver = imgSrcOver;
		appendChild(this.id, divElm);
		var textNode = document.createTextNode(arguments[0].displayText);
		appendChild(arguments[0].id, textNode);
		divElm.parent = this;
		divElm.setSubMenu = setSubMenu;
		divElm.setClassName = setClassName;
		divElm.onclick = menuItemClick;
		divElm.onmouseover = menuItemOver;
		divElm.onmouseout = menuItemOut;
		if (arguments[0].itemName.length > 0) {
			this.items[arguments[0].itemName] = divElm;
		} else {
			this.items[this.items.length] = divElm;
		}
	}
}

function menuItem() {
	this.id = "Item" + (++itemCount);
	this.displayText = arguments[0];
	this.itemName = "";
	this.target = "";
	this.enabled = true;
	if(this.displayText == "-") {
		this.className = hrClass;
	} else {
		this.className = itemClass;
  	}
	this.hrClass = hrClass;
	this.classNameOver = itemClassOver;
	var length = arguments.length;
	if (length > 1) {
		if (arguments[1].length > 0) {
			this.itemName = arguments[1];
		}
	}
	if (length > 2) {
		if (arguments[2].length > 0) {
			this.target = arguments[2];
		}
	} 
	if (length > 3) {
		if (typeof(arguments[3]) == "boolean") {
			this.enabled = arguments[3];
		}
	}
	if (length > 4) {
		if (arguments[4].length > 0) {
			if (arguments[4] == "-") {
				this.className = arguments[4];
				this.hrClass = arguments[4];
			} else {
				this.className = arguments[4];
			}
		}
	}
	if (length > 5) {
		if (arguments[5].length > 0) {
			this.classNameOver = arguments[5];
		}
  	}
}

function jsDOMenu() {
	this.id = "DOMenu" + (++menuCount);
	this.level = 10;
	this.items = new Array;
	this.previousItem = null;
	this.menuClass = menuClass;
	if(arguments.length > 1) {
		if (arguments[1].length > 0) {
			this.menuClass = arguments[1];
		}
	}
	var divElm = createElm("div");
	divElm.id = this.id;
	divElm.mainMenu = (menuCount == 1);
	divElm.level = this.level;
	divElm.className = this.menuClass;
	with (divElm.style) {
		width = arguments[0] + "px";
		left = "0px";
		top = "0px";
		borderWidth = "2px";
	}
	document.body.appendChild(divElm);
	this.properties = divElm;
	this.setClassName = setClassName;
	this.addMenuItem = addMenuItem;
}

function activatejsDOMenu(e) {
	if ((isIE()) && (checkTag(event.srcElement.tagName))) {
		return;
	}
	if ((isGecko()) && (checkTag(e.target.tagName))) {
		return;
	}
	var divAll = getTags("div");
	for (i = 0; i < divAll.length; i++) {
		if ((divAll[i].id.indexOf("DOMenu") > -1) && (divAll[i].mainMenu)) {
			var state = divAll[i].style.visibility;
			break;
		}
	}
	if (state == "visible") {
		for (i = 0; i < divAll.length; i++) {
			for (j = 0; j < divAll[i].childNodes.length; j++) {
				if (divAll[i].childNodes[j].enabled) {
					divAll[i].childNodes[j].className = divAll[i].childNodes[j].itemClass;
					if (divAll[i].childNodes[j].hasSubMenu) {
						var imgElm = getElm(divAll[i].childNodes[j].id + "Arrow");
						imgElm.src = divAll[i].childNodes[j].imgSrc;
					}
				}
 			}
		}
	}
	for (i = 0; i < divAll.length; i++) {
		if (divAll[i].id.indexOf("DOMenu") > -1) {
			if (state == "visible") {
				divAll[i].style.visibility = "hidden";
			} else {
				if(divAll[i].mainMenu) {
					popUpMainMenu(e, divAll[i]);
					divAll[i].style.visibility = "visible";
					break;
				}
			}
		}
	}
	return false;
}

function adjustArrowPos() {
	if (isIE()) {
		divAll = getTags("div");
		for (i = 0; i < divAll.length; i++) {
			if (divAll[i].subMenuItem) {
				divAll[i].subMenuItem.style.marginTop = (parseInt(divAll[i].subMenuItem.currentStyle.marginTop.slice(0, -2)) + 3) + "px";
				divAll[i].subMenuItem.style.left = (parseInt(divAll[i].subMenuItem.currentStyle.left.slice(0, -2)) + (-5)) + "px";
			}
		}
	}
	if ((isGecko()) && (webPageMode == 2)) {
		divAll = getTags("div");
		for (i = 0; i < divAll.length; i++) {
			if (divAll[i].subMenuItem) {
				divAll[i].subMenuItem.style.marginTop = (-10) + "px";
			}
		}
	}
}

function activatejsDOMenuBy(value) {
	adjustArrowPos();
	switch (value) {
	case 0:
		document.onclick = activatejsDOMenu;
		break;
    	case 1:
      		document.oncontextmenu = activatejsDOMenu;
		break;
    	case 2:
		document.onclick = activatejsDOMenu;
		document.oncontextmenu = activatejsDOMenu;
		break;
  	}
}

function initjsDOMenu() {
	menu1 = new jsDOMenu(100);
	with (menu1) {
		addMenuItem(new menuItem("", "", ""));
		addMenuItem(new menuItem("About", "", "about.so"));
		addMenuItem(new menuItem("-", "", ""));
		addMenuItem(new menuItem("Home", "item1", "main.so"));
		addMenuItem(new menuItem("Admin", "item2", "admin.so"));
		addMenuItem(new menuItem("Network", "item3", "network.so"));
		addMenuItem(new menuItem("DMZ", "item4", "dmz.so"));
		addMenuItem(new menuItem("Policy", "item5", "policy.so"));
		addMenuItem(new menuItem("IDS", "item6", "ids.so"));
		addMenuItem(new menuItem("Logs", "item7", "stats.so"));
		addMenuItem(new menuItem("-", "", ""));
		addMenuItem(new menuItem("Exit", "", "login.so?logout=1"));
		addMenuItem(new menuItem("", "", ""));
	}
	menu10 = new jsDOMenu(120);
	with (menu10) {
		addMenuItem(new menuItem("System Actions", "", "main0.so"));
		addMenuItem(new menuItem("System Info", "", "main1.so"));
		addMenuItem(new menuItem("System Update", "", "main2.so"));
	}
	menu20 = new jsDOMenu(130);
	with (menu20) {
		addMenuItem(new menuItem("WebAdmin Setup", "", "admin0.so"));
		addMenuItem(new menuItem("Backup/Restore", "", "admin1.so"));
		addMenuItem(new menuItem("Remote Backup", "", "admin4.so"));
		addMenuItem(new menuItem("Password Setup", "", "admin2.so"));
		addMenuItem(new menuItem("AlarmSetup", "", "admin3.so"));
	}
	menu30 = new jsDOMenu(140);
	with (menu30) {
		addMenuItem(new menuItem("DNS/Gateway", "", "network0.so"));
		addMenuItem(new menuItem("Interfaces Setup", "", "network1.so"));
		addMenuItem(new menuItem("Routing Setup", "", "network5.so"));
		addMenuItem(new menuItem("Host Addresses", "", "network2.so"));
		addMenuItem(new menuItem("ARP cache", "", "network3.so"));
		addMenuItem(new menuItem("Bandwidth Shaping", "", "network4.so"));
		addMenuItem(new menuItem("Link Failover", "", "network6.so"));
	}
	menu40 = new jsDOMenu(160);
	with (menu40) {
		addMenuItem(new menuItem("DMZ Services Setup", "", "dmz0.so"));
		addMenuItem(new menuItem("DMZ External IP Setup", "", "dmz1.so"));
		addMenuItem(new menuItem("DMZ Hostname Setup", "", "dmz2.so"));
	}
	menu50 = new jsDOMenu(120);
	with (menu50) {
		addMenuItem(new menuItem("Filter Policy", "", "policy0.so"));
		addMenuItem(new menuItem("Custom Policy", "", "policy1.so"));
		addMenuItem(new menuItem("Interfaces Policy", "", "policy5.so"));
		addMenuItem(new menuItem("NAT Policy", "", "policy2.so"));
		addMenuItem(new menuItem("String Policy", "", "policy3.so"));
		addMenuItem(new menuItem("IP Sysctl Setup", "", "policy4.so"));
	}
	menu60 = new jsDOMenu(130);
	with (menu60) {
		addMenuItem(new menuItem("Blocking Setup", "", "ids0.so"));
		addMenuItem(new menuItem("Signature Info", "", "ids1.so"));
		addMenuItem(new menuItem("Manage Signature", "", "ids2.so"));
	}
	menu70 = new jsDOMenu(110);
	with (menu70) {
		addMenuItem(new menuItem("Network Graph", "", "stats0.so"));
		addMenuItem(new menuItem("Statistic Graph", "", "stats1.so"));
		addMenuItem(new menuItem("IDS Logs", "", "stats2.so"));
		addMenuItem(new menuItem("Portscan Logs", "", "stats3.so"));
		addMenuItem(new menuItem("Policy Logs", "", "stats4.so"));
		addMenuItem(new menuItem("User Logs", "", "stats5.so"));
	}
	menu1.items["item1"].setSubMenu(menu10);
	menu1.items["item2"].setSubMenu(menu20);
	menu1.items["item3"].setSubMenu(menu30);
	menu1.items["item4"].setSubMenu(menu40);
	menu1.items["item5"].setSubMenu(menu50);
	menu1.items["item6"].setSubMenu(menu60);
	menu1.items["item7"].setSubMenu(menu70);
	activatejsDOMenuBy(1);
}

//--></script>
