var __service_depend_status={
	"sophos_av": 0,
	"clam_av": 0,
	"ads": 0,
	"anyterm": 0
};

var __login_id=null;

var __tnpage=function() {
	var _page_history=[];
	var _color_in="#FF6600";
	var _color_out="#FFFFFF";
	var _doclick=null;
	var _dohclick=null;
	var _bg_in=null;
	var _bg_out=null;
	var _bgh_in=null;
	var _bgh_out=null;

	var _load_icon=function() {
		$.ajaxSetup({async: false});
		$.getScript('/?js_var=icon');
		if(_ICON['_ICON_PAGE_LOGIN']) {
			for(var _x=0; _x < _ICON['_ICON_PAGE_LOGIN'].length; _x++) {
				__tnutils.preload_img(_ICON['_ICON_PAGE_LOGIN'][_x]);
			}
		}
		_bg_in="url("+_ICON['_ICON_MENU_ARROW'][1]+")";
		_bg_out="url("+_ICON['_ICON_MENU_ARROW'][0]+")";
		_bgh_in="url("+_ICON['_ICON_MENU_BAR'][1]+")";
		_bgh_out="url("+_ICON['_ICON_MENU_BAR'][0]+")";
	};

	var _backend_error=function() {
		_page_loading("hide");
		__tndialog.ebox(js_gettext("Unable to complete backend request. Please reload your browser."));
	};

	var _anyterm_win=function() {
		$('#webterm').hover(
			function() {
				$(this).css('cursor','pointer');
			},
			function() {
				$(this).css('cursor','default');
			}
		).click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			var LeftPosition = (screen.width) ? (screen.width-620)/2 : 0;
			var TopPosition = (screen.height) ? (screen.height-530)/2 : 0;
			var settings='width=600,height=430,top='+TopPosition+',left='+LeftPosition+',scrollbars=1,status=0,resizable=1,menubar=0';
			var index='/bfafd813d7ea65ee4db1f09d7c8ffbf4';
			var _time=Math.round(new Date().getTime()/1000);
			var win=window.open(index,'logwin'+_time,settings);
			if(!win) {
				__tndialog.ebox(js_gettext('Please allow window popup'));
				return false;
			}
			if(parseInt(navigator.appVersion) >= 4){win.window.focus();}
			return false;
		});
	};

	var _anyterm_enab=function() {
		if(__service_depend_status['anyterm']==1) {
			$('#box_title_l').css({'float':'right','left':'945px','top':'85px'}).show().html("<img id='webterm' src='"+_ICON['_ICON_TERMINAL']+"'>");
			_anyterm_win();
		} else {
			$('#box_title_l').hide().empty();
		}
	};

	var _page_loading=function(option) {
		var _option=option || "hide";
		switch(_option) {
			case 'show':
				$("#loading").show();
			break;
			case 'hide':
				$("#loading").hide("slow");
			break;
		}
	};

	var _add_history=function(pid,data,async) {
		var _data=data || null;
		var _async=async || false;
		_page_history=[pid,_data,_async];
	};

	var _load_page=function(pid,data,async) {
		var _async=async || false;
		$.ajax({
			url: "index.exh",
			data: data,
			cache: false,
			type: "POST",
			method: "POST",
			dataType: "html",
			async: _async,
			beforeSend: function() {
				_page_loading("show");
			},
			success: function(buff) {
				$(pid).html(buff);
				_page_loading('hide');
  			},
			error: function() {
				_backend_error();
			},
			complete: function() {
				_page_loading("hide");
			}
		});
	};

	var _page_redirect=function(url) {
		top.location.href=url;
		return false;
	};

	var _logout=function(msg) {
		clearInterval(_session_interval);
		_page_loading("show");
		$("#nav_box").empty();
		$("#box_title").empty();
		$("#box_title_l").empty();
		$("#dashboard").hide();
		$("#toolbox_a").hide();
		$("#toolbox_b").hide();
		$("#__aboutbox").hide();
		$(".ebox").hide();
		__tndefinitions.navclose('fast');
		__tndialog.maskhide();
		$("#content_box").html("<span id='session_logout'>"+msg+"</span>");
		setTimeout("window.location.reload(true)",2000);
		return false;
	};

	var _bartext=function(_str) {
		$("#box_title").html(_str);
		return false;
	};

	var _hmenu_reset=function() {
		$(_dohclick).css({"background-image": _bgh_out});
		_dohclick=null;
		__tndialog.clear();
	};

	var _menu_reset=function() {
		$(_doclick).css({"color": _color_out, "background-image": _bg_out});
		_doclick=null;
 		__tndialog.clear();
	};

	var _page_dashboard=function() {
		clearTimeout(_dashboard_interval);
		_hmenu_reset();
		_menu_reset();
		var data="req_data=dashboard_title";
		_load_page('#box_title',data,true);
		$("#menu").accordion("activate", 0);
		data="req_data=dashboard";
		_load_page('#content_page',data,false);
		_add_history('dashboard');
	};

	var _menu_accordion=function() {
		$("#menu").show();
		$("#menu").accordion({
			alwaysOpen: true,
			autoHeight: false,
			fillSpace: false,
			active: false,
			navigation: false,
			header: "div.menu_header"
		});

		$("#menu div.menu_header").hover(
			function() {
				$(this).css({"background-image": _bgh_in});
			},
			function() {
				if(_dohclick!==this) {
					$(this).css({"background-image": _bgh_out});
				}
			}
		).click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			clearTimeout(_dashboard_interval);
			_hmenu_reset();
			_dohclick=this;
			$(this).css({"background-image": _bgh_in});
			var _pval=$(this).attr('id');
			if(!is_null(_pval)) {
				var _parray=_pval.split('|',3);
				if(is_array(_parray)) {
					var __id=_parray[0];
					var __title='<i>'+_parray[1]+'</i>';
					var __link='req_data='+_parray[2];
					_bartext(__title);
					_anyterm_enab();
					_load_page('div#content_page',__link,false);
					_add_history('div#content_page',__link,false);
					$("#menu").accordion("activate",__id);
				}
			}
		});


		$("#menu div.menu_list").hover(
			function() {
				$(this).css({"color": _color_in});
				$(this).css({"background-image": _bg_in});
			},
			function() {
				if(_doclick!==this) {
					$(this).css({"color": _color_out, "background-image": _bg_out});
				}
			}
		).click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			clearTimeout(_dashboard_interval);
			_menu_reset();
			_doclick=this;
			$(this).css({"color": _color_in, "background-image": _bg_in});
			var _pval=$(this).attr('id');
			if(!is_null(_pval)) {
				var _parray=_pval.split('|',4);
				if(is_array(_parray)) {
					var __id=_parray[0];
					var __title='<i>'+_parray[1]+'</i> &#187; <i>'+_parray[2]+'</i>';
					var __link='req_data='+_parray[3];
					_bartext(__title);
					_anyterm_enab();
					_load_page('div#content_page',__link,false);
					_add_history('div#content_page',__link,false);
					$("#menu").accordion("activate",__id);
					__tndialog.clear();
				}
			}
		});

	};
	

	var _page_reload=function() {
		var _pid=_page_history[0] || null;
		var _data=_page_history[1] || null;
		var _async=_page_history[2] || false;

		if(_pid!==null) {
			if(_pid==="dashboard") {
				_page_dashboard(_data);
			} else {
				if(_pid!==null && _data!==null) {
					_load_page(_pid,_data,_async);
				}
			}
		}
	};

	var _menu_toolbox=function() {
		$("#toolbox_a").click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			__tndialog.abox();
		});
		$("#toolbox_b").click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			_page_reload();
		});
		$("#toolbox_c").click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			_logout(js_gettext("Session logout"));
		});
	};

	var _button_hover=function() {
		$("div.button").hover(
			function() {
				$(this).css({"background-image":"url("+_ICON['_ICON_BUTTON'][1]+")","cursor":"pointer"});
			},
			function() {
				$(this).css({"background-image":"url("+_ICON['_ICON_BUTTON'][0]+")","cursor":"default"});
			}
		);
		$("div.button_s").hover(
			function() {
				$(this).css({"background-image":"url("+_ICON['_ICON_BUTTON_SMALL'][1]+")","cursor":"pointer"});
			},
			function() {
				$(this).css({"background-image":"url("+_ICON['_ICON_BUTTON_SMALL'][0]+")","cursor":"default"});
			}
		);
	};

	var _main_page=function() {
		__tndialog.cbox(1);
		__tnutils.cache_obj();
		var data="req_data=page_content";
		_load_page('#__content',data,false);
		_add_history('#__content',data,false);
		setTimeout("__tndialog.cbox(2)",1000);
	};

	/* login */
	var _login=function() {
		var _clear=function() {
			$('#login_pass').attr({value: ''}).focus();
			if($.browser.msie) {
				document.forms[0].userpass.value='';
				document.forms[0].userpass.focus();
			}
		};
		var data="req_data=page_login";
		_load_page('#__content',data,false);
		_clear();
		$('#login_pass').click(function() {
			_clear();
		});

		var options = { 
			url: 'index.exh',
			type: 'POST',
			method: 'POST',
			dataType: 'json',
			clearForm: false,
			resetForm: false,
			cache: false,
			beforeSubmit: function() {
				var pass=$("#login_pass").val();
				if($.trim(pass)==='') {
					return false;
				}
			},
			success: function(data) {
				if(is_object(data)) {
					if(data.logged=='OK') {
						_main_page();
					} else {
						_page_loading("hide");
						__tndialog.ebox(data.msg);
					}
				}
			},
			beforeSend: function() {
				_page_loading("show");
			},
			error: function() {
				_backend_error();
				_page_loading("hide");
				_clear();
			},
			complete: function() {
				_page_loading("hide");
			}
		};
		$("#login_submit").click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			$('#login_form').ajaxSubmit(options);
			return false;
		});
		$('#login_form').submit(function() {
			$(this).ajaxSubmit(options);
			return false;
		});
	};

	var _session_ping=function() {
		$.ajax({
			url: "index.exh",
			data: "req_data=session_check",
			cache: false,
			type: "POST",
			dataType: "text",
			async: true,
			success: function(buff) {
				if(buff==="expired") {
					_logout('Session expired');
				}
  			},
			error: function(proto, status, error) {
				clearInterval(_session_interval);
			}
		});
	};

	var _loading_setup=function() {
		$.ajaxSetup({
			beforeSend: function() {
				_page_loading('show');
			},
			success: function() {
				_page_loading('hide');
			},
			error: function() {
				_backend_error();
			},
			complete: function() {
				_page_loading('hide');
			}
		});
	};

	var _data_update=function(data,func) {
		$.ajax({
			url: "index.exh",
			data: data,
			cache: false,
			type: "POST",
			dataType: "json",
			async: false,
			beforeSend: function() {
				_page_loading("show");
			},
			success: function(pdata) {
				_page_loading('hide');
				if(is_object(pdata)) {
					if(pdata.status=='OK') {
						if(!is_null(pdata.msg)) {
							__tndialog.sbox(pdata.msg);
						}
						if(func && function_exists(func)) {
							func(pdata);
						}
					} else {
						if(!is_null(pdata.msg)) {
							__tndialog.ebox(pdata.msg);
						}
					}
				}
  			},
			error: function() {
				_backend_error();
			},
			complete: function() {
				_page_loading("hide");
			}
		});
	};

	return {
		init: function() {
			$(document).ready(function() {
				_load_icon();
				_login();
			});
		},
		accordion: function() {
			$(document).ready(function() {
				_menu_accordion();
			});
		},

		toolbox: function() {
			$(document).ready(function() {
				_menu_toolbox();
			});
		},
		logout: function(msg) {
			_logout(msg);
		},
		sessionping: function() {
			$(document).ready(function() {
				_session_ping();
			});
		},
		dashboard: function() {
			$(document).ready(function() {
				_page_dashboard();
			});
		},
		redirect: function(url) {
			_page_redirect(url);
		},
		loadpage: function(pid,data,async) {
			_load_page(pid,data,async);
			_add_history(pid,data,async);
		},
		button_hover: function() {
			$(document).ready(function() {
				_button_hover();
			});
		},
		loading_setup: function() {
			$(document).ready(function() {
				_loading_setup();
			});
		},
		loading: function(opt) {
			$(document).ready(function() {
				_page_loading(opt);
			});
		},
		backend_error: function(opt) {
			$(document).ready(function() {
				_backend_error();
			});
		},
		data_update: function(data,func) {
			$(document).ready(function() {
				_data_update(data,func);
			});
		},
		is_admin: function() {
			if(!is_null(__login_id) && is_same(__login_id,'admin')) {
				return true;
			}
			__tndialog.ebox(js_gettext("Permission denied"));
			return false;
		},
		anyterm: function(_opt) {
			var opt=_opt || 0;
			if(opt==0) {
				_anyterm_enab();
			} else {
				_anyterm_win();
			}
		},
		reload: function() {
			 _page_reload();
		},
		error: function() {
			_backend_error();
		}
	};
}();
