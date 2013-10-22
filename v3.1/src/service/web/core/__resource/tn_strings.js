/*
 * Tracenetwork Javascript String function.
 * Most of function taken from phpjs library 
 * More info at: http://kevin.vanzonneveld.net/techblog/article/phpjs_licensing/
 *
 */

function ucfirst(str) {
	str += '';
	var f = str.charAt(0).toUpperCase();
	return f + str.substr(1);
}

function ucwords(str) {
	return str.replace(/^(.)|\s(.)/g, function($1) { return $1.toUpperCase(); } );
}

function htmlspecialchars(str) {
	var string = str.toString();
	string = string.replace(/&/g, '&amp;');
	string = string.replace(/</g, '&lt;');
	string = string.replace(/>/g, '&gt;');
	string = string.replace(/"/g, '&quot;');
	string = string.replace(/'/g, '&#039;');
	return string;
}

function htmlspecialchars_decode(str) {
	var string = str.toString();
	string = string.replace(/&amp;/g, '&');
	string = string.replace(/&lt;/g, '<');
	string = string.replace(/&gt;/g, '>');
	string = string.replace(/&quot;/g, '"');
	string = string.replace(/&#039;/g, '\'');
	return string;
}

function is_ipv6(str) {
	var pat="/^(([A-Fa-f0-9]{1,4}:){7}[A-Fa-f0-9]{1,4})$|^([A-Fa-f0-9]{1,4}::([A-Fa-f0-9]{1,4}:){0,5}[A-Fa-f0-9]{1,4})$|^(([A-Fa-f0-9]{1,4}:){2}:([A-Fa-f0-9]{1,4}:){0,4}[A-Fa-f0-9]{1,4})$|^(([A-Fa-f0-9]{1,4}:){3}:([A-Fa-f0-9]{1,4}:){0,3}[A-Fa-f0-9]{1,4})$|^(([A-Fa-f0-9]{1,4}:){4}:([A-Fa-f0-9]{1,4}:){0,2}[A-Fa-f0-9]{1,4})$|^(([A-Fa-f0-9]{1,4}:){5}:([A-Fa-f0-9]{1,4}:){0,1}[A-Fa-f0-9]{1,4})$|^(([A-Fa-f0-9]{1,4}:){6}:[A-Fa-f0-9]{1,4})$/";
	if(str.match(pat)){
		return true;
	}
	return false;
}

function strip_html(str) {
	return str.replace(/<([^>]+>).*<\/\1/g,"");
}

function basename(path, suffix) {
	var b = path.replace(/^.*[\/\\]/g, '');
	if(typeof(suffix) == 'string' && b.substr(b.length-suffix.length) == suffix) {
		b = b.substr(0, b.length-suffix.length);
	}
	return b;
}

function is_email(str) {
	var m=/^.+\@(\[?)[a-zA-Z0-9\-\.]+\.([a-zA-Z]{2,3}|[0-9]{1,3})(\]?)$/;
	if(str.match(m)==null) {
		return false;
	}
	return true;
}

function is_int(str) {
	var y = parseInt(str * 1);
	if(isNaN(y)) {
		return false;
	}    
	return str == y && str.toString() == y.toString(); 
}

function is_num(num) {
	if(is_int(num) && num >= 0) {
		return true;
	}
	return false;
}

function count(str, mode) {
	var key, cnt = 0;
	if( mode == 'COUNT_RECURSIVE' ) {
		mode = 1;
	}
	if( mode != 1 ) {
		mode = 0;
	}
	for(key in str) {
		cnt++;
		if(mode==1 && str[key] && (str[key].constructor === Array || str[key].constructor === Object)) {
			cnt += count(str[key], 1);
		}
	}
	return cnt;
}

function isset() {
	var a=arguments; var l=a.length; var i=0;
	if(l==0) {
		throw new Error('Empty isset');
	}
	while(i!=l) {
		if(typeof(a[i])=='undefined' || a[i]===null) { 
			return false;
		} else {
			i++;
		}
	}
	return true;
}

function is_array(str) {
	return (str instanceof Array);
}

function is_object(str) {
	if(str instanceof Array) {
		return false;
	} else {
		return (str !== null) && (typeof(str) == 'object');
	}
}

function is_string(str) {
	return (typeof(str) == 'string');
}

function is_null(str) {
	if(typeof(str)=='undefined' || str==='' || str==null) {
		return true;
	}
	return false;
}

function is_same(str1,str2) {
	if(str1===str2) {
		return true;
	}
	return false;
}

function function_exists(func) {
	if(typeof func=='string'){
		return (typeof window[func] == 'function');
	} else {
		return (func instanceof Function);
	}
}

function utf8_decode ( str_data ) {
	var tmp_arr = [], i = ac = c = c1 = c2 = 0;
	while ( i < str_data.length ) {
		c = str_data.charCodeAt(i);
		if (c < 128) {
			tmp_arr[ac++] = String.fromCharCode(c); 
			i++;
		} else if ((c > 191) && (c < 224)) {
			c2 = str_data.charCodeAt(i+1);
			tmp_arr[ac++] = String.fromCharCode(((c & 31) << 6) | (c2 & 63));
			i += 2;
		} else {
			c2 = str_data.charCodeAt(i+1);
			c3 = str_data.charCodeAt(i+2);
			tmp_arr[ac++] = String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
			i += 3;
		}
	}
	return tmp_arr.join('');
}

function utf8_encode(string) {
	string = string.replace(/\r\n/g,"\n");
	var utftext = "";
	var start, end;
 
	start = end = 0;
	for(var n = 0; n < string.length; n++) {
		var c = string.charCodeAt(n);
		var enc = null;
 
		if (c < 128) {
			end++;
		} else if((c > 127) && (c < 2048)) {
			enc = String.fromCharCode((c >> 6) | 192) + String.fromCharCode((c & 63) | 128);
		} else {
			enc = String.fromCharCode((c >> 12) | 224) + String.fromCharCode(((c >> 6) & 63) | 128) + String.fromCharCode((c & 63) | 128);
		}
		if (enc != null) {
			if (end > start) {
				utftext += string.substring(start, end);
			}
			utftext += enc;
			start = end = n+1;
		}
    	}
    
    	if (end > start) {
		utftext += string.substring(start, string.length);
	}
	return utftext;
}

function base64_decode(data) {
	var b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	var o1, o2, o3, h1, h2, h3, h4, bits, i = ac = 0, dec = "", tmp_arr = [];

	do {
		h1 = b64.indexOf(data.charAt(i++));
		h2 = b64.indexOf(data.charAt(i++));
		h3 = b64.indexOf(data.charAt(i++));
		h4 = b64.indexOf(data.charAt(i++));

		bits = h1<<18 | h2<<12 | h3<<6 | h4;

		o1 = bits>>16 & 0xff;
		o2 = bits>>8 & 0xff;
		o3 = bits & 0xff;

		if (h3 == 64) {
			tmp_arr[ac++] = String.fromCharCode(o1);
		} else if (h4 == 64) {
			tmp_arr[ac++] = String.fromCharCode(o1, o2);
		} else {
			tmp_arr[ac++] = String.fromCharCode(o1, o2, o3);
		}
	} while (i < data.length);
    
	dec = tmp_arr.join('');
	dec = utf8_decode(dec);
    
	return dec;
}

function base64_encode(data) {      
	var b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	var o1, o2, o3, h1, h2, h3, h4, bits, i = ac = 0, enc="", tmp_arr = [];
	data = utf8_encode(data);

	do {
		o1 = data.charCodeAt(i++);
		o2 = data.charCodeAt(i++);
		o3 = data.charCodeAt(i++);

		bits = o1<<16 | o2<<8 | o3;

		h1 = bits>>18 & 0x3f;
		h2 = bits>>12 & 0x3f;
		h3 = bits>>6 & 0x3f;
		h4 = bits & 0x3f;

		tmp_arr[ac++] = b64.charAt(h1) + b64.charAt(h2) + b64.charAt(h3) + b64.charAt(h4);
	} while (i < data.length);
    
	enc = tmp_arr.join('');
	    
	switch( data.length % 3 ){
		case 1:
			enc = enc.slice(0, -2) + '==';
		break;
		case 2:
			enc = enc.slice(0, -1) + '=';
		break;
	}
	return enc;
}

function time() {
	return Math.round(new Date().getTime()/1000);
}

function icon2status(str) {
	if(is_object(_ICON)) {
		var _str=basename(str);
		if(_str==basename(_ICON['_ICON_YESNO'][0])) {
			return 0;
		}
		if(_str==basename(_ICON['_ICON_YESNO'][1])) {
			return 1;
		}
		if(_str==basename(_ICON['_ICON_ONOFF'][0])) {
			return 0;
		}
		if(_str==basename(_ICON['_ICON_ONOFF'][1])) {
			return 1;
		}
		if(_str==basename(_ICON['_ICON_ENABLE'][0])) {
			return 0;
		}
		if(_str==basename(_ICON['_ICON_ENABLE'][1])) {
			return 1;
		}
	}
	return 0;
}

function js_gettext(str) {
	if(typeof(_LANG)!='undefined' && is_object(_LANG)) {
		if(!is_null(_LANG[str])) {
			return _LANG[str];
		}
	}
	return str;
}

/* jquery ext */
jQuery.trim = function(str,charlist) {
	str=str || "";
	var whitespace, l = 0;
	if(!charlist) {
		whitespace = ' \n\r\t\f\x0b\xa0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u200b\u2028\u2029\u3000';
	} else {
		whitespace = charlist.replace(/([\[\]\(\)\.\?\/\*\{\}\+\$\^\:])/g, '\$1');
	}
	l = str.length;
	for(var i = 0; i < l; i++) {
		if(whitespace.indexOf(str.charAt(i)) === -1) {
			str = str.substring(i);
			break;
		}
	}

	l = str.length;
	for(i = l - 1; i >= 0; i--) {
		if (whitespace.indexOf(str.charAt(i)) === -1) {
			str = str.substring(0, i + 1);
			break;
		}
	}
	return whitespace.indexOf(str.charAt(0)) === -1 ? str : '';
};


