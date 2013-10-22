/* networks */
var __deft_n={
	"all":"All",
	"host":"Hosts",
	"network":"Networks",
	"group":"Groups",
	"dnshost":"DNS Hosts",
	"hostrange":"Hosts Range",
	"hostmac":"MAC Addresses",
	"interface_addresses":"Interface addresses",
	"interface_networks":"Interface networks",
	"interface_broadcast_addresses":"Interface broadcast addresses",
	"builtin":"Builtin"
};

var __defb_n={
	"host":"Host",
	"dnshost":"DNS Host",
	"hostrange":"Hosts Range",
	"hostmac":"MAC Addresses",
	"network":"Network",
	"group":"Group"
};

var __defa_n={
	"all":"#dnd_host,#dnd_dnshost,#dnd_network,#dnd_hostmac,#dnd_hostrange,#dnd_builtin,#dnd_group,#dnd_interface_addresses,#dnd_interface_networks,#dnd_interface_broadcast_addresses",
	"host":"#dnd_host",
	"network":"#dnd_network",
	"group":"#dnd_group",
	"dnshost":"#dnd_dnshost",
	"hostrange":"#dnd_hostrange",
	"hostmac":"#dnd_hostmac",
	"interface_addresses":"#dnd_interface_addresses",
	"interface_networks":"#dnd_interface_networks",
	"interface_broadcast_addresses":"#dnd_interface_broadcast_addresses",
	"builtin":"#dnd_builtin"
};

/* services */
var __deft_s={
	"all":"All",
	"tcp":"TCP",
	"udp":"UDP",
	"tcpudp":"TCP/UDP",
	"icmp":"ICMP",
	"ah":"AH",
	"esp":"ESP",
	"ip":"IP",
	"group":"Groups"
};

var __defb_s={
	"tcp":"TCP",
	"udp":"UDP",
	"tcpudp":"TCP/UDP",
	"icmp":"ICMP",
	"ah":"AH",
	"esp":"ESP",
	"ip":"IP",
	"group":"Groups"
};

var __defa_s={
	"all":"#dnd_tcp,#dnd_udp,#dnd_tcpudp,#dnd_icmp,#dnd_ah,#dnd_esp,#dnd_ip,#dnd_group,#dnd_builtin",
	"tcp":"#dnd_tcp",
	"udp":"#dnd_udp",
	"tcpudp":"#dnd_tcpudp",
	"icmp":"#dnd_icmp",
	"ah":"#dnd_ah",
	"esp":"#dnd_esp",
	"ip":"#dnd_ip",
	"group":"#dnd_group",
	"builtin":"#dnd_builtin"
};

/* user/group */
var __deft_u={
	"all":"All",
	"user":"Users",
	"group":"Groups"
};

var __defb_u={
	"user":"Users",
	"group":"Groups"
};

var __defa_u={
	"all":"#dnd_user,#dnd_group",
	"user":"#dnd_user",
	"group":"#dnd_group"
};

/* time events */
var __deft_t={
	"all":"All",
	"single":"Single",
	"recurring":"Recurring"
};

var __defb_t={
	"single":"Single",
	"recurring":"Recurring"
};

var __defa_t={
	"all":"#dnd_single,#dnd_recurring",
	"single":"#dnd_single",
	"recurring":"#dnd_recurring"
};

/* interfaces */
var __deft_i={
	"all":"All",
	"static":"Ethernet Standard (static)",
	"vlan":"Ethernet VLAN"
};

/* default toogle */
var __navtoggle={
	"networks":false,
	"services":false,
	"interfaces":false,
	"users":false,
	"timeevents":false
};

