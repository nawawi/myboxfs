myboxgears = {
	createstore: function() {
		if('undefined' == typeof google || ! google.gears) {
			return;
		}
		if('undefined' == typeof localServer) {
			localServer = google.gears.factory.create("beta.localserver");
		}
		store = localServer.createManagedStore(this.storename());
		store.manifestUrl = "gears-manifest.exh";
		store.checkForUpdate();
		$('div[id*=gears-msg]').hide();
		$('div#gears-msg3').show();
		store.oncomplete = function(){$('span#gears-wait').html(js_gettext('Update completed.'));};
		store.onerror = function(){$('span#gears-wait').html(js_gettext('Error'));};
		store.onprogress = function(e){
			if($('span#gears-upd-number')) {
				$('span#gears-upd-number').html(' ' + e.filesComplete + ' / ' + e.filesTotal);
			}
		};
	},

	getpermission: function() {
		var perm = true;
		if('undefined' != typeof google && google.gears) {
			if(!google.gears.factory.hasPermission) {
				perm = google.gears.factory.getPermission('MyboxWebGUI');
			}
			if(perm) {
				try { this.createstore(); } catch(e) { 
					$('div[id*=gears-msg]').hide();
					$('div#gears-msg2').show();
				 }
			} else {
				$('div[id*=gears-msg]').hide();
				$('div#gears-msg4').show();
			}
		}
	},

	storename: function() {
		var name = window.location.protocol + window.location.host;
		name = name.replace(/[\/\\:*"?<>|;,]+/g, '_');
		name = 'mybox_' + name.substring(0, 60);
		return name;
	},

	init: function() {
		if(typeof google=='undefined' || !google.gears) {
			$('div[id*=gears-msg]').hide();
			$('div#gears-msg1').show();
		} else {
			if(google.gears.factory.hasPermission) {
				if(typeof store=='undefined') {
					this.createstore();
				} else {
					$('div[id*=gears-msg]').hide();
					$('div#gears-msg4').show();
				}
			} else {
				$('div[id*=gears-msg]').hide();
				$('div#gears-msg2').show();
			}
		}
	}
};

(function() {
	if(typeof google!='undefined' && google.gears) {
		return;
	}
	var gf = false;
	if(typeof GearsFactory!='undefined') {
		gf = new GearsFactory();
	} else {
		try {
			gf = new ActiveXObject('Gears.Factory');
			if(factory.getBuildInfo().indexOf('ie_mobile') != -1) {
				gf.privateSetGlobalObject(this);
			}
		} catch (e) {
			if(('undefined' != typeof navigator.mimeTypes) && navigator.mimeTypes['application/x-googlegears']) {
				gf = document.createElement("object");
				gf.style.display = "none";
				gf.width = 0;
				gf.height = 0;
				gf.type = "application/x-googlegears";
				document.documentElement.appendChild(gf);
			}
		}
	}

	if(!gf) {
		return;
	}
	if('undefined' == typeof google) {
		google = {};
	}
	if(!google.gears) {
		google.gears = { factory : gf };
	}
})();
