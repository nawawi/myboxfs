<?
	$color='darkgray';
	$bcolor='lightgrey';
	if($msg=='Please wait. Registering session.') {
		$color='lightgreen';
	}
	if($msg=='Please wait. Performing login command.') {
		$color='lightgreen';
	}
	if($msg=='Please wait. Removing session.') {
		$color='red';
	}
	if($msg=='Password Incorrect. Please try again.') {
		$color='red';
	}
	if($msg=='Session Expired. Please try again.') {
		$color='red';
	}
	if($msg=='Please wait. Performing logout command.') {
		$color='red';
	}
?>

<script language="javascript"><!--
<?if($color=='red'){?>
if (top.frames.length!=0) // out from main frame
		top.location=self.document.location;
<?}?>
var loadedcolor='<?echo $color;?>';       // PROGRESS BAR COLOR
var unloadedcolor='<?echo $bcolor;?>';     // COLOR OF UNLOADED AREA
var bordercolor='black';            // COLOR OF THE BORDER
var barheight=10;                  // HEIGHT OF PROGRESS BAR IN PIXELS
var barwidth=350;                  // WIDTH OF THE BAR IN PIXELS
var waitTime='1';                   // NUMBER OF SECONDS FOR PROGRESSBAR
var action=function()
{
window.location='<?echo $where;?>';
}

var ns4=(document.layers)?true:false;
var ie4=(document.all)?true:false;
var blocksize=(barwidth-2)/waitTime/10;
var loaded='0';
var PBouter;
var PBdone;
var PBbckgnd;
var Pid='0';
var txt='';
if(ns4){
txt+='<table border=0 cellpadding=0 cellspacing=0><tr><td>';
txt+='<ilayer name="PBouter" visibility="hide" height="'+barheight+'" width="'+barwidth+'" onmouseup="hidebar()">';
txt+='<layer width="'+barwidth+'" height="'+barheight+'" bgcolor="'+bordercolor+'" top="0" left="0"></layer>';
txt+='<layer width="'+(barwidth-2)+'" height="'+(barheight-2)+'" bgcolor="'+unloadedcolor+'" top="1" left="1"></layer>';
txt+='<layer name="PBdone" width="'+(barwidth-2)+'" height="'+(barheight-2)+'" bgcolor="'+loadedcolor+'" top="1" left="1"></layer>';
txt+='</ilayer>';
txt+='</td></tr></table>';
}else{
txt+='<div id="PBouter" onmouseup="hidebar()" style="position:relative; visibility:hidden; background-color:'+bordercolor+'; width:'+barwidth+'px; height:'+barheight+'px;">';
txt+='<div style="position:absolute; top:1px; left:1px; width:'+(barwidth-2)+'px; height:'+(barheight-2)+'px; background-color:'+unloadedcolor+'; font-size:1px;"></div>';
txt+='<div id="PBdone" style="position:absolute; top:1px; left:1px; width:0px; height:'+(barheight-2)+'px; background-color:'+loadedcolor+'; font-size:1px;"></div>';
txt+='</div>';
}

document.write(txt);

function incrCount(){
window.status="Please wait, command running...";
loaded++;
if(loaded<0)loaded='0';
if(loaded>=waitTime*10){
clearInterval(Pid);
loaded=waitTime*10;
setTimeout('hidebar()',100);
}
resizeEl(PBdone, 0, blocksize*loaded, barheight-2, 0);
}

function hidebar(){
clearInterval(Pid);
window.status='';
//if(ns4)PBouter.visibility="hide";
//else PBouter.style.visibility="hidden";
action();
}

//THIS FUNCTION BY MIKE HALL OF BRAINJAR.COM
function findlayer(name,doc){
var i,layer;
for(i='0';i<doc.layers.length;i++){
layer=doc.layers[i];
if(layer.name==name)return layer;
if(layer.document.layers.length>0)
if((layer=findlayer(name,layer.document))!=null)
return layer;
}
return null;
}

function progressBarInit(){
PBouter=(ns4)?findlayer('PBouter',document):(ie4)?document.all['PBouter']:document.getElementById('PBouter');
PBdone=(ns4)?PBouter.document.layers['PBdone']:(ie4)?document.all['PBdone']:document.getElementById('PBdone');
resizeEl(PBdone,0,0,barheight-2,0);
if(ns4)PBouter.visibility="show";
else PBouter.style.visibility="visible";
Pid=setInterval('incrCount()',95);
}

function resizeEl(id,t,r,b,l){
if(ns4){
id.clip.left=l;
id.clip.top=t;
id.clip.right=r;
id.clip.bottom=b;
}else id.style.width=r+'px';
}

window.onload=progressBarInit;
//--></script>