var __tndefinitions=function() {
	var _navcontent=function(section,_default) {
		var _default=_default || "all";
		var _page="";
		var _sarray={};
		if(section==='networks' && (__deft_n)) {
			_sarray=__deft_n;
		} else if(section==='services' && (__deft_s)) {
			_sarray=__deft_s;
		} else if(section==='users' && (__deft_u)) {
			_sarray=__deft_u;
		} else if(section==='timeevents' && (__deft_t)) {
			_sarray=__deft_t;
		} else if(section==='interfaces' && (__deft_i)) {
			_sarray=__deft_i;
		} else {
			_sarray={};
		}
		if(_sarray!=={}) {
			for(var _x in _sarray) {
				var _v=_x;
				var _t=_sarray[_x];
				if(_v===_default) {
					_page +="<option value='"+_v+"' selected>"+_t+"</option>";
				} else {
					_page +="<option value='"+_v+"'>"+_t+"</option>";
				}
			}
		}
		if(_page!=="") {
			__tnpage.loading_setup();
			var _data={"definitions_nav":section};
			$("div#def_box_content").html(js_gettext("Loading data.."));
			$("div#def_box_content").load("index.exh",_data, function() {
				$("div#def_box_content_t select").html(_page);
				$("div#def_box_content_t input").attr({value: '', autocomplete: 'off'});
				$("div.dnd_drag").draggable({
					revert: "invalid",
					helper: function() {
 						return $(this).clone().css({'background-color':'#ffffff','border':'1px solid #000000','height':'16px'}).css('width', this.offsetWidth)[0];
					},
					zIndex: 300,
					scroll: false,
					drag: function() {
						$(this).css({"background-color": "#d8e4f1"});
					},
					stop: function() {
						$(this).filter(function() {
							_rowcolor('div.dnd_drag');
						});
					}
				});

				$("div.dnd_drag").hover(
					function() {
						this.oldcolor=$(this).css('background-color');
						$(this).css({"background-color": "#d8e4f1"});
					},
					function() {
						$(this).css({"background-color": this.oldcolor});
					}
				);
				$("div#def_box").draggable({handle:'div#def_box_title'});

				$("div#def_box_content_t select").change(function() {
					var _mid=$(this).attr('value');
					if(_mid=='all') {
						$("div.dnd_drag").show();
					} else {
						$("div.dnd_drag").hide().filter("#dnd_"+_mid).show();
					}
					_rowcolor('div.dnd_drag','','',true);
				});

				$("div#def_box_content_t input").keyup(function(event) {
					var _opt=$("div#def_box_content_t select option:selected").attr("value");
					var _oid="div#dnd_"+_opt;
					var _str=$(this).val();
					if(_str.length===0) {
						if(_opt==="all") {
							$("div.dnd_drag").show();
						} else {
							$("div.dnd_drag").hide().filter(_oid).show();
						}
					} else {
						if(_opt==="all") {
							$("div.dnd_drag").hide().each(function(i) {
								var _p=$(this).text();
								var _regex=new RegExp(_str, "i");
								if(_regex.exec(_p)) {
									$(this).show();
								}
							});
						} else {
							$("div.dnd_drag").hide().each(function(i) {
								var _p=$(this).text();
								var _regex=new RegExp(_str, "i");
								if(_regex.exec(_p)) {
									$(this).filter(_oid).show();
								}
							});
						}
					}
					_rowcolor('div.dnd_drag','','',true);
				});
				_rowcolor('div.dnd_drag');
			});
		}
	};

	var _navclose=function() {
		for(var x in __navtoggle) {
			__navtoggle[x]=false;
		}
		$('div#def_box').hide();
	};
	
	var _navopen=function(section) {
		if(!document.getElementById('def_box')) {
			_navbox_win=document.createElement('div');
			_navbox_win.id='def_box';
			$(_navbox_win).hide().html(
				"<div id='def_box_title'>"+
					"<div id='def_box_title_l'></div>"+
					"<div id='def_box_title_r'></div>"+
				"</div>"+
				"<div id='def_box_content_t'>"+
					"<table style='width: 100%; border-spacing: 0px; border-collapse: collapse;'><tr>"+
					"<td><select class='tbox_s' style='width: 89px;' size='1'></select></td>"+
					"<td><input type='text' class='tbox_t tbox_it' /></td>"+
					"</tr></table>"+
				"</div>"+
				"<div id='def_box_content'></div>"
			);
			document.body.appendChild(_navbox_win);
		}
			
		$('div#def_box_title_r').click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			__tndefinitions.navclose();
		});

		var __title=section;
		if(__title==='timeevents') {
			__title='time events';
		}
		$('div#def_box_title_l').html(ucwords(__title));
		if(__navtoggle[section]==true) {
			_navclose();
		} else if(__navtoggle[section]==false) {
			_navcontent(section);
			// css: see -> div#def_box
			$('div#def_box').css({"top":"115px","left":"2px"}).show();
			__navtoggle[section]=true;
			for(var x in __navtoggle) {
				if(x!==section) {
					__navtoggle[x]=false;
				}
			}
		}
	};

	var _defclose=function(pid) {
		$(pid).remove();
	};

	var _defopen_group_box=function(dataval) {
		var _page="<div class='table_fixed' style='width: 153px;'>"+
				"<div id='tbox_acl' style='width: 153px;height: 150px;'>"+
				"<div id='tbox_acl_title'>"+
				"<div id='tbox_acl_title_t'>"+js_gettext("Members")+"</div>"+
				"<div id='tbox_acl_title_l'></div>"+
			  	"</div>"+
				"<div id='tbox_acl_content' style='width: 151px;height: 137px;'>"+
				"<table class='tbox_acl_tb'><tbody>"+
				"<tr><td class='tbox_acl_td_it'><input type='text' class='tbox_acl_it'></td></tr>"+
				"</tbody>"+
				"</table>"+
				"</div>"+
			   	"</div><br />"+
			  "</div>";
		return _page;
	};

	var _defopen=function(pid,section,top,left,accept,dataval) {
		var _pid=pid;
		var _jpid='div#'+_pid;
		var __title=section;
		if(__title==='timeevents') {
			__title='time events';
		}
		dataval=dataval || {};
		if(!is_object(dataval)) {
			dataval={};
		}
		_defclose(_pid);
		defbox_win=document.createElement('div');
		defbox_win.id=_pid;
		$(defbox_win).hide();
		$(defbox_win).addClass("defi_box");
		document.body.appendChild(defbox_win);

		$(_jpid).css({"top":top,"left":left,"width":"320px"}).html(
			"<div id='defi_box_title'>"+
				"<div id='defi_box_title_l'>"+ucwords(__title)+"</div>"+
				"<div id='defi_box_title_r' onclick=\"__tndefinitions.defclose('"+_jpid+"');return false;\"></div>"+
			"</div>"+
			"<div id='defi_box_content'></div>"
		);

		$(_jpid).draggable({handle:'div#defi_box_title'});

		if(section==='networks') {
			var _accept=accept.replace(/#dnd_/g, '');
			var _pp=_accept.split(",");
			var _page="";
			if(dataval.__DATA_TYPE && dataval.__DATA_TYPE=="hostrange" && dataval.ADDR) {
				var _taddr=dataval.ADDR;
				var _parray=_taddr.split('-',2)
				if(is_array(_parray)) {
					dataval.ADDR1=_parray[0];
					dataval.ADDR2=_parray[1];
				}
			}
			_page +="<center><table class='tbox_t' style='width: 320px;'>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Name")+"</td><td class='tboxi_tdr'><input id='name' ";
			if(dataval.NAME) {
				_page +="value='"+dataval.NAME+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Type")+"</td><td class='tboxi_tdr'><select class='tbox_s' id='type' size='1'>";
			if(__defb_n) {
				for(var _x in __defb_n) {
					var _v=_x;
					var _t=__defb_n[_x];
					if(_pp.length > 0) {
						for(var _i in _pp) {
							if(_v===_pp[_i]) {
								_page +="<option value='"+_v+"'>"+_t+"</option>";
							}
						}
					}
				}
			}
			_page +="</select></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Address")+"</td><td class='tboxi_tdr'><input id='address' ";
			if(dataval.ADDR) {
				_page +="value='"+dataval.ADDR+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Interface")+"</td><td class='tboxi_tdr'>";
			_page +="<select class='tbox_s' id='interface' size='1'>";
			_page +="</select>";
			_page +="</td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Netmask")+"</td><td class='tboxi_tdr'>";
			_page +="<select class='tbox_s' id='netmask' size='1'>";
			if(_classip_array) {
				for(var _x in _classip_array) {
					var _v=_x;
					var _t=_classip_array[_x];
					if(dataval.MASK && dataval.MASK==_t) {
						_page +="<option value='"+_t+"' selected>/"+_v+" ("+_t+")</option>";
					} else if(!dataval.MASK && _v==='24') {
						_page +="<option value='"+_t+"' selected>/"+_v+" ("+_t+")</option>";
					} else {
						_page +="<option value='"+_t+"'>/"+_v+" ("+_t+")</option>";
					}
				}
			}
			_page +="</select>";
			_page +="</td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Range start")+"</td><td class='tboxi_tdr'><input id='hrange1' ";
			if(dataval.ADDR1) {
				_page +="value='"+dataval.ADDR1+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Range end")+"</td><td class='tboxi_tdr'><input id='hrange2' ";
			if(dataval.ADDR2) {
				_page +="value='"+dataval.ADDR2+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Hostname")+"</td><td class='tboxi_tdr'><input id='hostname' ";
			if(dataval.HOSTNAME) {
				_page +="value='"+dataval.HOSTNAME+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("MAC Address")+"</td><td class='tboxi_tdr'><input id='macaddr' ";
			if(dataval.ADDR) {
				_page +="value='"+dataval.ADDR+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>&nbsp;</td><td class='tboxi_tdr'>";
			_page +=_defopen_group_box(dataval);
			_page +="</td></tr>";

			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Description")+"</td><td class='tboxi_tdr'><input id='note' ";
			if(dataval.NOTE) {
				_page +="value='"+dataval.NOTE+"' ";
			}
			_page +="type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td colspan='2' style='text-align: center;'><hr class='tboxi_hr' /></td></tr>";
			_page +="<tr><td colspan='2' align='center'><table style='width: 80%;'>";
			_page +="<tr><td><div class='button' id='button_save'>Save</div>";
			_page +="</td><td><div class='button' id='button_cancel'>Cancel</div></td></tr>";
			_page +="</table></td></tr>";
			_page +="</table></center>";
			_page +="";
			$(_jpid+" div#defi_box_content").html(_page);
			$(_jpid+" div#defi_box_content select[id=interface]").load("index.exh",{"definitions_nav":"interface_select"},function() {
				if(dataval.DNAME) {
					$(_jpid+" div#defi_box_content select[id=interface] option[value="+dataval.DNAME).filter(function() {
						$(this).attr('selected','selected');
					});
				}
			});

			__tnpage.button_hover();
			
			var _default=$(_jpid+" select[id=type] option:selected").attr("value");
			var _csel=function(opt) {
				$(_jpid+" table.tbox_t tr").hide().each(function(index) {
					if(index <= 1 || index >=10 ) {
						$(this).show();
					}
					if(opt==='host') {
						if(index==2 || index==3) {
							$(this).show();
						}
					} else if(opt=='network') {
						if(index==2 || index==3 || index==4) {
							$(this).show();
						}
					} else if(opt=='dnshost') {
						if(index==3 || index==7) {
							$(this).show();
						}
					} else if(opt=='hostrange') {
						if(index==3 || index==5 || index==6) {
							$(this).show();
						}
					} else if(opt=='hostmac') {
						if(index==3 || index==8) {
							$(this).show();
						}
					} else if(opt=='group') {
						if(index==9) {
							$(this).show();
						}
					}
				});
			};
			
			_csel(_default);		
			$(_jpid).show();

		} else if(section==='services') {
			var _accept=accept.replace(/#dnd_/g, '');
			var _pp=_accept.split(",");
			var _page="";
			_page +="<center><table class='tbox_t' style='width: 350px;'>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Name")+"</td><td class='tboxi_tdr'><input id='name' type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Type of Definition")+"</td><td class='tboxi_tdr'><select class='tbox_s' id='type' size='1'>";
			if(__defb_s) {
				for(var _x in __defb_s) {
					var _v=_x;
					var _t=__defb_s[_x];
					if(_pp.length > 0) {
						for(var _i in _pp) {
							if(_v===_pp[_i]) {
								_page +="<option value='"+_v+"'>"+_t+"</option>";
							}
						}
					}
				}
			}
			_page +="</select></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Destination port")+"</td><td class='tboxi_tdr'><input id='dport' type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Source port")+"</td><td class='tboxi_tdr'><input id='sport' type='text' class='tbox_i' value='1:65535' /></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Type/Code")+"</td><td class='tboxi_tdr'>";
			_page +="<select class='tbox_s' id='icmptype' size='1'>";
			if(_icmptype_array) {
				for(var _x in _icmptype_array) {
					var _v=_x;
					var _t=_icmptype_array[_x];
					_page +="<option value='"+_v+"' selected>"+_t+"</option>";
				}
			}
			_page +="</select>";
			_page +="</td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("SPI")+"</td><td class='tboxi_tdr'><input id='spi' type='text' class='tbox_i' value='256:4294967295' /></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Protocol number")+"</td><td class='tboxi_tdr'><input id='protocol' type='text' class='tbox_i' value='' /></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>&nbsp;</td><td class='tboxi_tdr'>";
			_page +=_defopen_group_box(dataval);
			_page +="</td></tr>";

			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Description")+"</td><td class='tboxi_tdr'><input id='note' type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td colspan='2' style='text-align: center;'><hr class='tboxi_hr' /></td></tr>";
			_page +="<tr><td colspan='2' align='center'><table style='width: 80%;'>";
			_page +="<tr><td><div class='button' id='button_save'>Save</div>";
			_page +="</td><td><div class='button' id='button_cancel'>Cancel</div></td></tr>";
			_page +="</table></td></tr>";
			_page +="</table></center>";
			_page +="";
			$(_jpid+" div#defi_box_content").html(_page);
			__tnpage.button_hover();
			
			var _default=$(_jpid+" select[id=type] option:selected").attr("value");
			var _csel=function(opt) {
				$(_jpid+" table.tbox_t tr").hide().each(function(index) {
					if(index <= 1 || index >=8 ) {
						$(this).show();
					}
					if(opt==='tcp' || opt=='udp' || opt=='tcpudp') {
						if(index==2 || index==3) {
							$(this).show();
						}
					} else if(opt=='icmp') {
						if(index==4) {
							$(this).show();
						}
					} else if(opt=='ip') {
						if(index==6) {
							$(this).show();
						}
					} else if(opt=='esp' || opt=='ah') {
						if(index==5) {
							$(this).show();
						}
					} else if(opt=='group') {
						if(index==7) {
							$(this).show();
						}
					}
				});
			};
			
			_csel(_default);		
			$(_jpid).show();

		} else if(section==='timeevents') {
			var _accept=accept.replace(/#dnd_/g, '');
			var _pp=_accept.split(",");
			var _page="";
			_page +="<center><table class='tbox_t' style='width: 350px;'>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Name")+"</td><td class='tboxi_tdr'><input id='name' type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Type")+"</td><td class='tboxi_tdr'><select class='tbox_s' id='type' size='1'>";
			if(__defb_t) {
				for(var _x in __defb_t) {
					var _v=_x;
					var _t=__defb_t[_x];
					if(_pp.length > 0) {
						for(var _i in _pp) {
							if(_v===_pp[_i]) {
								_page +="<option value='"+_v+"'>"+_t+"</option>";
							}
						}
					}
				}
			}
			_page +="</select></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Start Date")+"</td><td class='tboxi_tdr'><input id='sdate' type='text' class='tbox_i' value='' /></td></tr>";
			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("End Date")+"</td><td class='tboxi_tdr'><input id='edate' type='text' class='tbox_i' value='' /></td></tr>";

			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Start Time")+"</td><td class='tboxi_tdr'>";
			_page +="<select class='tbox_s' style='width: 55px;' id='stime_m' size='1'>";
			for(var _x=0; _x < 24; _x++) {
				if(_x < 10) {
					_x="0"+_x;
				}
				_page +="<option value='"+_x+"'>"+_x+"</option>";
			}
			_page +="</select>";
			_page +="<select class='tbox_s' style='margin-left: 2px; width: 55px;' id='stime_s' size='1'>";
			for(var _x=0; _x < 59; _x++) {
				var _xx=_x;
				if(_x < 10) {
					_xx="0"+_x;
				}
				_page +="<option value='"+_xx+"'>"+_xx+"</option>";
			}
			_page +="</select>";
			_page +="</td></tr>";

			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("End Time")+"</td><td class='tboxi_tdr'>";
			_page +="<select class='tbox_s' style='width: 55px;' id='etime_m' size='1'>";
			for(var _x=0; _x < 24; _x++) {
				var _xx=_x;
				if(_x < 10) {
					_xx="0"+_x;
				}
				if(_x==23) {
					_page +="<option value='"+_xx+"' selected>"+_xx+"</option>";
				} else {
					_page +="<option value='"+_xx+"'>"+_xx+"</option>";
				}
			}
			_page +="</select>";
			_page +="<select class='tbox_s' style='margin-left: 2px; width: 55px;' id='etime_s' size='1'>";
			for(var _x=0; _x < 59; _x++) {
				var _xx=_x;
				if(_x < 10) {
					_xx="0"+_x;
				}
				if(_x==58) {
					_page +="<option value='"+_xx+"' selected>"+_xx+"</option>";
				} else {
					_page +="<option value='"+_xx+"'>"+_xx+"</option>";
				}
			}
			_page +="</select>";
			_page +="</td></tr>";

			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Weekdays")+"</td><td class='tboxi_tdr'>";
			if(_weekday_array) {
				for(var _x in _weekday_array) {
					var _v=_x;
					var _t=_weekday_array[_x];
					_page +="<div class='table_fixed' style='float: left; vertical-align: middle;'><input type='checkbox' id='"+_v+"'>&nbsp;"+ucfirst(_t)+"</div><br />";
				}
			}
			_page +="</td></tr>";

			_page +="<tr><td class='tboxi_tdl'>"+js_gettext("Description")+"</td><td class='tboxi_tdr'><input id='note' type='text' class='tbox_i'></td></tr>";
			_page +="<tr><td colspan='2' style='text-align: center;'><hr class='tboxi_hr' /></td></tr>";
			_page +="<tr><td colspan='2' align='center'><table style='width: 80%;'>";
			_page +="<tr><td><div class='button' id='button_save'>Save</div>";
			_page +="</td><td><div class='button' id='button_cancel'>Cancel</div></td></tr>";
			_page +="</table></td></tr>";
			_page +="</table></center>";
			_page +="";
			$(_jpid+" div#defi_box_content").html(_page);
			__tnpage.button_hover();
			
			var _default=$(_jpid+" select[id=type] option:selected").attr("value");
			var _csel=function(opt) {
				$(_jpid+" table.tbox_t tr").hide().each(function(index) {
					if(index <= 1 || index >=7 ) {
						$(this).show();
					}
					if(opt==='single') {
						if(index==2 || index==3 || index==4 || index==5) {
							$(this).show();
						}
					} else if(opt=='recurring') {
						if(index==4 || index==5 || index==6) {
							$(this).show();
						}
					}
				});
			};
			
			var _img_calendar=_ICON['_ICON_CALENDAR'];
			$('#sdate').datepicker({
				mandatory: true,
			 	yearRange: '2008:2019',
				dateFormat: 'yy-mm-dd',
				defaultDate: '01-Jan-2008'
			}).attr('readonly', 'readonly').css({
				'cursor': 'pointer',
				'background-image': 'url('+_img_calendar+')',
				'background-position': 'right',
				'background-repeat': 'no-repeat'
			});

			$('#edate').datepicker({
				mandatory: true,
			 	yearRange: '2008:2019',
				dateFormat: 'yy-mm-dd',
				defaultDate: null
			}).attr('readonly', 'readonly').css({
				'cursor': 'pointer',
				'background-image': 'url('+_img_calendar+')',
				'background-position': 'right',
				'background-repeat': 'no-repeat'
			});

			_csel(_default);
			$(_jpid).show();
		}

		if(_csel) {
			$(_jpid+" select[id=type]").change(function() {
				var _pid=$(this).attr('value');
				_csel(_pid);
			});
		}

		$(_jpid+" div#button_cancel").click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			_defclose(_jpid);
		});

		// group box
		var _img_delete=_ICON['_ICON_DELETE'];
		var _defbox_click_action=function() {
			$(_jpid+' table.tbox_acl_tb img').hover(
				function() {
					$(this).css({'cursor':'pointer'});
				},
				function() {
					$(this).css({'cursor':'default'});
				}
			).click(function() {
				var _attr=basename($(this).attr('src'));
				if(_attr===basename(_img_delete)) {
					var _delid=$(this).attr('id');
					$(_jpid+' tr#'+_delid).remove();
					_rowcolor(_jpid+' table.tbox_acl_tb tr[id*=del] td','','',true);
				}
			});
		};

		var _defbox_search_action=function() {
			var _search_hide=function(opt) {
				$(_jpid+' table.tbox_acl_tb tr').each(function(i) {
					var _tid=$(this).attr('id');
					if(_tid && _tid.match(/del\d+/)) {
						if(opt===1) {
							$(this).hide();
						} else {
							$(this).show();
						}
					}
				});
			};
			$(_jpid+' div#tbox_acl_content input.tbox_acl_it').keyup(function(event) {
				var _str=$(this).val();
				if(_str.length===0) {
					_search_hide(0);
				} else {
					_search_hide(1);
					$(_jpid+' table.tbox_acl_tb tr').each(function(i) {
						var _tid=$(this).attr('id');
						if(_tid && _tid.match(/del\d+/)) {
							var _p=$(this).text();
							var _regex=new RegExp(_str, 'i');
							if(_regex.exec(_p)) {
								$(this).show().filter(function() {
									_rowcolor(_jpid+' table.tbox_acl_tb tr[id*=del] td','','',true);
								});
							}
						}
					});
				}
			});
		};
		
		$(_jpid+' div#tbox_acl_title_l').click(function(evt) {
			var e=evt || window.event;
			e.preventDefault();
			_navopen(section);
		});
		$(_jpid+' div#tbox_acl_content').droppable({
			accept: accept, 
			drop: function(ev, ui) {
				var _me=$(this).html();
				ui.draggable.clone().filter(
					function() {
						var _delcnt=$(_jpid+' table.tbox_acl_tb tbody tr[id*=del]').length;
						if(_delcnt!==0) {
							_delcnt++;
						}
						var _name=$(this).html();
						var _namef=$(this).text();
						if(__tndefinitions.checklist(_jpid+' table.tbox_acl_tb td',_namef)!==0) {
							return false;
						}
						var _page=$(_jpid+' table.tbox_acl_tb tbody').html()+"<tr id='dummy'>";
						_page +="<td class='tbox_acl_td_it tbox_acl_itt'><img id='dummy' style='padding-right: 2px; float: left; height: 14px;' src='"+_img_delete+"'><div class='def_ibox1_ii'>"+_name+"</div></td>";
						_page +="</tr>";
						$(_jpid+' table.tbox_acl_tb tbody').html(_page);
						$('tr#dummy').attr({'id':'del'+_delcnt});
						$('img#dummy').attr({'id':'del'+_delcnt});
						_defbox_click_action();
						_defbox_search_action();
					});
					_rowcolor(_jpid+' table.tbox_acl_tb tr[id*=del] td','','',true);
				}
		});
	};

	var _country=function(_default) {
		var _page="";
		if(_country_array) {
			for(var x in _country_array) {
				var _s=x;
				var _l=_country_array[x];
				if(_s===_default) {
					_page +="<option value='"+_s+"' selected>"+_l+"</option>";
				} else {
					_page +="<option value='"+_s+"'>"+_l+"</option>";
				}
			}
		}
		$('select#country').html(_page);
	};
	var _timezone=function(_default) {
		var _page="";
		if(_timezone_array) {
			for(var x in _timezone_array) {
				var _t=_timezone_array[x];
				if(_t===_default) {
					_page +="<option value='"+_t+"' selected>"+_t+"</option>";
				} else {
					_page +="<option value='"+_t+"'>"+_t+"</option>";
				}
			}
		}
		$('select#timezone').html(_page);
	};

	var _keybind=function(evt) {
		var e = evt || window.event;
		var _keys = {
			'Z':function(){_navopen('networks');},
			'X':function(){_navopen('services');},
			'C':function(){_navopen('interfaces');},
  			'E':function(){_navopen('users');},
  			'V':function(){_navopen('timeevents');}
		};
		var knum = document.all ? window.event.keyCode : e.which;
		if(e.ctrlKey && (knum != 17)) {
			var key = String.fromCharCode(knum);
			if(_keys[key]) {
				_keys[key]();
			}
		}
		return true;
	};

	var _check_list=function(pid,name) {
		var _found=0;
		$(pid).each(function() {
			var _find=$(this).text();
			if($.trim(_find)===$.trim(name)) {
				_found++;
			}
		});
		if(_found!=0) {
			if(function_exists('nd')) {
				nd();
			}
		}
		return _found;
	};

	var _rowcolor=function(pid,color1,color2,ch) {
		var _c1=color1 || "#e8e6e6";
		var _c2=color1 || "#ededed";
		var _h=ch || false;
		$(document).ready(function() {
			var _x=1;
			$(pid).each(function(i) {
				if(_h==true) {
					if($(this).css('display')!=='none') {
						if(_x==2) {
							$(this).css('background-color',_c1);
							_x=0;
						} else {
							$(this).css('background-color',_c2);
						}
						_x++;
					}
				} else {
					if(_x==2) {
						$(this).css('background-color',_c1);
						_x=0;
					} else {
						$(this).css('background-color',_c2);
					}
					_x++;
				}
			});
		});
	};
	
	return {
		navopen: function(section) {
			_navopen(section);
		},
		navclose: function(speed) {
			_navclose(speed);
		},
		navrefresh: function(section) {
			if(document.getElementById('def_box')) {
				if(__navtoggle[section]==true) {
					__navtoggle[section]=false;
					_navopen(section);
				}
				return true;
			}
			return false;
		},

		defopen: function(pid,section,top,left,accept,dataval) {
			_defopen(pid,section,top,left,accept,dataval);
		},
		defclose: function(pid) {
			_defclose(pid);
		},
		country: function(_default) {
			_country(_default);
		},
		timezone: function(_default) {
			_timezone(_default);
		},
		keybind: function() {
			document.onkeydown=_keybind;
			var _rmovl=function() {
				if(function_exists('nd')) {
					nd();
				}
			};
			document.onmouseout=_rmovl;
		},
		checklist: function(pid,name) {
			return _check_list(pid,name);
		},
		rowcolor: function(pid,color1,color2,ch) {
			_rowcolor(pid,color1,color2,ch);
		}
	};
}();
