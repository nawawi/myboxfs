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
// $Id: dmz.inc,v 1.00 2003/07/28 1:07 AM nawawi Exp $
//
	function do_flush() {
		shell_exec("/sbin/ipvsadm -C >/dev/null 2>&1");
	}
	function do_hosts() {
		if(file_exists('/etc/init.d/hosts.sh')) {
			shell_exec('php -q /etc/init.d/hosts.sh >/dev/null 2>&1');
		}
	}
	function do_dmz() {
		if(!file_exists('/hd/configs/dmz.lst')) {
			return;
		}
		$handle=fopen('/hd/configs/dmz.lst','r');
		while(!feof($handle)) {
			$buff=fgets($handle,500);
			$buff=trim($buff);
			if($buff!='') {
				list($extip,$intip,$proto,$port,$desc)=preg_split("/\|/",$buff);
				if($proto=='tcp') {
					shell_exec("/sbin/ipvsadm -A -t $extip:$port -s rr >/dev/null 2>&1");
					shell_exec("/sbin/ipvsadm -a -t $extip:$port -r $intip:$port -m -w 1 >/dev/null 2>&1");
				}
				if($proto=='udp') {
					shell_exec("/sbin/ipvsadm -A -u $extip:$port -s rr");
					shell_exec("/sbin/ipvsadm -a -u $extip:$port -r $intip:$port -m -w 1 >/dev/null 2>&1");
				}
			}
		}
		fclose($handle);
	}
	function do_extip() {
		$handle=popen("/sbin/ifconfig |grep ^eth",'r');
		while($buff=fgets($handle,4096)) {
			$buff=trim($buff);
			list($eth,$rest)=preg_split("/\s+/",$buff);
			if($eth!='' && preg_match("/^eth0:(\d+)/",$eth)) {
        			shell_exec("/sbin/ifconfig $eth down");
			} 
		}
		pclose($handle);
		if(!file_exists('/hd/configs/dmz_e.lst')) {
			return;
		}
		$handle=fopen('/hd/configs/dmz_e.lst','r');
		$x='0';
		while(!feof($handle)) {
			$buff=fgets($handle,500);
			$buff=trim($buff);
			if($buff!='') {
				list($extip,$emask)=preg_split("/\|/",$buff);
				if($extip!='') {
					$test=shell_exec("/sbin/ifconfig |grep \"inet addr:$extip\"");
					if($test=='') {
						if($emask!='') {
							shell_exec("/sbin/ifconfig eth0:$x $extip netmask $emask");
						} else {
							shell_exec("/sbin/ifconfig eth0:$x $extip");
						}
						$x++;
					}
				}
				unset($test);
			}
		}
		fclose($handle);

	}
?>