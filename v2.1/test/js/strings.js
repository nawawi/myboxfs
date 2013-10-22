
if (top.frames.length==0) {
	top.location='index.exh';
}
function Trim(str) {
	return(str.replace(/^\s*/,'').replace(/\s*$/,'').replace(/^\t*/,'').replace(/\t*$/,'').replace(/^\f*/,'').replace(/\f*$/,'').replace(/^\r*/,'').replace(/\r*$/,''));
}

function isInteger(val0){
	var val=Trim(val0)
	for(var i='0';i<val.length;i++){
		if(!isDigit(val.charAt(i))){return false;}
	}
	return true;
}

function isNum(num) {
	var val=Trim(num);
	for(var i='0';i<val.length;i++){
		if(!isDigit(val.charAt(i))){return false;}
	}
	return true;
}

function isDigit(num) {
	var string="1234567890";
	if (string.indexOf(num)!=-1){return true;}
	return false;
}

function isValidIPv4(Obj) {
	var matchArray=Obj.split(".");
	if(matchArray==null) {
		return false;
	}
	for(var i='0';i<matchArray.length;i++) {
		var val=matchArray[i];
		if(!isInteger(val)) {
			return false;
		}
		if(val>255) {
			return false;
		}
	}
	return true;
}

function isgroupDigit(num) {
	var string="1234567890/-";
	if (string.indexOf(num)!=-1){return true;}
	return false;
}
function isgroupInteger(val0){
	var val=Trim(val0)
	for(var i='0';i<val.length;i++){
		if(!isgroupDigit(val.charAt(i))){return false;}
	}
	return true;
}

function isIPgroup(ip) {
	var matchArray=ip.split(".");
	if(matchArray==null) {
		return false;
	}
	for(var i='0';i<matchArray.length;i++) {
		var val=matchArray[i];
		if(!isgroupInteger(val)) {
			return false;
		}
		if(val>255) {
			return false;
		}
	}
	return true;
}

function isPortgroup(port) {
	var string="1234567890:-";
	var val=Trim(port);
	for(var i='0';i<val.length;i++){
		if(string.indexOf(val.charAt(i))==-1){return false;}
	}
	return true;
}

var popwin=null;
function popupwin_open(txt,w,h) {
	LeftPosition = (screen.width) ? (screen.width-620)/2 : 0;
	TopPosition = (screen.height) ? (screen.height-530)/2 : 0;
	settings='width='+w+',height='+h+',top='+TopPosition+',left='+LeftPosition+',scrollbars=0,status=0,resizable=0,copyhistory=0';
	var html='';
	html +='<html><head><title>Info</title></head><body scroll=no style="border-style: solid;border-width: 0px">';
	html +=txt;
	html +='</body></html>';
	popwin=window.open("","Info",settings);
	popwin.document.write(html);
	popwin.focus();
}

function popupwin_close() {
	popwin.close();
	popwin=null;
}
function popup_on(divid) {
	document.getElementById(divid).style.visibility = "visible";
}
function popup_off(divid) {
	document.getElementById(divid).style.visibility = "hidden";
}

function check_valid_email(email) {
	var m1=/^(.+)@(.+)$/;
	var ok='1',f='';
	email=Trim(email);
	if(email!='') {
		if(email.match(m1)!=null) return true;
	}
	return false;
}

function isIPpolicy(ip) {
	var matchArray=ip.split(" ");
	if(matchArray==null) {
		if(!isIPgroup(ip)) {
			alert(ip);
			return false;
		}
	}
	for(var i='0';i<matchArray.length;i++) {
		var val=matchArray[i];
		if(!isIPgroup(val)) {
			return false;
		}
	}
	return true;
}
