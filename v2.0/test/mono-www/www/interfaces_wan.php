#!/usr/local/bin/php
<?php 
/*
	interfaces_wan.php
	part of m0n0wall (http://m0n0.ch/wall)
	
	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	
	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.
	
	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

require("guiconfig.inc");

$wancfg = &$config['interfaces']['wan'];
$optcfg = &$config['interfaces']['wan'];

$pconfig['username'] = $config['pppoe']['username'];
$pconfig['password'] = $config['pppoe']['password'];
$pconfig['provider'] = $config['pppoe']['provider'];

$pconfig['pptp_username'] = $config['pptp']['username'];
$pconfig['pptp_password'] = $config['pptp']['password'];
$pconfig['pptp_local'] = $config['pptp']['local'];
$pconfig['pptp_subnet'] = $config['pptp']['subnet'];
$pconfig['pptp_remote'] = $config['pptp']['remote'];

$pconfig['bigpond_username'] = $config['bigpond']['username'];
$pconfig['bigpond_password'] = $config['bigpond']['password'];
$pconfig['bigpond_authserver'] = $config['bigpond']['authserver'];
$pconfig['bigpond_authdomain'] = $config['bigpond']['authdomain'];
$pconfig['bigpond_minheartbeatinterval'] = $config['bigpond']['minheartbeatinterval'];

$pconfig['dhcphostname'] = $wancfg['dhcphostname'];

if ($wancfg['ipaddr'] == "dhcp") {
	$pconfig['type'] = "DHCP";
} else if ($wancfg['ipaddr'] == "pppoe") {
	$pconfig['type'] = "PPPoE";
} else if ($wancfg['ipaddr'] == "pptp") {
	$pconfig['type'] = "PPTP";
} else if ($wancfg['ipaddr'] == "bigpond") {
	$pconfig['type'] = "BigPond";
} else {
	$pconfig['type'] = "Static";
	$pconfig['ipaddr'] = $wancfg['ipaddr'];
	$pconfig['subnet'] = $wancfg['subnet'];
	$pconfig['gateway'] = $wancfg['gateway'];
}

$pconfig['blockpriv'] = isset($wancfg['blockpriv']);
$pconfig['spoofmac'] = $wancfg['spoofmac'];
$pconfig['mtu'] = $wancfg['mtu'];

/* Wireless interface? */
if (isset($optcfg['wireless'])) {
	require("interfaces_wlan.inc");
	wireless_config_init();
}

