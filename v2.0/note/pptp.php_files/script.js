function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}
function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}
function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}
function MM_jumpMenu(targ,selObj,restore){ //v3.0
  if(selObj.options[selObj.selectedIndex].value != 0) {
	  eval(targ+".location='"+selObj.options[selObj.selectedIndex].value+"'");
   }
  if (restore) selObj.selectedIndex=0;
}
function jumpMenu(targ,selObj,restore){ //v3.0
  eval(targ+".location='"+selObj.options[selObj.selectedIndex].value+"'");
  if (restore) selObj.selectedIndex=0;
}
function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}
function MM_jumpMenuGo(selName,targ,restore){ //v3.0
  var selObj = MM_findObj(selName); if (selObj) MM_jumpMenu(targ,selObj,restore);
}
function jumpMenuGo(selName,targ,restore){ //v3.0
  var selObj = MM_findObj(selName); if (selObj) MM_jumpMenu(targ,selObj,restore);
}
function confirmAction(question, url) {
	if(confirm(question)) {
		eval("parent.location='"+url+"'");
	}
}
function addEvent(obj, evType, fn){ 
 if (obj.addEventListener){ 
   obj.addEventListener(evType, fn, true); 
   return true; 
 } else if (obj.attachEvent){ 
   var r = obj.attachEvent("on"+evType, fn); 
   return r; 
 } else { 
   return false; 
 } 
}
addEvent(window, 'load', function() { 
 var input, textarea; 
 var inputs = document.getElementsByTagName('input');
 for (var i = 0; (input = inputs[i]); i++) { 
   if(input.type == "text" || input.type == "password") {
     addEvent(input, 'focus', oninputfocus); 
     addEvent(input, 'blur', oninputblur); 
   }
 } 
 var textareas = document.getElementsByTagName('textarea'); 
 for (var i = 0; (textarea = textareas[i]); i++) { 
   addEvent(textarea, 'focus', oninputfocus); 
   addEvent(textarea, 'blur', oninputblur); 
 } 
}); 
function oninputfocus(e) { 
 /* Cookie-cutter code to find the source of the event */ 
 if (typeof e == 'undefined') { 
   var e = window.event; 
 } 
 var source; 
 if (typeof e.target != 'undefined') { 
    source = e.target; 
 } else if (typeof e.srcElement != 'undefined') { 
    source = e.srcElement; 
 } else { 
   return; 
 } 
 /* End cookie-cutter code */ 
 source.className = "focus";
} 
function oninputblur(e) { 
 /* Cookie-cutter code to find the source of the event */ 
 if (typeof e == 'undefined') { 
   var e = window.event; 
 } 
 var source; 
 if (typeof e.target != 'undefined') { 
    source = e.target; 
 } else if (typeof e.srcElement != 'undefined') { 
    source = e.srcElement; 
 } else { 
   return; 
 } 
 /* End cookie-cutter code */ 
 source.className = "blur";
}
function focusElement(id) {
  element = MM_findObj(id);
  element.className = "focus";
  element.focus();
}