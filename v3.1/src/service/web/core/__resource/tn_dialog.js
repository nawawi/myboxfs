/** dialog box **/
var __sboxtimer=null;
var __eboxwin=0;
var __aboxwin=0;
var __iboxwin=0;
var __tndialog=function() {

	var _maskbox_show=function(zindex) {
		var _zindex=zindex || 500;
		var _mask='box_mask';
		var _jqid='div#'+_mask;

		_height=$(document).height();
		_width=$(document).width();

		var _overlay={
			'opacity':'.4'
		};

		if(__iffixie()==true) {
			_overlay={
				'filter':'alpha(opacity=40)'
			};
		}

		_height=_height+"px";
		_width=_width+"px";

		var _css={
			'display': 'none',
			'z-index': _zindex,
			'position': 'absolute',
			'top': '0px',
			'left': '0px',
			'height': _height,
			'width': _width,
			'border': 'none',
			'overflow': 'hidden',
			'background-color': '#000000'
		};

		if(!__ifidexist(_mask)) {
			mask_win=document.createElement('div');
			mask_win.id=_mask;
			$(mask_win).hide();
			document.body.appendChild(mask_win);
		}
		$(_jqid).css(_css).css(_overlay).show();
	};

	var _maskbox_hide=function(opt) {
		var _opt=opt || null;
		if(_opt!=null) {
			$("div#box_mask").hide(_opt);
		} else {
			$("div#box_mask").hide();
		}
	};

	var _gbox=function(pid,title,top,left,width,content,modal) {
		var _modal=modal || false;
		var _width=width || "298px";		
		var _pid=pid;
		var _jpid='div#'+_pid;
		if(_modal==true) {
			_maskbox_show(198);
		}
		__pidremove(_jpid);
		gbox_win=document.createElement('div');
		gbox_win.id=_pid;
		$(gbox_win).hide();
		document.body.appendChild(gbox_win);
		$(_jpid).addClass('gbox').css({'top':top,'left':left,'width': _width}).html(
			"<div id='gbox_title'>"+
				"<div id='gbox_title_l'>"+title+"</div>"+
				"<div id='gbox_title_r' onclick=\"$('"+_jpid+"').remove();\"></div>"+
			"</div>"+
			"<div id='gbox_content'>"+content+"</div>"
		).show();
	};

	var _ebox=function(msg) {
		var _pid='f7b3ffa'+time();
		var _jpid='div#'+_pid;

		_maskbox_show();
		__eboxwin=1;
		__pidremove(_jpid);

		var _css={
			'top': '200px',
			'left': '400px'
		};

		ebox_win=document.createElement('div');
		ebox_win.id=_pid;
		$(ebox_win).hide();
		document.body.appendChild(ebox_win);
		$(_jpid).addClass('ebox').css(_css).html(
			"<div id='ebox_title'>"+
				"<div id='ebox_title_l'>"+js_gettext("Error:")+"</div>"+
				"<div id='ebox_title_r'></div>"+
			"</div>"+
			"<div id='ebox_content'>"+
				"<table style='width: 100%; border-spacing: 0px; border-collapse: collapse;'>"+
				"<tr><td style='text-align: left; padding: 2px; padding-top: 3px;'><img style='float: left; margin-right: 10px; margin-bottom: 10px;' src='"+_ICON['_ICON_ERROR']+"'>"+msg+"</td></tr>"+
				"<tr><td style='border-spacing: 0px; border-collapse: collapse;text-align: center;padding-top: 3px;'><hr class='ebox_hr'></td></tr>"+
				"<tr><td style='padding: 3px; padding-top: 5px;'><center><div class='button' id='bt_ebox' style='width: 80px; font-weight: bold;'>Ok</div></center></td></tr>"+
				"</table>"+
			"</div>"+
			"<script type='text/javascript'>"+
				"var _ehide=function() {"+
					"$('"+_jpid+"').remove();"+
					"__tndialog.maskhide();"+
					"__eboxwin=0;"+
				"};"+
				"$('"+_jpid+" div#ebox_title_r').click(function(evt) {"+
					"var e=evt || window.event;"+
					"e.preventDefault();"+
					"_ehide();"+
				"});"+
				"$('"+_jpid+" div#bt_ebox').click(function(evt) {"+
					"var e=evt || window.event;"+
					"e.preventDefault();"+
					"_ehide();"+
				"});"+
				"$(document).keyup(function(evt) {"+
					"var e=evt || window.event;"+
					"var knum = document.all ? window.event.keyCode : e.which;"+
					"if(knum===13) {"+
						"_ehide();"+
					"}"+
				"});"+
			"</script>"
		).show();
		__tnpage.button_hover();
	};

	var _ibox=function(msg) {
		var _pid='ea50d80c63'+time();
		var _jpid='div#'+_pid;

		_maskbox_show();
		__iboxwin=1;
		__pidremove(_jpid);

		var _css={
			'top': '200px',
			'left': '400px',
			'z-index': '600',
			'width': '250px'
		};

		ibox_win=document.createElement('div');
		ibox_win.id=_pid;
		$(ibox_win).hide();
		document.body.appendChild(ibox_win);
		$(_jpid).addClass('ibox').css(_css).html(
			"<div id='ibox_title'>"+
				"<div id='ibox_title_l'>"+js_gettext("Information:")+"</div>"+
				"<div id='ibox_title_r'></div>"+
			"</div>"+
			"<div id='ibox_content'>"+
				"<table style='width: 100%; border-spacing: 0px; border-collapse: collapse;'>"+
				"<tr><td style='text-align: left; padding: 2px; padding-top: 3px;'><img style='float: left; margin-right: 10px; margin-bottom: 10px;' src='"+_ICON['_ICON_INFO']+"'>"+msg+"</td></tr>"+
				"<tr><td style='border-spacing: 0px; border-collapse: collapse;text-align: center;padding-top: 3px;'><hr class='ibox_hr'></td></tr>"+
				"<tr><td style='padding: 3px; padding-top: 5px;'><center><div class='button' id='bt_ibox' style='width: 80px; font-weight: bold;'>Ok</div></center></td></tr>"+
				"</table>"+
			"</div>"+
			"<script type='text/javascript'>"+
				"var _ehide=function() {"+
					"$('"+_jpid+"').remove();"+
					"__tndialog.maskhide();"+
					"__iboxwin=0;"+
				"};"+
				"$('"+_jpid+" div#ibox_title_r').click(function(evt) {"+
					"var e=evt || window.event;"+
					"e.preventDefault();"+
					"_ehide();"+
				"});"+
				"$('"+_jpid+" div#bt_ibox').click(function(evt) {"+
					"var e=evt || window.event;"+
					"e.preventDefault();"+
					"_ehide();"+
				"});"+
				"$(document).keyup(function(evt) {"+
					"var e=evt || window.event;"+
					"var knum = document.all ? window.event.keyCode : e.which;"+
					"if(knum===13) {"+
						"_ehide();"+
					"}"+
				"});"+
			"</script>"
		).show();
		__tnpage.button_hover();
	};

	var _ecbox=function(msg,action,width) {
		var _pid='4c0a50f73c'+time();
		var _jpid='div#'+_pid;

		_maskbox_show();
		__pidremove(_jpid);

		var _css={
			'top': '200px',
			'left': '400px'
		};

		ecbox_win=document.createElement('div');
		ecbox_win.id=_pid;
		$(ecbox_win).hide();
		document.body.appendChild(ecbox_win);
		$(_jpid).addClass('ecbox').css(_css).html(
			"<div id='ecbox_title'>"+
				"<div id='ecbox_title_l'>Confirmation:</div>"+
				"<div id='ecbox_title_r'></div>"+
			"</div>"+
			"<div id='ecbox_content'>"+
				"<table style='width: 100%; border-spacing: 0px; border-collapse: collapse;'>"+
				"<tr><td colspan='2' style='text-align: left; padding: 2px; padding-top: 3px;'><img style='float: left;margin-right: 10px; margin-bottom: 10px;' src='"+_ICON['_ICON_WARNING']+"'>"+msg+"</td></tr>"+
				"<tr><td colspan='2' style='border-spacing: 0px; border-collapse: collapse;text-align: center;padding-top: 3px;'><hr class='ecbox_hr'></td></tr>"+
				"<tr><td style='padding: 3px; padding-top: 5px;'><center><div class='button' id='bt_ecbox1' style='width: 80px; font-weight: bold;'>Cancel</div></center></td>"+
				"<td style='padding: 3px; padding-top: 5px;'><center><div class='button' id='bt_ecbox2' style='width: 80px; font-weight: bold;'>Ok</div></center></td>"+
				"</tr>"+
				"</table>"+
			"</div>"+
			"<script type='text/javascript'>"+
				"var _ehide=function() {"+
					"$('"+_jpid+"').remove();"+
					"__tndialog.maskhide();"+
				"};"+
				"$('"+_jpid+" div#ecbox_title_r').click(function(evt) {"+
					"var e=evt || window.event;"+
					"e.preventDefault();"+
					"_ehide();"+
				"});"+
				"$('"+_jpid+" div#bt_ecbox1').click(function(evt) {"+
					"var e=evt || window.event;"+
					"e.preventDefault();"+
					"_ehide();"+
				"});"+
				"var action="+action+";"+
                                "$('"+_jpid+" div#bt_ecbox2').click(function(evt) {"+
                                        "var e=evt || window.event;"+
                                        "e.preventDefault();"+
                                        "_ehide();"+
					"if(action && function_exists(action)) {"+
                                        	"action();"+
					"}"+
                                "});"+
			"</script>"
		).show();
		__tnpage.button_hover();
	};


	var _abox=function() {
		var _pid='c3ce1a5b23ddae3d8fd7340cff6bf1a4';
		var _jpid='div#'+_pid;

		_maskbox_show();
		__aboxwin=1;
		var _css={
			'top': '200px',
			'left': '400px',
			'border': '2px solid #4f6d81',
			'display': 'none',
			'z-index': '600',
			'position': 'absolute',
			'width': '350px',
			'background-color': '#ffffff'
		};

		if(!__ifidexist(_pid)) {
			abox_win=document.createElement('div');
			abox_win.id=_pid;
			$(abox_win).hide();
			document.body.appendChild(abox_win);
			__tnpage.loading_setup();
			$.ajaxSetup({
				async:false,
				error: function() {
					__pidremove(_pid);
					_maskbox_hide();
					__aboxwin=0;
				}
			});
			$(_jpid).css(_css).load('index.exh',{'req_data':'p_about'},function() {
				$(this).show('slow');
				__tnpage.button_hover();
				$(this).click(function() {
					$(_jpid).hide('slow');
					_maskbox_hide('slow');
					__aboxwin=0;
				});
			});
		} else {
			$(_jpid).css(_css).show('slow').click(function() {
				$(this).hide('slow');
				_maskbox_hide('slow');
				__aboxwin=0;
			});
		}
		$(document).keyup(function(evt) {
			var e=evt || window.event;
			var knum = document.all ? window.event.keyCode : e.which;
			if(knum===13) {
				$(_jpid).hide('slow');
				_maskbox_hide('slow');
				__aboxwin=0;
			}
		});
	};

	var _cbox=function(opt) {
		var _pid='e1ad5fb99b67f85c6772fa92b4de948a';
		var _jpid='div#'+_pid;

		var _overlay={
			'opacity':'.8'
		};

		if(__iffixie()==true) {
			_overlay={
				'filter':'alpha(opacity=80)'
			};
		}

		if(opt==1) {
			var _css={
				'top': '200px',
				'left': '400px',
				'border': '1px solid #ff6600',
				'display': 'none',
				'z-index': '600',
				'position': 'absolute',
				'padding-left': '20px',
				'padding-right': '20px',
				'padding-top': '10px',
				'padding-bottom': '10px',
				'font-weight': 'bold',
				'font-size': '12px',
				'color': '#000000',
				'background-color': '#ffffff',
				'cursor': 'pointer'
			};		
			if(!__ifidexist(_pid)) {
				cbox_win=document.createElement('div');
				cbox_win.id=_pid;
				$(cbox_win).hide();
				document.body.appendChild(cbox_win);
				$(_jpid).css(_css).css(_overlay).html("<img src='"+_ICON['_ICON_SPINNER']+"'> "+js_gettext("Reading data. Please wait..")).show();
			} else {
				$(_jpid).show();
			}
		} else {
			$(_jpid).hide();
		}
	};

	var _sbox=function(msg,opt) {
		var _pid='s8e950897f09fa3b70df381649131e2a5';
		var _jpid='div#'+_pid;

		if(opt===1) {
			_maskbox_show();
			var _css={
				'top': '200px',
				'left': '400px',
				'border': '1px solid #924949',
				'display': 'none',
				'z-index': '600',
				'position': 'absolute',
				'padding-left': '20px',
				'padding-right': '20px',
				'padding-top': '10px',
				'padding-bottom': '10px',
				'font-weight': 'bold',
				'font-size': '14px',
				'color': '#000000',
				'background-color': '#ffffff',
				'cursor': 'default'
			};
			if(!__ifidexist(_pid)) {
				sbox_win=document.createElement('div');
				sbox_win.id=_pid;
				$(sbox_win).hide();
				document.body.appendChild(sbox_win);
				$(_jpid).css(_css).html(msg).show();
			} else {
				$(_jpid).css(_css).html(msg).show();
			}

			$(_jpid).mouseover(function() {
				$(_jpid).hide();
				clearTimeout(__sboxtimer);
				_maskbox_hide();
			});
			$('input').click(function() {
				$(_jpid).hide();
				clearTimeout(__sboxtimer);
				_maskbox_hide();
			});
			__sboxtimer=setTimeout('__tndialog.sbox_hide()',3000);
		} else {
			$(_jpid).hide();
			clearTimeout(__sboxtimer);
			_maskbox_hide();
		}
	};

	return {
		ebox: function(msg) {
			_sbox('',2);
			_ebox(msg);
		},
		ibox: function(msg) {
			_sbox('',2);
			_ibox(msg);
		}, 
		abox: function() {
			_sbox('',2);
			_abox();
		},
		pop: function(msg) {
			alert(msg);
			return false;
		},
		gbox: function(pid,title,top,left,width,content,modal) {
			modal=modal || false;
			width=width || "298px";
			_gbox(pid,title,top,left,width,content,modal);
		},
		maskshow: function(zindex) {
			_maskbox_show(zindex);
		},
		maskhide: function(opt) {
			var opt=opt || null;
			_maskbox_hide(opt);
		},
		cbox: function(opt) {
			_cbox(opt);
		},
		confirm_box: function(msg,action) {
			_ecbox(msg,action);
		},
		sbox: function(msg) {
			if(__eboxwin==0 && __aboxwin==0 && __iboxwin==0) {
				_sbox(msg,1);
			}
		},
		sbox_hide: function() {
			_sbox('',2);
		},
		clear: function() {
			$('div.defi_box').remove();
			$('div.gbox').remove();
			$('div.ebox').remove();
			$('div.ecbox').remove();
			$('div.ibox').remove();
		}
	};
}();
