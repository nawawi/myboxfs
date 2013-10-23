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
// $Id: functions.inc,v 1.003 2003/07/28 1:07 AM nawawi Exp $
//
	$CONFIG='/hd/configs/mybox.cnf';
	if(file_exists('inifunc.inc')) {
		include_once('inifunc.inc');
	} elseif(file_exists('scripts/inifunc.inc')) {
		include_once('scripts/inifunc.inc');
	} else {
		include_once('/usr/miniserv/scripts/inifunc.inc');
	}
	function get_ini($section,$param) {
		global $CONFIG;
		$ret=getini($section,$param,$CONFIG);
		if($ret=='NULL') writeini($section,$param,'',$CONFIG);
		return trim("$ret");
	}
	function write_ini($section,$param,$value) {
		global $CONFIG;
		$ret=writeini($section,$param,$value,$CONFIG);
		return trim("$ret");
	}
	function write_updatelog($pkg) {
		$date=date('l|d/m/Y|h:i:s a');
		$remote_addr=getenv("REMOTE_ADDR");
		if(file_exists('/hd/configs/update.lst')) {
			$handle=fopen("/hd/configs/update.lst", "a");
		} else {
			$handle=fopen("/hd/configs/update.lst", "w");
		}
		fputs($handle,"$date|$pkg|$remote_addr\n");
		fclose($handle);
	}

	function write_backuplog($action) {
		$date=date('l|d/m/Y|h:i:s a');
		if(file_exists('/hd/configs/backup.lst')) {
			$handle=fopen("/hd/configs/backup.lst", "a");
		} else {
			$handle=fopen("/hd/configs/backup.lst", "w");
		}
		fputs($handle,"$date|$action\n");
		fclose($handle);
	}

	function write_rbackuplog($status) {
		$date=date('l|d/m/Y|h:i:s a');
		if(file_exists('/hd/configs/rbackup.lst')) {
			$handle=fopen("/hd/configs/rbackup.lst", "a");
		} else {
			$handle=fopen("/hd/configs/rbackup.lst", "w");
		}
		fputs($handle,"$date|$status\n");
		fclose($handle);
	}
	function write_alarmlist($value) {
		$list=explode(',',$value);
		$handle=fopen("/hd/configs/alarm.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}

	function write_hostlist($value) {
		$list=explode(',',$value);
		$handle=fopen("/hd/configs/hosts.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}

	function del_route() {
		if(!file_exists('/hd/configs/routing.lst')) {
			return;
		}
		$handle=fopen('/hd/configs/routing.lst','r');
		while(!feof($handle)) {
			$buff=fgets($handle,500);
			$buff=trim($buff);
			if($buff!='') {
				list($ip,$netmask,$gw)=explode('|',$buff);
				if($buff!='') {
					if($ip!='' && $netmask!='' && $gw!='') {
						shell_exec("/sbin/route del -net $ip netmask $netmask gw $gw >/dev/null 2>&1");
					}
				}
			}
		}
		fclose($handle);
	}

	function write_routelist($value) {
		del_route();
		$list=explode(',',$value);
		$handle=fopen("/hd/configs/routing.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}

	function write_dmzhostlist($value) {
		$list=explode(',',$value);
		$handle=fopen("/hd/configs/dmz_h.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}
	function write_dmzextiplist($value) {
		$list=explode(',',$value);
		$handle=fopen("/hd/configs/dmz_e.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}
	function write_dmzlist($value) {
		$list=explode(',',$value);
		$handle=fopen("/hd/configs/dmz.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}

	function write_filterlist($value,$rule) {
		if($rule==0) {
			$handle=fopen("/hd/configs/fwrule.lst", "a");
			fputs($handle,"$value\n");
			fclose($handle);
		} else {
			if(file_exists('/hd/configs/fwrule.lst')) {
				$buff=file('/hd/configs/fwrule.lst');
				$handle=fopen("/hd/configs/fwrule.lst", "w");
				fputs($handle,"$value\n");
				foreach($buff as $line) {
					if($line!='') {
						$line=trim($line);
						fputs($handle,"$line\n");
					}
				}
				fclose($handle);
			} else {
				$handle=fopen("/hd/configs/fwrule.lst", "w");
				fputs($handle,"$value\n");
				fclose($handle);
			}
		}
	}

	function update_filterlist($value,$line) {
		if(file_exists('/hd/configs/fwrule.lst')) {
			$buff=file('/hd/configs/fwrule.lst');
			$buff[$line]="$value";
			$handle=fopen("/hd/configs/fwrule.lst", "w");
			foreach($buff as $line) {
				$line=trim($line);
				if($line!='') {
					fputs($handle,"$line\n");
				}
			}
			fclose($handle);
		}
	}

	function update_fwdlist($value,$line) {
		if(file_exists('/hd/configs/fwdrule.lst')) {
			$buff=file('/hd/configs/fwdrule.lst');
			$buff[$line]="$value";
			$handle=fopen("/hd/configs/fwdrule.lst", "w");
			foreach($buff as $line) {
				$line=trim($line);
				if($line!='') {
					fputs($handle,"$line\n");
				}
			}
			fclose($handle);
		}
	}

	function update_natlist($value,$line) {
		if(file_exists('/hd/configs/natrule.lst')) {
			$buff=file('/hd/configs/natrule.lst');
			$buff[$line]="$value";
			$handle=fopen("/hd/configs/natrule.lst", "w");
			foreach($buff as $line) {
				$line=trim($line);
				if($line!='') {
					fputs($handle,"$line\n");
				}
			}
			fclose($handle);
		}
	}

	function update_snatlist($value,$line) {
		if(file_exists('/hd/configs/snatrule.lst')) {
			$buff=file('/hd/configs/snatrule.lst');
			$buff[$line]="$value";
			$handle=fopen("/hd/configs/snatrule.lst", "w");
			foreach($buff as $line) {
				$line=trim($line);
				if($line!='') {
					fputs($handle,"$line\n");
				}
			}
			fclose($handle);
		}
	}

	function update_stringlist($value,$line) {
		if(file_exists('/hd/configs/strrule.lst')) {
			$buff=file('/hd/configs/strrule.lst');
			$buff[$line]="$value";
			$handle=fopen("/hd/configs/strrule.lst", "w");
			foreach($buff as $line) {
				$line=trim($line);
				if($line!='') {
					fputs($handle,"$line\n");
				}
			}
			fclose($handle);
		}
	}

	function write_fwdlist($value,$rule) {
		if($rule==0) {
			$handle=fopen("/hd/configs/fwdrule.lst", "a");
			fputs($handle,"$value\n");
			fclose($handle);
		} else {
			if(file_exists('/hd/configs/fwdrule.lst')) {
				$buff=file('/hd/configs/fwdrule.lst');
				$handle=fopen("/hd/configs/fwdrule.lst", "w");
				fputs($handle,"$value\n");
				foreach($buff as $line) {
					if($line!='') {
						$line=trim($line);
						fputs($handle,"$line\n");
					}
				}
				fclose($handle);
			} else {
				$handle=fopen("/hd/configs/fwdrule.lst", "w");
				fputs($handle,"$value\n");
				fclose($handle);
			}
		}
	}

	function write_natlist($value,$rule) {
		if($rule==0) {
			$handle=fopen("/hd/configs/natrule.lst", "a");
			fputs($handle,"$value\n");
			fclose($handle);
		} else {
			if(file_exists('/hd/configs/natrule.lst')) {
				$buff=file('/hd/configs/natrule.lst');
				$handle=fopen("/hd/configs/natrule.lst", "w");
				fputs($handle,"$value\n");
				foreach($buff as $line) {
					if($line!='') {
						$line=trim($line);

						fputs($handle,"$line\n");
					}
				}
				fclose($handle);
			} else {
				$handle=fopen("/hd/configs/natrule.lst", "w");
				fputs($handle,"$value\n");
				fclose($handle);
			}
		}
	}

	function write_snatlist($value,$rule) {
		if($rule==0) {
			$handle=fopen("/hd/configs/snatrule.lst", "a");
			fputs($handle,"$value\n");
			fclose($handle);
		} else {
			if(file_exists('/hd/configs/snatrule.lst')) {
				$buff=file('/hd/configs/snatrule.lst');
				$handle=fopen("/hd/configs/snatrule.lst", "w");
				fputs($handle,"$value\n");
				foreach($buff as $line) {
					if($line!='') {
						$line=trim($line);

						fputs($handle,"$line\n");
					}
				}
				fclose($handle);
			} else {
				$handle=fopen("/hd/configs/snatrule.lst", "w");
				fputs($handle,"$value\n");
				fclose($handle);
			}
		}
	}

	function write_customlist($list0,$list1,$list2,$list3) {
		$list=preg_split("/,/",$list0);
		$handle=fopen("/hd/configs/fwcustd.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);

		$list=preg_split("/,/",$list1);
		$handle=fopen("/hd/configs/fwcusta.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);

		$list=preg_split("/,/",$list2);
		$handle=fopen("/hd/configs/fwcusod.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);

		$list=preg_split("/,/",$list3);
		$handle=fopen("/hd/configs/fwcusoa.lst", "w");
		for($x='0';$x<count($list);$x++) {
			$pp=rtrim($list[$x],',');
			fputs($handle,"$pp\n");
		}
		fclose($handle);
	}

	function write_stringlist($value,$rule) {
		if($rule==0) {
			$handle=fopen("/hd/configs/strrule.lst", "a");
			fputs($handle,"$value\n");
			fclose($handle);
		} else {
			if(file_exists('/hd/configs/strrule.lst')) {
				$buff=file('/hd/configs/strrule.lst');
				$handle=fopen("/hd/configs/strrule.lst", "w");
				fputs($handle,"$value\n");
				foreach($buff as $line) {
					if($line!='') {
						$line=trim($line);
						fputs($handle,"$line\n");
					}
				}
				fclose($handle);
			} else {
				$handle=fopen("/hd/configs/strrule.lst", "w");
				fputs($handle,"$value\n");
				fclose($handle);
			}
		}
	}

	function change_stat($file,$what,$line,$val) {
		switch($file) {
			case 'filter':
				if(file_exists('/hd/configs/fwrule.lst')) {
					$file=file('/hd/configs/fwrule.lst');
					$pbuf=$file[$line];
					list($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$log,$active,$rule)=explode('|',$pbuf);
					if($what=='active') {
						$active=$val;
					}
					if($what=='action') {
						$action=$val;
					}
					if($what=='log') {
						$log=$val;
					}
					$buff="$src_ip|$src_po|$des_ip|$des_po|$proto|$traffic|$action|$desc|$log|$active|$rule";
					$file[$line]="$buff";
					$handle=fopen('/hd/configs/fwrule.lst','w');
					foreach($file as $line) {
						if($line!='') {
							$line=trim($line);
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'nat':
				if(file_exists('/hd/configs/natrule.lst')) {
					$file=file('/hd/configs/natrule.lst');
					$pbuf=$file[$line];
					list($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log,$active,$rule)=explode('|',$pbuf);
					if($what=='active') {
						$active=$val;
					}
					if($what=='log') {
						$log=$val;
					}
					$buff="$src_ip|$src_po|$des_ip|$des_po|$fw_ip|$fw_po|$proto|$desc|$log|$active|$rule";
					$file[$line]="$buff";
					$handle=fopen('/hd/configs/natrule.lst','w');
					foreach($file as $line) {
						if($line!='') {
							$line=trim($line);
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'snat':
				if(file_exists('/hd/configs/snatrule.lst')) {
					$file=file('/hd/configs/snatrule.lst');
					$pbuf=$file[$line];
					list($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log,$active,$rule)=explode('|',$pbuf);
					if($what=='active') {
						$active=$val;
					}
					if($what=='log') {
						$log=$val;
					}
					$buff="$src_ip|$src_po|$des_ip|$des_po|$fw_ip|$fw_po|$proto|$desc|$log|$active|$rule";
					$file[$line]="$buff";
					$handle=fopen('/hd/configs/snatrule.lst','w');
					foreach($file as $line) {
						if($line!='') {
							$line=trim($line);
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'fwd':
				if(file_exists('/hd/configs/fwdrule.lst')) {
					$file=file('/hd/configs/fwdrule.lst');
					$pbuf=$file[$line];
					list($int,$ont,$src_ip,$src_po,$des_ip,$des_po,$proto,$action,$desc,$log,$active,$rule)=preg_split("/\|/",$pbuf);
					if($what=='active') {
						$active=$val;
					}
					if($what=='action') {
						$action=$val;
					}
					if($what=='log') {
						$log=$val;
					}
					$buff="$int|$ont|$src_ip|$src_po|$des_ip|$des_po|$proto|$action|$desc|$log|$active|$rule";
					$file[$line]="$buff";
					$handle=fopen('/hd/configs/fwdrule.lst','w');
					foreach($file as $line) {
						if($line!='') {
							$line=trim($line);
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'shaper':
				if(file_exists('/hd/configs/bandwidth.lst')) {
					$file=file('/hd/configs/bandwidth.lst');
					$pbuf=$file[$line];
					list($src,$trans,$que,$desc,$active)=preg_split("/\|/",$pbuf);
					if($what=='active') {
						$active=$val;
					}
					if($what=='log') {
						$log=$val;
					}
					$buff="$src|$trans|$que|$desc|$active";
					$file[$line]="$buff";
					$handle=fopen('/hd/configs/bandwidth.lst','w');
					foreach($file as $line) {
						if($line!='') {
							$line=trim($line);
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'string':
				if(file_exists('/hd/configs/strrule.lst')) {
					$file=file('/hd/configs/strrule.lst');
					$pbuf=$file[$line];
					list($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$string,$log,$active,$rule)=explode('|',$pbuf);
					if($what=='active') {
						$active=$val;
					}
					if($what=='action') {
						$action=$val;
					}
					if($what=='log') {
						$log=$val;
					}
					$buff="$src_ip|$src_po|$des_ip|$des_po|$proto|$traffic|$action|$desc|$string|$log|$active|$rule";
					$file[$line]="$buff";
					$handle=fopen('/hd/configs/strrule.lst','w');
					foreach($file as $line) {
						if($line!='') {
							$line=trim($line);
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
		}
	}

	function do_delete($file,$del) {
		switch($file) {
			case 'filter':
				if(file_exists('/hd/configs/fwrule.lst')&&$del!='') {
					$file=file('/hd/configs/fwrule.lst');
					foreach($del as $line) {
						$file[$line]='';
					}
					$handle=fopen('/hd/configs/fwrule.lst','w');
					foreach($file as $line) {
						$line=trim($line);
						if($line!='') {
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'nat':
				if(file_exists('/hd/configs/natrule.lst')&&$del!='') {

					$file=file('/hd/configs/natrule.lst');
					foreach($del as $line) {
						$file[$line]='';
					}
					$handle=fopen('/hd/configs/natrule.lst','w');
					foreach($file as $line) {
						$line=trim($line);
						if($line!='') {
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'snat':
				if(file_exists('/hd/configs/snatrule.lst')&&$del!='') {

					$file=file('/hd/configs/snatrule.lst');
					foreach($del as $line) {
						$file[$line]='';
					}
					$handle=fopen('/hd/configs/snatrule.lst','w');
					foreach($file as $line) {
						$line=trim($line);
						if($line!='') {
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'fwd':
				if(file_exists('/hd/configs/fwdrule.lst')&&$del!='') {
					$file=file('/hd/configs/fwdrule.lst');
					foreach($del as $line) {
						$file[$line]='';
					}
					$handle=fopen('/hd/configs/fwdrule.lst','w');
					foreach($file as $line) {
						$line=trim($line);
						if($line!='') {
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'shaper':
				if(file_exists('/hd/configs/bandwidth.lst')&&$del!='') {
					$file=file('/hd/configs/bandwidth.lst');
					foreach($del as $line) {
						$buff=$file[$line];
						if($buff!='') {
							// remove from kernel
							list($src,$trans,$que,$desc,$active)=preg_split("/\|/",$buff);
							shell_exec("/usr/sbin/rshaperctl $src 0");
						}
						$file[$line]='';
					}
					$handle=fopen('/hd/configs/bandwidth.lst','w');
					foreach($file as $line) {
						$line=trim($line);
						if($line!='') {
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
			case 'string':
				if(file_exists('/hd/configs/strrule.lst')&&$del!='') {
					$file=file('/hd/configs/strrule.lst');
					foreach($del as $line) {
						$file[$line]='';
					}
					$handle=fopen('/hd/configs/strrule.lst','w');
					foreach($file as $line) {
						$line=trim($line);
						if($line!='') {
							fputs($handle,"$line\n");
						}
					}
					fclose($handle);
				}
			break;
		}	
	}
        function upline($file,$line) {
		switch($file) {
			case 'filter':
			if(file_exists('/hd/configs/fwrule.lst')) {
                		$lines=file('/hd/configs/fwrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=0) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line-1];
                        		$lines[$line-1]=$l1;
                        		$lines[$line]=$l2;
				}
				if($line==0) {

					$l1=$lines[$cnt];
                        		$l2=$lines[0];
                        		$lines[0]=$l1;
                        		$lines[$cnt]=$l2;
				}
				$handle=fopen('/hd/configs/fwrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
          		}
			break;
			case 'nat':
			if(file_exists('/hd/configs/natrule.lst')) {
                		$lines=file('/hd/configs/natrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=0) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line-1];
                        		$lines[$line-1]=$l1;
                        		$lines[$line]=$l2;
				}
				if($line==0) {
					$l1=$lines[$cnt];
                        		$l2=$lines[0];
                        		$lines[0]=$l1;
                        		$lines[$cnt]=$l2;
				}
				$handle=fopen('/hd/configs/natrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
          		}
			break;
			case 'snat':
			if(file_exists('/hd/configs/snatrule.lst')) {
                		$lines=file('/hd/configs/snatrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=0) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line-1];
                        		$lines[$line-1]=$l1;
                        		$lines[$line]=$l2;
				}
				if($line==0) {
					$l1=$lines[$cnt];
                        		$l2=$lines[0];
                        		$lines[0]=$l1;
                        		$lines[$cnt]=$l2;
				}
				$handle=fopen('/hd/configs/snatrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
          		}
			break;
			case 'fwd':
			if(file_exists('/hd/configs/fwdrule.lst')) {
                		$lines=file('/hd/configs/fwdrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=0) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line-1];
                        		$lines[$line-1]=$l1;
                        		$lines[$line]=$l2;
				}
				if($line==0) {
					$l1=$lines[$cnt];
                        		$l2=$lines[0];
                        		$lines[0]=$l1;
                        		$lines[$cnt]=$l2;
				}
				$handle=fopen('/hd/configs/fwdrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {

						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
          		}
			break;
			case 'shaper':
			if(file_exists('/hd/configs/bandwidth.lst')) {
                		$lines=file('/hd/configs/bandwidth.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=0) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line-1];
                        		$lines[$line-1]=$l1;
                        		$lines[$line]=$l2;
				}
				if($line==0) {
					$l1=$lines[$cnt];
                        		$l2=$lines[0];
                        		$lines[0]=$l1;
                        		$lines[$cnt]=$l2;
				}
				$handle=fopen('/hd/configs/bandwidth.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
          		}
			break;
			case 'string':
			if(file_exists('/hd/configs/strrule.lst')) {
                		$lines=file('/hd/configs/strrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=0) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line-1];
                        		$lines[$line-1]=$l1;
                        		$lines[$line]=$l2;
				}
				if($line==0) {

					$l1=$lines[$cnt];
                        		$l2=$lines[0];
                        		$lines[0]=$l1;
                        		$lines[$cnt]=$l2;
				}
				$handle=fopen('/hd/configs/strrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
          		}
			break;
		}
        }

        function downline($file,$line) {
		switch($file) {
			case 'filter':
         		if(file_exists('/hd/configs/fwrule.lst')) {
                		$lines=file('/hd/configs/fwrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=$cnt) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line+1];
                        		$lines[$line+1]=$l1;
                        		$lines[$line]=$l2;
                		}
				if($line==$cnt) {
					$ll=$lines[0];
                    			$lines[0]=$lines[$line];
					$lines[$cnt]=$ll;
				}
				$handle=fopen('/hd/configs/fwrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
			}
			break;
			case 'nat':
         		if(file_exists('/hd/configs/natrule.lst')) {
                		$lines=file('/hd/configs/natrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=$cnt) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line+1];
                        		$lines[$line+1]=$l1;
                        		$lines[$line]=$l2;
                		}
				if($line==$cnt) {
					$ll=$lines[0];
                    			$lines[0]=$lines[$line];
					$lines[$cnt]=$ll;
				}
				$handle=fopen('/hd/configs/natrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
			}
			break;
			case 'snat':
         		if(file_exists('/hd/configs/snatrule.lst')) {
                		$lines=file('/hd/configs/snatrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=$cnt) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line+1];
                        		$lines[$line+1]=$l1;
                        		$lines[$line]=$l2;
                		}
				if($line==$cnt) {
					$ll=$lines[0];
                    			$lines[0]=$lines[$line];
					$lines[$cnt]=$ll;
				}
				$handle=fopen('/hd/configs/snatrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
			}
			break;
			case 'fwd':
         		if(file_exists('/hd/configs/fwdrule.lst')) {
                		$lines=file('/hd/configs/fwdrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=$cnt) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line+1];
                        		$lines[$line+1]=$l1;
                        		$lines[$line]=$l2;
                		}
				if($line==$cnt) {
					$ll=$lines[0];
                    			$lines[0]=$lines[$line];
					$lines[$cnt]=$ll;
				}
				$handle=fopen('/hd/configs/fwdrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
			}
			break;
			case 'shaper':
         		if(file_exists('/hd/configs/bandwidth.lst')) {
                		$lines=file('/hd/configs/bandwidth.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=$cnt) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line+1];
                        		$lines[$line+1]=$l1;
                        		$lines[$line]=$l2;
                		}
				if($line==$cnt) {
					$ll=$lines[0];
                    			$lines[0]=$lines[$line];
					$lines[$cnt]=$ll;
				}
				$handle=fopen('/hd/configs/bandwidth.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
			}
			break;
			case 'string':
         		if(file_exists('/hd/configs/strrule.lst')) {
                		$lines=file('/hd/configs/strrule.lst');
                		$cnt=count($lines);
                		//$line=$line-1;
                		if($line!=$cnt) {
                        		$l1=$lines[$line];
                        		$l2=$lines[$line+1];
                        		$lines[$line+1]=$l1;
                        		$lines[$line]=$l2;
                		}
				if($line==$cnt) {
					$ll=$lines[0];
                    			$lines[0]=$lines[$line];
					$lines[$cnt]=$ll;
				}
				$handle=fopen('/hd/configs/strrule.lst','w');
                		for($x='0';$x<count($lines);$x++) {
					$line=trim($lines[$x]);
					if($line!='') {
						fputs($handle,"$line\n");
					}
                		}
				fclose($handle);
			}
			break;
		}
        }

	function go_redirect($where,$msg) {
		print("<html><head><title>MyBox Firewall - Redirecting...</title>\n");
		include_once('scripts/style.css');
		print("</head>\n");
		print("<body bgcolor=\"#E7E9F2\" oncontextmenu=\"return false\" oncontextmenu=\"return false\" scroll=\"no\">\n");
		print("<center><font class=td_pad>$msg</font>\n");
		include_once('scripts/timerbar.js');
		print("</center></body></html>\n\n");
		exit;
	}
	function go_exit() {
		print("<html><head><title>MyBox Firewall - Authorization Required</title>\n");
		include_once('scripts/style.css');
		print("<script language=\"javascript\"><!--\n");
		print("if (top.frames.length!=0) top.location=self.document.location;\n//--></script>\n</head>\n");
		print("<body bgcolor=\"#E7E9F2\" oncontextmenu=\"return false\" oncontextmenu=\"return false\" scroll=\"no\">\n");
		print("<center><font class=td_pad>Session Expired. Please login back</font></center>\n");
		print("<META HTTP-EQUIV=\"refresh\" content=\"3; url=/logout.so\">\n");
		print("</body></html>\n\n");
		exit;
	}
	function go_in() {
		print("<script language='javascript'><!--\n");
		print("top.location.href='/system_session.so'\n");
		print("</script>\n\n");
		exit;
	}
	function write_nocache() {
		if(!headers_sent()) { 
        		header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
        		header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
        		header("Cache-Control: no-store, no-cache, must-revalidate");
        		header("Cache-Control: post-check=0, pre-check=0",false);
        		header("Pragma: no-cache");
		}
		flush();
	}

	function get_net($src) {
		if(preg_match("/(\d+\.\d+\.\d+)\.\d+/",$src,$ip)) {
			return $ip[1];
		}
		return 0;
	}
	function check_mynetwork($src) {
		// 1 =  true; 0 = false
		$extip=get_net(get_ini('NETWORK','OUTSIDE_IP'));
		$intip=get_net(get_ini('NETWORK','INSIDE_IP'));
		$dmz1ip=get_net(get_ini('NETWORK','DMZ1_IP'));
		$dmz2ip=get_net(get_ini('NETWORK','DMZ2_IP'));
		if((get_net($src)==$extip) || (get_net($src)==$intip) || (get_net($src)==$dmz1ip) || (get_net($src)==$dmz2ip)) {
			return 1;
		}
		return 0;
	}

	function check_src_shaper($src) {
		if(file_exists('/hd/configs/bandwidth.lst')) {
			$handle=fopen('/hd/configs/bandwidth.lst','r');
			while(!feof($handle)) {
				$buff=fgets($handle,4096);
				$buff=trim($buff);
				if($buff!='') {
					list($ip,$trans,$que,$descr)=preg_split("/\|/",$buff);
					if($src==$ip) {
						return 1;
					}
				}
			}
			fclose($handle);
		}
		return 0;
	}
	function write_bandwidthlist($val) {
		$handle=fopen('/hd/configs/bandwidth.lst','a');
		$val=trim($val);
		fputs($handle,"$val\n");
		fclose($handle);
	}
	function get_servip() {
		$servip=gethostbyname($_SERVER["SERVER_NAME"]);
		$port=get_ini('MINISERV','PORT');
		return "https://$servip:$port";
	}
	function get_online() {
		$ip=getenv("REMOTE_ADDR");
		$agent=getenv("HTTP_USER_AGENT");
		echo "$ip - $agent";
	}
	function _mktime($date,$time) {
		list($d,$m,$y)=preg_split("/\//",$date);
		list($h,$m,$s)=preg_split("/:/",$time);
		$stime=mktime($h,$m,$s,$m,$d,$y);
		return date("h:i:s a",$stime);
	}
?>