if ($_POST) {

	unset($input_errors);
	$pconfig = $_POST;

	/* input validation */
	if ($_POST['type'] == "Static") {
		$reqdfields = explode(" ", "ipaddr subnet gateway");
		$reqdfieldsn = explode(",", "IP address,Subnet bit count,Gateway");
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	} else if ($_POST['type'] == "PPPoE") {
		$reqdfields = explode(" ", "username password");
		$reqdfieldsn = explode(",", "PPPoE username,PPPoE password");
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	} else if ($_POST['type'] == "PPTP") {
		$reqdfields = explode(" ", "pptp_username pptp_password pptp_local pptp_subnet pptp_remote");
		$reqdfieldsn = explode(",", "PPTP username,PPTP password,PPTP local IP address,PPTP subnet,PPTP remote IP address");
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	} else if ($_POST['type'] == "BigPond") {
		$reqdfields = explode(" ", "bigpond_username bigpond_password");
		$reqdfieldsn = explode(",", "BigPond username,BigPond password");
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	}
	
	if (($_POST['ipaddr'] && !is_ipaddr($_POST['ipaddr']))) {
		$input_errors[] = "A valid IP address must be specified.";
	}
	if (($_POST['subnet'] && !is_numeric($_POST['subnet']))) {
		$input_errors[] = "A valid subnet bit count must be specified.";
	}
	if (($_POST['gateway'] && !is_ipaddr($_POST['gateway']))) {
		$input_errors[] = "A valid gateway must be specified.";
	}
	if (($_POST['provider'] && !is_domain($_POST['provider']))) {
		$input_errors[] = "The service name contains invalid characters.";
	}
	if (($_POST['pptp_local'] && !is_ipaddr($_POST['pptp_local']))) {
		$input_errors[] = "A valid PPTP local IP address must be specified.";
	}
	if (($_POST['pptp_subnet'] && !is_numeric($_POST['pptp_subnet']))) {
		$input_errors[] = "A valid PPTP subnet bit count must be specified.";
	}
	if (($_POST['pptp_remote'] && !is_ipaddr($_POST['pptp_remote']))) {
		$input_errors[] = "A valid PPTP remote IP address must be specified.";
	}
	if (($_POST['bigpond_authserver'] && !is_domain($_POST['bigpond_authserver']))) {
		$input_errors[] = "The authentication server name contains invalid characters.";
	}
	if (($_POST['bigpond_authdomain'] && !is_domain($_POST['bigpond_authdomain']))) {
		$input_errors[] = "The authentication domain name contains invalid characters.";
	}
	if ($_POST['bigpond_minheartbeatinterval'] && !is_numericint($_POST['bigpond_minheartbeatinterval'])) {
		$input_errors[] = "The minimum heartbeat interval must be an integer.";
	}
	if (($_POST['spoofmac'] && !is_macaddr($_POST['spoofmac']))) {
		$input_errors[] = "A valid MAC address must be specified.";
	}
	if ($_POST['mtu'] && (($_POST['mtu'] < 576) || ($_POST['mtu'] > 1500))) {
		$input_errors[] = "The MTU must be between 576 and 1500 bytes.";
	}
	
	/* Wireless interface? */
	if (isset($optcfg['wireless'])) {
		$wi_input_errors = wireless_config_post();
		if ($wi_input_errors) {
			$input_errors = array_merge($input_errors, $wi_input_errors);
		}
	}

	if (!$input_errors) {
	
		unset($wancfg['ipaddr']);
		unset($wancfg['subnet']);
		unset($wancfg['gateway']);
		unset($wancfg['dhcphostname']);
		unset($config['pppoe']['username']);
		unset($config['pppoe']['password']);
		unset($config['pppoe']['provider']);
		unset($config['pptp']['username']);
		unset($config['pptp']['password']);
		unset($config['pptp']['local']);
		unset($config['pptp']['subnet']);
		unset($config['pptp']['remote']);
		unset($config['bigpond']['username']);
		unset($config['bigpond']['password']);
		unset($config['bigpond']['authserver']);
		unset($config['bigpond']['authdomain']);
		unset($config['bigpond']['minheartbeatinterval']);
	
		if ($_POST['type'] == "Static") {
			$wancfg['ipaddr'] = $_POST['ipaddr'];
			$wancfg['subnet'] = $_POST['subnet'];
			$wancfg['gateway'] = $_POST['gateway'];
		} else if ($_POST['type'] == "DHCP") {
			$wancfg['ipaddr'] = "dhcp";
			$wancfg['dhcphostname'] = $_POST['dhcphostname'];
		} else if ($_POST['type'] == "PPPoE") {
			$wancfg['ipaddr'] = "pppoe";
			$config['pppoe']['username'] = $_POST['username'];
			$config['pppoe']['password'] = $_POST['password'];
			$config['pppoe']['provider'] = $_POST['provider'];
		} else if ($_POST['type'] == "PPTP") {
			$wancfg['ipaddr'] = "pptp";
			$config['pptp']['username'] = $_POST['pptp_username'];
			$config['pptp']['password'] = $_POST['pptp_password'];
			$config['pptp']['local'] = $_POST['pptp_local'];
			$config['pptp']['subnet'] = $_POST['pptp_subnet'];
			$config['pptp']['remote'] = $_POST['pptp_remote'];
		} else if ($_POST['type'] == "BigPond") {
			$wancfg['ipaddr'] = "bigpond";
			$config['bigpond']['username'] = $_POST['bigpond_username'];
			$config['bigpond']['password'] = $_POST['bigpond_password'];
			$config['bigpond']['authserver'] = $_POST['bigpond_authserver'];
			$config['bigpond']['authdomain'] = $_POST['bigpond_authdomain'];
			$config['bigpond']['minheartbeatinterval'] = $_POST['bigpond_minheartbeatinterval'];
		}
		
		$wancfg['blockpriv'] = $_POST['blockpriv'] ? true : false;
		$wancfg['spoofmac'] = $_POST['spoofmac'];
		$wancfg['mtu'] = $_POST['mtu'];
			
		write_config();
		
		$retval = 0;
		if (!file_exists($d_sysrebootreqd_path)) {
			config_lock();
			$retval = interfaces_wan_configure();
			config_unlock();
		}
		$savemsg = get_std_save_message($retval);
	}
}
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title>firewall WEBGUI - Interfaces: WAN</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link href="gui.css" rel="stylesheet" type="text/css">
<script language="JavaScript">
<!--
function type_change() {
	switch (document.iform.type.selectedIndex) {
		case 0:
			document.iform.username.disabled = 1;
			document.iform.password.disabled = 1;
			document.iform.provider.disabled = 1;
			document.iform.ipaddr.disabled = 0;
			document.iform.subnet.disabled = 0;
			document.iform.gateway.disabled = 0;
			document.iform.pptp_username.disabled = 1;
			document.iform.pptp_password.disabled = 1;
			document.iform.pptp_local.disabled = 1;
			document.iform.pptp_subnet.disabled = 1;
			document.iform.pptp_remote.disabled = 1;
			document.iform.bigpond_username.disabled = 1;
			document.iform.bigpond_password.disabled = 1;
			document.iform.bigpond_authserver.disabled = 1;
			document.iform.bigpond_authdomain.disabled = 1;
			document.iform.bigpond_minheartbeatinterval.disabled = 1;
			document.iform.dhcphostname.disabled = 1;
			break;
		case 1:
			document.iform.username.disabled = 1;
			document.iform.password.disabled = 1;
			document.iform.provider.disabled = 1;
			document.iform.ipaddr.disabled = 1;
			document.iform.subnet.disabled = 1;
			document.iform.gateway.disabled = 1;
			document.iform.pptp_username.disabled = 1;
			document.iform.pptp_password.disabled = 1;
			document.iform.pptp_local.disabled = 1;
			document.iform.pptp_subnet.disabled = 1;
			document.iform.pptp_remote.disabled = 1;
			document.iform.bigpond_username.disabled = 1;
			document.iform.bigpond_password.disabled = 1;
			document.iform.bigpond_authserver.disabled = 1;
			document.iform.bigpond_authdomain.disabled = 1;
			document.iform.bigpond_minheartbeatinterval.disabled = 1;
			document.iform.dhcphostname.disabled = 0;
			break;
		case 2:
			document.iform.username.disabled = 0;
			document.iform.password.disabled = 0;
			document.iform.provider.disabled = 0;
			document.iform.ipaddr.disabled = 1;
			document.iform.subnet.disabled = 1;
			document.iform.gateway.disabled = 1;
			document.iform.pptp_username.disabled = 1;
			document.iform.pptp_password.disabled = 1;
			document.iform.pptp_local.disabled = 1;
			document.iform.pptp_subnet.disabled = 1;
			document.iform.pptp_remote.disabled = 1;
			document.iform.bigpond_username.disabled = 1;
			document.iform.bigpond_password.disabled = 1;
			document.iform.bigpond_authserver.disabled = 1;
			document.iform.bigpond_authdomain.disabled = 1;
			document.iform.bigpond_minheartbeatinterval.disabled = 1;
			document.iform.dhcphostname.disabled = 1;
			break;
		case 3:
			document.iform.username.disabled = 1;
			document.iform.password.disabled = 1;
			document.iform.provider.disabled = 1;
			document.iform.ipaddr.disabled = 1;
			document.iform.subnet.disabled = 1;
			document.iform.gateway.disabled = 1;
			document.iform.pptp_username.disabled = 0;
			document.iform.pptp_password.disabled = 0;
			document.iform.pptp_local.disabled = 0;
			document.iform.pptp_subnet.disabled = 0;
			document.iform.pptp_remote.disabled = 0;
			document.iform.bigpond_username.disabled = 1;
			document.iform.bigpond_password.disabled = 1;
			document.iform.bigpond_authserver.disabled = 1;
			document.iform.bigpond_authdomain.disabled = 1;
			document.iform.bigpond_minheartbeatinterval.disabled = 1;
			document.iform.dhcphostname.disabled = 1;
			break;
		case 4:
			document.iform.username.disabled = 1;
			document.iform.password.disabled = 1;
			document.iform.provider.disabled = 1;
			document.iform.ipaddr.disabled = 1;
			document.iform.subnet.disabled = 1;
			document.iform.gateway.disabled = 1;
			document.iform.pptp_username.disabled = 1;
			document.iform.pptp_password.disabled = 1;
			document.iform.pptp_local.disabled = 1;
			document.iform.pptp_subnet.disabled = 1;
			document.iform.pptp_remote.disabled = 1;
			document.iform.bigpond_username.disabled = 0;
			document.iform.bigpond_password.disabled = 0;
			document.iform.bigpond_authserver.disabled = 0;
			document.iform.bigpond_authdomain.disabled = 0;
			document.iform.bigpond_minheartbeatinterval.disabled = 0;
			document.iform.dhcphostname.disabled = 1;
			break;
	}
}
//-->
</script>
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle">Interfaces: WAN</p>
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
            <form action="interfaces_wan.php" method="post" name="iform" id="iform">
              <table width="100%" border="0" cellpadding="6" cellspacing="0">
                <tr> 
                  <td valign="middle"><strong>Type</strong></td>
                  <td> <select name="type" class="formfld" id="type" onchange="type_change()">
                      <?php $opts = split(" ", "Static DHCP PPPoE PPTP BigPond");
				foreach ($opts as $opt): ?>
                      <option <?php if ($opt == $pconfig['type']) echo "selected";?>> 
                      <?=htmlspecialchars($opt);?>
                      </option>
                      <?php endforeach; ?>
                    </select></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" height="4"></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" class="vnsepcell">General configuration</td>
                </tr>
                <tr> 
                  <td valign="top" class="vncell">MAC address</td>
                  <td class="vtable"> <input name="spoofmac" type="text" class="formfld" id="spoofmac" size="30" value="<?=htmlspecialchars($pconfig['spoofmac']);?>"> 
                    <br>
                    This field can be used to modify (&quot;spoof&quot;) the MAC 
                    address of the WAN interface<br>
                    (may be required with some cable connections)<br>
                    Enter a MAC address in the following format: xx:xx:xx:xx:xx:xx 
                    or leave blank</td>
                </tr>
                <tr> 
                  <td valign="top" class="vncell">MTU</td>
                  <td class="vtable"> <input name="mtu" type="text" class="formfld" id="mtu" size="8" value="<?=htmlspecialchars($pconfig['mtu']);?>"> 
                    <br>
                    If you enter a value in this field, then MSS clamping for 
                    TCP connections to the value entered above minus 40 (TCP/IP 
                    header size) will be in effect. If you leave this field blank, 
                    an MTU of 1492 bytes for PPPoE and 1500 bytes for all other 
                    connection types will be assumed.</td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" height="16"></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" class="vnsepcell">Static IP configuration</td>
                </tr>
                <tr> 
                  <td width="100" valign="top" class="vncellreq">IP address</td>
                  <td class="vtable"> <input name="ipaddr" type="text" class="formfld" id="ipaddr" size="20" value="<?=htmlspecialchars($pconfig['ipaddr']);?>">
                    / 
                    <select name="subnet" class="formfld" id="subnet">
                      <?php for ($i = 31; $i > 0; $i--): ?>
                      <option value="<?=$i;?>" <?php if ($i == $pconfig['subnet']) echo "selected"; ?>> 
                      <?=$i;?>
                      </option>
                      <?php endfor; ?>
                    </select></td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Gateway</td>
                  <td class="vtable"> <input name="gateway" type="text" class="formfld" id="gateway" size="20" value="<?=htmlspecialchars($pconfig['gateway']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" height="16"></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" class="vnsepcell">DHCP client configuration</td>
                </tr>
                <tr> 
                  <td valign="top" class="vncell">Hostname</td>
                  <td class="vtable"> <input name="dhcphostname" type="text" class="formfld" id="dhcphostname" size="40" value="<?=htmlspecialchars($pconfig['dhcphostname']);?>">
                    <br>
                    The value in this field is sent as the DHCP client identifier 
                    and hostname when requesting a DHCP lease. Some ISPs may require 
                    this (for client identification).</td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" height="16"></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" class="vnsepcell">PPPoE configuration</td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Username</td>
                  <td class="vtable"><input name="username" type="text" class="formfld" id="username" size="20" value="<?=htmlspecialchars($pconfig['username']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Password</td>
                  <td class="vtable"><input name="password" type="text" class="formfld" id="password" size="20" value="<?=htmlspecialchars($pconfig['password']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td valign="top" class="vncell">Service name</td>
                  <td class="vtable"><input name="provider" type="text" class="formfld" id="provider" size="20" value="<?=htmlspecialchars($pconfig['provider']);?>"> 
                    <br> <span class="vexpl">Hint: this field can usually be left 
                    empty</span></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" height="16"></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" class="vnsepcell">PPTP configuration</td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Username</td>
                  <td class="vtable"><input name="pptp_username" type="text" class="formfld" id="pptp_username" size="20" value="<?=htmlspecialchars($pconfig['pptp_username']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Password</td>
                  <td class="vtable"><input name="pptp_password" type="text" class="formfld" id="pptp_password" size="20" value="<?=htmlspecialchars($pconfig['pptp_password']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td width="100" valign="top" class="vncellreq">Local IP address</td>
                  <td class="vtable"> <input name="pptp_local" type="text" class="formfld" id="pptp_local" size="20" value="<?=htmlspecialchars($pconfig['pptp_local']);?>">
                    / 
                    <select name="pptp_subnet" class="formfld" id="pptp_subnet">
                      <?php for ($i = 31; $i > 0; $i--): ?>
                      <option value="<?=$i;?>" <?php if ($i == $pconfig['pptp_subnet']) echo "selected"; ?>> 
                      <?=$i;?>
                      </option>
                      <?php endfor; ?>
                    </select></td>
                </tr>
                <tr> 
                  <td width="100" valign="top" class="vncellreq">Remote IP address</td>
                  <td class="vtable"> <input name="pptp_remote" type="text" class="formfld" id="pptp_remote" size="20" value="<?=htmlspecialchars($pconfig['pptp_remote']);?>"> 
                  </td>
                </tr>
				<tr> 
                  <td colspan="2" valign="top" height="16"></td>
                </tr>
                <tr> 
                  <td colspan="2" valign="top" class="vnsepcell">BigPond Cable configuration</td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Username</td>
                  <td class="vtable"><input name="bigpond_username" type="text" class="formfld" id="bigpond_username" size="20" value="<?=htmlspecialchars($pconfig['bigpond_username']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td valign="top" class="vncellreq">Password</td>
                  <td class="vtable"><input name="bigpond_password" type="text" class="formfld" id="bigpond_password" size="20" value="<?=htmlspecialchars($pconfig['bigpond_password']);?>"> 
                  </td>
                </tr>
                <tr> 
                  <td valign="top" class="vncell">Authentication server</td>
                  <td class="vtable"><input name="bigpond_authserver" type="text" class="formfld" id="bigpond_authserver" size="20" value="<?=htmlspecialchars($pconfig['bigpond_authserver']);?>">
                    <br>
                  <span class="vexpl">If this field is left empty, the default (&quot;dce-server&quot;) is used. </span></td>
                </tr>
                <tr> 
                  <td valign="top" class="vncell">Authentication domain</td>
                  <td class="vtable"><input name="bigpond_authdomain" type="text" class="formfld" id="bigpond_authdomain" size="20" value="<?=htmlspecialchars($pconfig['bigpond_authdomain']);?>">
                    <br>
                  <span class="vexpl">If this field is left empty, the domain name assigned via DHCP will be used.<br>
                  <br>
                  Note: the BigPond client implicitly sets the &quot;Allow DNS server list to be overridden by DHCP/PPP on WAN&quot; on the System: General setup page.            </span></td>
                </tr>
                <tr>
                  <td valign="top" class="vncell">Min. heartbeat interval</td>
                  <td class="vtable">
                    <input name="bigpond_minheartbeatinterval" type="text" class="formfld" id="bigpond_minheartbeatinterval" size="8" value="<?=htmlspecialchars($pconfig['bigpond_minheartbeatinterval']);?>">
                    seconds<br>
    Setting this to a sensible value (e.g. 60 seconds) can protect against DoS attacks. </td>
                </tr>
                <?php /* Wireless interface? */
				if (isset($optcfg['wireless']))
					wireless_config_print();
				?>
                <tr> 
                  <td height="16" colspan="2" valign="top"></td>
                </tr>
                <tr> 
                  <td valign="middle">&nbsp;</td>
                  <td class="vtable"> <input name="blockpriv" type="checkbox" id="blockpriv" value="yes" <?php if ($pconfig['blockpriv'] == "yes") echo "checked"; ?>> 
                    <strong>Block private networks</strong><br>
                    When set, this option blocks traffic from IP addresses that 
                    are reserved for private<br>
                    networks as per RFC 1918 (10/8, 172.16/12, 192.168/16) as 
                    well as loopback addresses<br>
                    (127/8). You should generally leave this option turned on, 
                    unless your WAN network<br>
                    lies in such a private address space, too.</td>
                </tr>
                <tr> 
                  <td width="100" valign="top">&nbsp;</td>
                  <td> &nbsp;<br> <input name="Submit" type="submit" class="formbtn" value="Save"> 
                  </td>
                </tr>
              </table>
</form>
<script language="JavaScript">
<!--
type_change();
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
