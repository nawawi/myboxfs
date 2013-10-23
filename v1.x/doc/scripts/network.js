<script language="javascript1.2"><!--
function trimSpaces(str){
	var whiteSpace = " \r\n\t\f";
  	str += ""                      //Make sure str is a string
  	startPos = 0;                  //Note: if 0 length string startPos will be 0
  	endPos   = str.length - 1;     //Note: if 0 length string endPos will be -1
  	while(startPos < str.length){
     		if(whiteSpace.indexOf(str.charAt(startPos))< 0){
        		break;
     		} else {
        		startPos++;
     		}
  	}
  	while(endPos > -1){
     		if(whiteSpace.indexOf(str.charAt(endPos))< 0){
        		break;
     		} else {
        		endPos--;
     		}
  	}
	if(startPos > endPos) return '';
	return str.substring(startPos, endPos+1)
}

function isInteger(val0){
	var val=trimSpaces(val0)
	for(var i='0';i<val.length;i++){
		if(!isDigit(val.charAt(i))){return false;}
	}
	return true;
}

function isDigit(num) {
	var string="1234567890/:-";
	if (string.indexOf(num)!=-1){return true;}
	return false;
}

function checkIp(Obj) {
	var matchArray=Obj.split(".");
	if(matchArray==null) {
		alert("Incorrect IP Address: '"+ Obj +"'");
		return false;
	}
	for(var i='0';i<matchArray.length;i++) {
		var val=matchArray[i];
		if(!isInteger(val)) {
			alert("Invalid IP: '"+ Obj +"'");
			return false;
		}
		if(val>255) {
      			alert("Destination IP address is invalid: '"+ Obj +"'");
			return false;
		}
	}
	return true;
}

function emailCheck(emailStr) {
	var emailPat=/^(.+)@(.+)$/
	var specialChars="\\(\\)<>@,;:\\\\\\\"\\.\\[\\]"
	var validChars="\[^\\s" + specialChars + "\]"
	var quotedUser="(\"[^\"]*\")"
	var ipDomainPat=/^\[(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\]$/
	var atom=validChars + '+'
	var word="(" + atom + "|" + quotedUser + ")"
	var userPat=new RegExp("^" + word + "(\\." + word + ")*$")
	var domainPat=new RegExp("^" + atom + "(\\." + atom +")*$")
	var matchArray=emailStr.match(emailPat)

	if(matchArray==null) {
		alert("Incorrect Email address (check @ and .'s): '"+ emailStr +"'");
		return false
	}

	var user=matchArray[1]
	var domain=matchArray[2]

	if (user.match(userPat)==null) {
    		alert("Invalid Username: '"+ user +"'")
    		return false
	}
	var IPArray=domain.match(ipDomainPat)
	if(IPArray!=null) {
	  	for(var i='1';i<='4';i++) {
	    		if(IPArray[i]>255) {
	        		alert("Destination IP address is invalid: '"+ domain +"'");
				return false
	    		}
    		}
    		return true
	}
	var domainArray=domain.match(domainPat)
	if(domainArray==null) {
		alert("The domain name doesn't seem to be valid: '"+ domain +"'");
    		return false
	}
	var atomPat=new RegExp(atom,"g")
	var domArr=domain.match(atomPat)
	var len=domArr.length
	if(domArr[domArr.length-1].length<2 || domArr[domArr.length-1].length>3) {
   		alert("The address must end in a three-letter domain, or two letter country.")
   		return false
	}
	if(len<2) {
   		var errStr="This address is missing a hostname!: '"+ domain +"'";
   		alert(errStr)
   		return false
	}
	return true;
}

function netfromIp(Obj) {
	var matchArray=Obj.split(".");
	if(matchArray==null) {
		return '0.0.0.0';
	}
	var ip1=matchArray[0];
	var ip2=matchArray[1];
	var ip3=matchArray[2];
	var ip=ip1+'.'+ip2+'.'+ip3+'.0';
	return ip;
}

function bcastfromIp(Obj) {
	var matchArray=Obj.split(".");
	if(matchArray==null) {
		return '0.0.0.255';
	}
	var ip1=matchArray[0];
	var ip2=matchArray[1];
	var ip3=matchArray[2];
	var ip=ip1+'.'+ip2+'.'+ip3+'.255';
	return ip;
}

function domainfromhost(Obj) {
	var matchArray=Obj.split(".");
	if(matchArray==null) {
		return Obj;
	}
	var len=matchArray.length;
	var ip='';
	for(i='1';i<=len-1;i++) {
		ip += matchArray[i];
		if(i!=len-1) ip += ".";
	}
	return ip;
}
//--></script>
