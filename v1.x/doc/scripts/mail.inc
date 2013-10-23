<?
//
// Mybox Firewall - A Firewall Appliance
// http://www.mybox.net.my/
//
// (C) Copyright 2002,2003,2004 Mohd Nawawi Mohamad Jamili, TraceNetwork Corporation Sdn. Bhd.
//
// This program is not free software; you cannot redistribute it and/or
// modify without permission copyright owner.
//
// This code is protected by copyright law and international treaties. 
// Unauthorized reproduction or distribution of this program, or any portion of it, 
// may result in severe civil and criminal penalties, and will be prosecuted to the 
// maximum extent possible under the law.
//
// $Id: mail.inc,v 1.05 2003/07/28 1:07 AM nawawi Exp $
//

	function system_alert($access,$msg) {
		if(!is_dir("/hd/reports/spool")) @mkdir("/hd/reports/spool");
		clearstatcache();
		if($access=='0') {
			$who='Admin';
		} elseif($access=='1') {
			$who='User';
		} else {
			// no access
			return;
		}
		$ip=getenv("REMOTE_ADDR");
		$message="$who from $ip -> $msg";
		$time=time();
		$fi=fopen("/hd/reports/spool/1-$time","w");
		fputs($fi,"$message");
		fclose($fi);
	}

	function fa_alert($msg) {
		if(!is_dir("/hd/reports/spool")) @mkdir("/hd/reports/spool");
		clearstatcache();
		$ip=getenv("REMOTE_ADDR");
		$time=time();
		$fi=fopen("/hd/reports/spool/3-$time","w");
		fputs($fi,"$msg");
		fclose($fi);
	}

	function backup_alert($access,$msg) {
		if(!is_dir("/hd/reports/spool")) @mkdir("/hd/reports/spool");
		clearstatcache();
		if($access=='0') {
			$who='Admin';
		} elseif($access=='1') {
			$who='User';
		} else {
			// no access
			return;
		}
		$ip=getenv("REMOTE_ADDR");
		$message="$who from $ip -> $msg";
		$time=time();
		$fi=fopen("/hd/reports/spool/2-$time","w");
		fputs($fi,"$message");
		fclose($fi);
	}
?>