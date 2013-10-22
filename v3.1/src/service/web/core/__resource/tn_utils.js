/** utils **/
function __ifidexist(pid) {
	if(document.getElementById(pid)) {
		return true;
	}
	return false;
}
function __iffixie() {
	if($.browser.msie && $.browser.version < 7) {
		return true;
	}
	return false;
}
function __pidremove(pid) {
	$(pid).remove();
}

var _progress_timer=null;
var _progress_page=null;
var _progress_req=null;
function __upload_update_start(page,req) {
	_progress_page=page;
	_progress_req=req+'&'+time();
	__tnpage.loadpage(_progress_page,_progress_req,false);
	if(_progress_page!=null && _progress_req!=null) {
		_progress_timer=setTimeout("__upload_update_start(_progress_page,_progress_req)",1000);
	}
}
function __upload_update_stop() {
	clearTimeout(_progress_timer);
	_progress_timer=null;
	_progress_page=null;
	_progress_req=null;
}

var __tnutils=function() {
	var _preload_img=function(file) {
		var img=new Image();
		img.src=file;
	};

	var _load_obj=function() {
		__tnpage.loading_setup();

		$.ajaxSetup({async: false});
		$.getScript('/__obj/js_obj.js',function() {
			if(js_obj) {
				for(var x in js_obj) {
					$.getScript(js_obj[x]);
				}
			}
		});

		$.ajaxSetup({async: false});
		$.getScript('/?js_var=obj');

		$.ajaxSetup({async: false});
		$.getScript('/__obj/img_obj.js',function() {
			if(image_cache) {
				for(var i in image_cache) {
					_preload_img(image_cache[i]);
				}
			}
		});
	};

	return {
		cache_obj: function() {
			_load_obj();
		},
		preload_img: function(file) {
			_preload_img(file);
		},
		
		tooltips: function(_text) {
			if($.trim(_text)!=='') {
				var _len=_text.length;
				if(_len > 100) {
					_len=300;
				} else {
					_len=-1;
				}
				return overlib(_text,HAUTO,VAUTO,WIDTH,_len,TEXTSIZE,'12px',CELLPAD,2,4,2,4, DELAY,200);
			}
		}
	};
}();

