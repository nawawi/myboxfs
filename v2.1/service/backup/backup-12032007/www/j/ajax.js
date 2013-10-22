function createRequestObject() {var con_id;try { con_id=new ActiveXObject("Msxml2.XMLHTTP"); }catch(e) {try { con_id=new ActiveXObject("Microsoft.XMLHTTP");}catch(f) { con_id=null; }};if(!con_id&&typeof XMLHttpRequest!="undefined") {con_id=new XMLHttpRequest();};return con_id;}
var http = createRequestObject();
function handleResponseText() {try {if((http.readyState == 4)&& (http.status == 200)){var response = http.responseText;document.getElementById("ajax_msg").style.display='';document.getElementById("ajax_msg").innerHTML = response;}}catch(e){};}
function sendRequestPost(file,xpost) {try{http.open('POST',file);http.setRequestHeader('Content-Type', "application/x-www-form-urlencoded");http.onreadystatechange = handleResponseText;http.send(xpost);}catch(e){};}
function sendRequestGet(params) {try{http.open('GET',params);http.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');http.onreadystatechange = handleResponseText;http.send(null);} catch(e){};}
