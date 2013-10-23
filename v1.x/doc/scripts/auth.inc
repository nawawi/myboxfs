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
// $Id: auth.inc,v 1.00 2003/07/28 1:07 AM nawawi Exp $
//

	function check_login($password) {
		if($password=='') return 0;
		$pass0=get_ini('MINISERV','ADMIN');
		$pass1=get_ini('MINISERV','LOOK');
                $pass2=md5(crypt($password,$password));
                if($pass2=="$pass0" || $pass2=="$pass1") {
			unset($password);
			set_cookie($pass2);
                        return 1;
                }
		return 0;
	}

	function set_cookie($pwd) {
		$rand=crypt(mt_rand(0, rand())/rand());
		$rand=md5(crypt($rand));
		$remote_addr=getenv("REMOTE_ADDR");
		$timeout=get_ini('MINISERV','SESSTIMEOUT');
		if($timeout=='') $timeout=1;
		$time_in=time()+3600*$timeout;
		$access0=get_ini('MINISERV','ADMIN');
		$access1=get_ini('MINISERV','LOOK');
		if($pwd==$access0) $access='0';
		if($pwd==$access1) $access='1';
		if(!is_dir('/tmp/_session')) {
			mkdir('/tmp/_session',0700);
			clearstatcache();
		}
		if(file_exists('/tmp/_session/session.lst')) {
			$handle=fopen('/tmp/_session/session.lst','a');
			fputs($handle,"$rand|$remote_addr|$time_in|$access|\n");
			fclose($handle);
		} else {
			$handle=fopen('/tmp/_session/session.lst','w');
			fputs($handle,"$rand|$remote_addr|$time_in|$access|\n");
			fclose($handle);
		}
		if(!setcookie("session_uid",$rand,$time_in,"/","", 1)) {
			echo "error on cookie set.";
			exit;
		} 
	}

	function chk_session() {
		global $_COOKIE;
		$uid=$_COOKIE["session_uid"];
		$OK='';
		if($uid!='') {
			if(file_exists('/tmp/_session/session.lst')) {
				$buff=file('/tmp/_session/session.lst');
				$cnt=0;
				$handle=fopen('/tmp/_session/session.lst','w');
				foreach($buff as $line) {
					$line=trim($line);
					if($line!='') {
						list($id,$ip,$time,$access)=explode('|',$line);
						if(time() >= $time) {
							if($id==$uid) {
								write_userexprlog($id,$ip,$access,'Session Expired',"Auto logout in 15 seconds"); 
								delete_cookie();
							}
						} else {
							if($id==$uid) {
								if($ip==getenv("REMOTE_ADDR")) {
									$OK=$access;
								} else {
									write_userexprlog($id,$ip,$access,'Session Expired -> IP not match',"Auto logout in 15 seconds");
								}
							}
							fputs($handle,"$line\n");
						}
					}
				$cnt++;
				}
				fclose($handle);
			}
		}
		return $OK;
	}
	function do_logout() {
		global $_COOKIE;
		$uid=$_COOKIE["session_uid"];
		if($uid!='') {
			if(file_exists('/tmp/_session/session.lst')) {
				$buff=file('/tmp/_session/session.lst');
				$cnt=0;
				$handle=fopen('/tmp/_session/session.lst','w');
				foreach($buff as $line) {
					$line=trim($line);
					if($line!='') {
						list($id,$ip,$time,$access)=explode('|',$line);
						if($id!=$uid) {
							fputs($handle,"$line\n");
						}
					}
				$cnt++;
				}
				fclose($handle);
			}
		}
		delete_cookie();
	}
	function delete_cookie() {
		setcookie("session_uid","");
	}

	function write_userlog($action,$status) {
		$date=date('d/m/Y');
		$time=date('h:i:s a');
		$ldate=date('y-m-d');
		$logdir=get_ini('MINISERV','REPORT');
		if($logdir=='NULL' || $logdir=='') {
			$logdir='/hd/reports/auth';
			write_ini('MINISERV','REPORT',$logdir);
		}
		if(!is_dir($logdir)) {
			shell_exec("mkdir -p $logdir");
		}
		$access=chk_session();
		if($access=='') {
			$access='Restricted';
		}
		if($action=='Session logout' && $status=='Session removed' && $access=='Restricted') {
			// no access, no session: dont need to log
			return;
		}
		$access=str_replace('0','Admin',$access);
		$access=str_replace('1','Look',$access);
		$log="$logdir/$ldate.log";
		$ip=getenv("REMOTE_ADDR");
		$handle=fopen($log, "a");
		fputs($handle,"DATE=$date TIME=$time SRC_IP=$ip ACCESS=$access ACTION=$action STATUS=$status\n");
		fflush($handle);
		fclose($handle);
		$lyear=date('Y');
		$syear=date('y');
		$year="$syear:$lyear";
		$w=1;
		if(file_exists("$logdir/year.lst")) {
			$handle=fopen("$logdir/year.lst",'r');
			while(!feof($handle)) {
				$buff=fgets($handle,150);
				$buff=trim($buff);
				if($buff==$year) $w=0;
			}
			fclose($handle);
		}
		if($w==1) {
			$handle=fopen("$logdir/year.lst",'a');
			fputs($handle,"$year\n");
			fflush($handle);
			fclose($handle);
		}
	}
	function write_userexprlog($id,$ip,$access,$action,$status) {
		$date=date('d/m/Y');
		$time=date('h:i:s a');
		$ldate=date('y-m-d');
		$logdir=get_ini('MINISERV','REPORT');
		if($logdir=='NULL' || $logdir=='') {
			$logdir='/hd/reports/auth';
			write_ini('MINISERV','REPORT',$logdir);
		}
		if(!is_dir($logdir)) {
			shell_exec("mkdir -p $logdir");
		}
		$access=str_replace('0','Admin',$access);
		$access=str_replace('1','Look',$access);
		$log="$logdir/$ldate.log";
		$ip=getenv("REMOTE_ADDR");
		$handle=fopen($log, "a");
		fputs($handle,"DATE=$date TIME=$time SRC_IP=$ip ACCESS=$access ACTION=$action STATUS=$status\n");
		fflush($handle);
		fclose($handle);
		$lyear=date('Y');
		$syear=date('y');
		$year="$syear:$lyear";
		$w=1;
		if(file_exists("$logdir/year.lst")) {
			$handle=fopen("$logdir/year.lst",'r');
			while(!feof($handle)) {
				$buff=fgets($handle,150);
				$buff=trim($buff);
				if($buff==$year) $w=0;
			}
			fclose($handle);
		}
		if($w==1) {
			$handle=fopen("$logdir/year.lst",'a');
			fputs($handle,"$year\n");
			fflush($handle);
			fclose($handle);
		}
	}

	function is_admin($access) {
		if($access=='0') {
			return 0;
		} elseif($access=='1') {
			return 1;
		} else {
			// no privilege
			return 2;
		}
	}
?>