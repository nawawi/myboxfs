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
// $Id: rules.inc,v 3.00 2003/07/28 1:07 AM nawawi Exp $
//
// History: 04/10/2004 - By default OUTPUT is accept 
//	    23/01/2005 - add snat 
//
	// global variable.
	$INSIDE_DEV=get_ini('NETWORK','INSIDE_DEV');
	$OUTSIDE_DEV=get_ini('NETWORK','OUTSIDE_DEV');
	$DMZ1_DEV=get_ini('NETWORK','DMZ1_DEV');
	$DMZ2_DEV=get_ini('NETWORK','DMZ2_DEV');
	global $INSIDE_DEV,$OUTSIDE_DEV,$DMZ1_DEV,$DMZ2_DEV;

	function do_iptable($value) {
		shell_exec("/sbin/iptables $value >/dev/null 2>&1");
		flush();
	}

	function do_flush() {
		global $INSIDE_DEV,$OUTSIDE_DEV,$DMZ1_DEV,$DMZ2_DEV;
		$set=get_ini('IPSYSCTL','LIST');
		list($sys0,$sys1,$sys2,$sys3,$sys4,$sys5,$sys6,$sys7,$sys8,$sys9)=preg_split("/,/",$set);
		do_iptable("-F");
		if(file_exists('/proc/net/ip_tables_names')) {
			$handle=fopen("/proc/net/ip_tables_names","r");
			while(!feof ($handle)) {
    				$buffer=fgets($handle, 4096);
    				$buffer=trim($buffer);
				if($buffer!='') {
					do_iptable("-F -t $buffer");
				}
			}
			fclose ($handle);
		}
		do_iptable("-X");
		do_iptable("-Z");
		do_iptable("-P INPUT DROP");
		// 04/10/2004 - awie
		do_iptable("-P OUTPUT ACCEPT");
		do_iptable("-P FORWARD DROP");
		do_iptable("-A INPUT -i lo -j ACCEPT");
		//do_iptable("-A OUTPUT -o lo -j ACCEPT");
		//do_iptable("-A OUTPUT -p udp -m udp --dport 25 -j ACCEPT");
		//do_iptable("-A OUTPUT -p udp -m udp --dport 53 -j ACCEPT");
		//do_iptable("-A OUTPUT -p tcp -m tcp --dport 25 -j ACCEPT");
		//do_iptable("-A OUTPUT -p tcp -m tcp --dport 53 -j ACCEPT");
		//do_iptable("-A OUTPUT -p tcp -m tcp --dport 21 -j ACCEPT");
		//do_iptable("-A OUTPUT -p udp -m udp --dport 135:139 -j ACCEPT");
		//do_iptable("-A OUTPUT -p tcp -m tcp --dport 135:139 -j ACCEPT");
		//do_iptable("-A OUTPUT -p udp -m udp --dport 161 -j ACCEPT");
		//do_iptable("-A OUTPUT -p tcp -m tcp --dport 161 -j ACCEPT");
		//do_iptable("-A OUTPUT -p udp -m udp --dport 33434:33600 -j ACCEPT"); // traceroute
		do_iptable("-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT");
		filter_snat();
		do_iptable("-t nat -A POSTROUTING -o $OUTSIDE_DEV -j MASQUERADE");
		do_iptable("-t nat -A POSTROUTING -o $DMZ1_DEV -j MASQUERADE");
		do_iptable("-t nat -A POSTROUTING -o $DMZ2_DEV -j MASQUERADE");
		if($sys0==1) {
			do_iptable("-A INPUT -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec -j ACCEPT");
			do_iptable("-A INPUT -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec -j ACCEPT");
			do_iptable("-A INPUT -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec -j ACCEPT");
			do_iptable("-A INPUT -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec -j ACCEPT");
			// output
			do_iptable("-A OUTPUT -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec -j ACCEPT");
			do_iptable("-A OUTPUT -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec -j ACCEPT");
			do_iptable("-A OUTPUT -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec -j ACCEPT");
			do_iptable("-A OUTPUT -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec -j ACCEPT");
		}
		do_iptable("-A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT");
		// block reset
		touch('/tmp/block_reset');
	}
	
	function custom_filter() {
		global $INSIDE_DEV,$OUTSIDE_DEV,$DMZ1_DEV,$DMZ2_DEV;
		if(file_exists('/hd/configs/fwcusta.lst')) {
			$handle=fopen('/hd/configs/fwcusta.lst','r');
			while(!feof($handle)) {
				$buff=fgets($handle,550);
				$buff=trim($buff);
				list($name,$service)=preg_split("/->/",$buff);
				if($service!='') {
					list($port,$proto)=preg_split("/\//",$service);
					if($port!='ALL') {
						do_iptable("-A INPUT -i $OUTSIDE_DEV -p $proto -m $proto --dport $port -j ACCEPT");
					} else {
						do_iptable("-P INPUT ACCEPT");
					}
				}
			}
			fclose($handle);
		}
		if(file_exists('/hd/configs/fwcusoa.lst')) {
			$handle=fopen('/hd/configs/fwcusoa.lst','r');
			while(!feof($handle)) {
				$buff=fgets($handle,550);
				$buff=trim($buff);
				list($name,$service)=preg_split("/->/",$buff);
				if($service!='') {
					list($port,$proto)=preg_split("/\//",$service);
					if($port!='ALL') {
						do_iptable("-A OUTPUT -o $OUTSIDE_DEV -p $proto -m $proto --dport $port -j ACCEPT");
					} else {
						do_iptable("-P OUTPUT ACCEPT");
					}
				}
			}
			fclose($handle);
		}
	}

	function webadmin_enable() {
		global $INSIDE_DEV,$OUTSIDE_DEV,$DMZ1_DEV,$DMZ2_DEV;
		$conn=get_ini('MINISERV','CONNECTION');
		$network=get_ini('NETWORK','INSIDE_NETWORK');
		$nmask=get_ini('NETWORK','INSIDE_NETMASK');
		$port=get_ini('MINISERV','PORT');
		$ipallow=get_ini('MINISERV','IPALLOW');
		if($ipallow=="NULL" || $ipallow=='') {
			do_iptable("-I INPUT -s $network/$nmask -p tcp -m tcp --dport $port -j ACCEPT");
			do_iptable("-I OUTPUT -s $network/$nmask -p tcp -m tcp --dport $port -j ACCEPT");
		} else {
			$iplist=preg_split("/,/",$ipallow);
			foreach($iplist as $list) {
				$list=trim($list);
				if($list!='') {
					if($conn!="NULL" && $conn!='') {
						list($wan,$dmz1,$dmz2,$lan)=preg_split("/,/",$conn);
						if($wan==1) {
							do_iptable("-I INPUT -s $list -i $OUTSIDE_DEV -p tcp -m tcp --dport $port -j ACCEPT");
							do_iptable("-I OUTPUT -s $list -o $OUTSIDE_DEV -p tcp -m tcp --dport $port -j ACCEPT");
						}
						if($dmz1==1) {
							do_iptable("-I INPUT -s $list -i $DMZ1_DEV -p tcp -m tcp --dport $port -j ACCEPT");
							do_iptable("-I OUTPUT -s $list -o $DMZ1_DEV -p tcp -m tcp --dport $port -j ACCEPT");
						}
						if($dmz2==1) {
							do_iptable("-I INPUT -s $list -i $DMZ1_DEV -p tcp -m tcp --dport $port -j ACCEPT");
							do_iptable("-I OUTPUT -s $list -o $DMZ1_DEV -p tcp -m tcp --dport $port -j ACCEPT");
						}
						if($lan==1) {
							do_iptable("-I INPUT -s $list -i $INSIDE_DEV -p tcp -m tcp --dport $port -j ACCEPT");
							do_iptable("-I OUTPUT -s $list -o $INSIDE_DEV -p tcp -m tcp --dport $port -j ACCEPT");
						}
					} 
				}

			}
		}
		
	}

	function do_filter($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$log) {
		$sport='';
		$dport='';
		$protocol='';
		if($src_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$sport="--sport $src_po";
		}
		if($des_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$dport="--dport $des_po";
		}
		if($proto!='all') {
			$protocol="-p $proto";
		}
		if($proto=='tcp'||$proto=='udp'||$proto=='icmp') {
			$match="-m $proto";
		}
		if($src_ip!='any') {
			$src_ip="-s $src_ip";
		} else {
			unset($src_ip);
		}
		if($des_ip!='any') {
			$des_ip="-d $des_ip";
		} else {
			unset($des_ip);
		}
		if($log==1) {
			$laction=str_replace('DROP','D',$action);
			$laction=str_replace('ACCEPT','A',$action);
			do_iptable("-A $traffic $protocol $match $src_ip $sport $des_ip $dport -m state --state NEW -m limit --limit 1/sec -j LOG --log-level info --log-prefix \"FW:F:$laction:$desc \"");
		}
		if($proto=='icmp') {
			do_iptable("-A $traffic -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec $src_ip $sport $des_ip $dport -j $action");
			do_iptable("-A $traffic -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec $src_ip $sport $des_ip $dport -j $action");
			do_iptable("-A $traffic -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec $src_ip $sport $des_ip $dport -j $action");
			do_iptable("-A $traffic -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec $src_ip $sport $des_ip $dport -j $action");
		} else { 
			do_iptable("-A $traffic $protocol $match $src_ip $sport $des_ip $dport -j $action");
		}
	}

	function filter_rule() {
		if(file_exists('/hd/configs/fwrule.lst')) {
			$file=fopen('/hd/configs/fwrule.lst','r');
			while(!feof($file)) {
				$buff=fgets($file,4096);
				$buff=trim($buff);
				if($buff!='') {
					list($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$log,$active,$rule)=explode('|',$buff);
					if($active!=0) {
						$traffic=strtoupper($traffic);
						$action=str_replace('0','DROP',$action);
						$action=str_replace('1','ACCEPT',$action);
						do_filter($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$log);
					}
				}
			}
			fclose($file);
		}
	}

	function do_forward($int,$ont,$src_ip,$src_po,$des_ip,$des_po,$proto,$action,$desc,$log) {
		$sport='';
		$dport='';
		$protocol='';
		if($src_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$sport="--sport $src_po";
		}
		if($des_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$dport="--dport $des_po";
		}
		if($proto!='all') {
			$protocol="-p $proto";
		}
		if($proto=='tcp'||$proto=='udp'||$proto=='icmp') {
			$match="-m $proto";
		}
		$int=strtolower($int);
		if($int!='all') {
			$int="-i $int";
		} else {
			$int="-i eth+";
			//unset($int);
		}
		$ont=strtolower($ont);
		if($ont!='all') {
			$ont="-o $int";
		} else {
			$ont="-o eth+";
			//unset($ont);
		}
		if($src_ip!='any') {
			$src_ip="-s $src_ip";
		} else {
			unset($src_ip);
		}
		if($des_ip!='any') {
			$des_ip="-d $des_ip";
		} else {
			unset($des_ip);
		}
		if($log==1) {
			$laction=str_replace('DROP','D',$action);
			$laction=str_replace('ACCEPT','A',$action);
			do_iptable("-A FORWARD $int $ont $protocol $match $src_ip $sport $des_ip $dport -m state --state NEW -m limit --limit 1/sec -j LOG --log-level info --log-prefix \"FW:W:$laction:$desc \"");
		}
		if($proto=='icmp') {
 			do_iptable("-A FORWARD $int $ont -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec $src_ip $sport $des_ip $dport -j $action");
 			do_iptable("-A FORWARD $int $ont -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec $src_ip $sport $des_ip $dport -j $action");
 			do_iptable("-A FORWARD $int $ont -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec $src_ip $sport $des_ip $dport -j $action");
 			do_iptable("-A FORWARD $int $ont -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec $src_ip $sport $des_ip $dport -j $action");
		} else {
			do_iptable("-A FORWARD $int $ont $protocol $match $src_ip $sport $des_ip $dport -j $action");
		}
	}

	function forward_rule() {
		global $INSIDE_DEV,$OUTSIDE_DEV,$DMZ1_DEV,$DMZ2_DEV;
		do_iptable("-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT");
		// remove 01/11/2003
		//do_iptable("-A FORWARD -m state --state NEW,INVALID -i $OUTSIDE_DEV -j DROP");
		if(file_exists('/hd/configs/fwdrule.lst')) {
			$file=fopen('/hd/configs/fwdrule.lst','r');
			while(!feof($file)) {
				$buff=fgets($file,4096);
				$buff=trim($buff);
				if($buff!='') {
					list($int,$ont,$src_ip,$src_po,$des_ip,$des_po,$proto,$action,$desc,$log,$active,$rule)=preg_split("/\|/",$buff);
					if($active!=0) {
						$action=str_replace('0','DROP',$action);
						$action=str_replace('1','ACCEPT',$action);
						do_forward($int,$ont,$src_ip,$src_po,$des_ip,$des_po,$proto,$action,$desc,$log);
					}
				}
			}
			fclose($file);
		}

		do_iptable("-A FORWARD -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec -j ACCEPT");
		do_iptable("-A FORWARD -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec -j ACCEPT");
		do_iptable("-A FORWARD -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec -j ACCEPT");
		do_iptable("-A FORWARD -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec -j ACCEPT");
	}

	function do_nat($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log) {
		$sport='';
		$dport='';
		$protocol='';
		$destination='';
		if($src_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$sport="--sport $src_po";
		}
		if($des_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$dport="--dport $des_po";
		}
		if($fw_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$fport="$fw_po";
		}
		if($proto!='all') {
			$protocol="-p $proto";
		}
		if($proto=='tcp'||$proto=='udp'||$proto=='icmp') {
			$match="-m $proto";
			$destination="$fw_ip:$fport";
		} else {
			$destination="$fw_ip";
		}
		if($log==1) {
			do_iptable("-t nat -A PREROUTING $protocol $match -s $src_ip $sport -d $des_ip $dport -m state --state NEW -m limit --limit 1/sec -j LOG --log-level info --log-prefix \"FW:N:R:$desc \"");
		} 
		if($proto=='icmp') {
			do_iptable("-t nat -A PREROUTING -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec -s $src_ip $sport -d $des_ip $dport -j DNAT --to-destination $destination");			
			do_iptable("-t nat -A PREROUTING -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec -s $src_ip $sport -d $des_ip $dport -j DNAT --to-destination $destination");			
			do_iptable("-t nat -A PREROUTING -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec -s $src_ip $sport -d $des_ip $dport -j DNAT --to-destination $destination");			
			do_iptable("-t nat -A PREROUTING -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec -s $src_ip $sport -d $des_ip $dport -j DNAT --to-destination $destination");			
		} else {
			do_iptable("-t nat -A PREROUTING $protocol $match -s $src_ip $sport -d $des_ip $dport -j DNAT --to-destination $destination");
		}
	}

	function filter_nat() {
		if(file_exists('/hd/configs/natrule.lst')) {
			$file=fopen('/hd/configs/natrule.lst','r');
			while(!feof($file)) {
				$buff=fgets($file,4096);
				$buff=trim($buff);
				if($buff!='') {
					list($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log,$active,$rule)=explode('|',$buff);
					if($active!=0) {
						$src_ip=str_replace('any','0/0',$src_ip);
						$des_ip=str_replace('any','0/0',$des_ip);
						$fw_ip=str_replace('any','0/0',$fw_ip);
						do_nat($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log);
					}
				}
			}
			fclose($file);
		}
	}

	function do_snat($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log) {
		$sport='';
		$dport='';
		$protocol='';
		$destination='';
		if($src_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$sport="--sport $src_po";
		}
		if($des_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$dport="--dport $des_po";
		}
		if($fw_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$fport="$fw_po";
		}
		if($proto!='all') {
			$protocol="-p $proto";
		}
		if($proto=='tcp'||$proto=='udp'||$proto=='icmp') {
			$match="-m $proto";
			$destination="$fw_ip:$fport";
		} else {
			$destination="$fw_ip";
		}
		if($log==1) {
			do_iptable("-t nat -A POSTROUTING $protocol $match -s $src_ip $sport -d $des_ip $dport -m state --state NEW -m limit --limit 1/sec -j LOG --log-level info --log-prefix \"FW:N:R:$desc \"");
		} 
		if($proto=='icmp') {
			do_iptable("-t nat -A POSTROUTING -p icmp -m icmp --icmp-type 8 -m limit --limit 20/sec -s $src_ip $sport -d $des_ip $dport -j SNAT --to-source $destination");			
			do_iptable("-t nat -A POSTROUTING -p icmp -m icmp --icmp-type 0 -m limit --limit 10/sec -s $src_ip $sport -d $des_ip $dport -j SNAT --to-source $destination");			
			do_iptable("-t nat -A POSTROUTING -p icmp -m icmp --icmp-type 3 -m limit --limit 5/sec -s $src_ip $sport -d $des_ip $dport -j SNAT --to-source $destination");			
			do_iptable("-t nat -A POSTROUTING -p icmp -m icmp --icmp-type 30 -m limit --limit 10/sec -s $src_ip $sport -d $des_ip $dport -j SNAT --to-source $destination");			
		} else {
			do_iptable("-t nat -A POSTROUTING $protocol $match -s $src_ip $sport -d $des_ip $dport -j SNAT --to-source $destination");
		}
	}

	function filter_snat() {
		if(file_exists('/hd/configs/snatrule.lst')) {
			$file=fopen('/hd/configs/snatrule.lst','r');
			while(!feof($file)) {
				$buff=fgets($file,4096);
				$buff=trim($buff);
				if($buff!='') {
					list($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log,$active,$rule)=explode('|',$buff);
					if($active!=0) {
						$src_ip=str_replace('any','0/0',$src_ip);
						$des_ip=str_replace('any','0/0',$des_ip);
						$fw_ip=str_replace('any','0/0',$fw_ip);
						do_snat($src_ip,$src_po,$des_ip,$des_po,$fw_ip,$fw_po,$proto,$desc,$log);
					}
				}
			}
			fclose($file);
		}
	}

	function do_string($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$string,$log) {
		$sport='';
		$dport='';
		$protocol='';
		if($string=='') return;
		if($traffic=='') return;
		if($src_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$sport="--sport $src_po";
		}
		if($des_po!='any' && ($proto=='tcp'||$proto=='udp'||$proto=='icmp')) {
			$dport="--dport $des_po";
		}
		if($proto!='all') {
			$protocol="-p $proto";
		}
		if($proto=='tcp'||$proto=='udp'||$proto=='icmp') {
			$match="-m $proto";
		}
		if($src_ip!='any') {
			$src_ip="-s $src_ip";
		} else {
			unset($src_ip);
		}
		if($des_ip!='any') {
			$des_ip="-d $des_ip";
		} else {
			unset($des_ip);
		}
		if($traffic=='FORWARD') {
			$traffic="$traffic -i eth+ -o eth+";
		}

		if($proto=='icmp') {
			do_iptable("-I $traffic -p icmp -m icmp -m string --string \"$string\" --icmp-type 8 -m limit --limit 20/sec $src_ip $sport $des_ip $dport -j $action");
			do_iptable("-I $traffic -p icmp -m icmp -m string --string \"$string\" --icmp-type 0 -m limit --limit 10/sec $src_ip $sport $des_ip $dport -j $action");
			do_iptable("-I $traffic -p icmp -m icmp -m string --string \"$string\" --icmp-type 3 -m limit --limit 5/sec $src_ip $sport $des_ip $dport -j $action");
			do_iptable("-I $traffic -p icmp -m icmp -m string --string \"$string\" --icmp-type 30 -m limit --limit 10/sec $src_ip $sport $des_ip $dport -j $action");
		} else { 
			if($traffic!='ALL') {
				do_iptable("-I $traffic $protocol $match -m string --string \"$string\" $src_ip $sport $des_ip $dport -j $action");
			} else {
				do_iptable("-I FORWARD $protocol $match -m string --string \"$string\" $src_ip $sport $des_ip $dport -j $action");
				do_iptable("-I OUTPUT $protocol $match -m string --string \"$string\" $src_ip $sport $des_ip $dport -j $action");
				do_iptable("-I INPUT $protocol $match -m string --string \"$string\" $src_ip $sport $des_ip $dport -j $action");
			}
		}
		// string will filter on the top
		if($log==1) {
			$laction=str_replace('DROP','D',$action);
			$laction=str_replace('ACCEPT','A',$action);
			if($traffic!='ALL') {
				do_iptable("-I $traffic $protocol $match $src_ip $sport $des_ip $dport -m string --string \"$string\" -m limit --limit 3/sec -j LOG --log-level info --log-prefix \"FW:S:$laction:$desc \"");
			} else {
				do_iptable("-I FORWARD $protocol $match $src_ip $sport $des_ip $dport -m string --string \"$string\" -m limit --limit 3/sec -j LOG --log-level info --log-prefix \"FW:S:$laction:$desc \"");
				do_iptable("-I OUTPUT $protocol $match $src_ip $sport $des_ip $dport -m string --string \"$string\" -m limit --limit 3/sec -j LOG --log-level info --log-prefix \"FW:S:$laction:$desc \"");
				do_iptable("-I INPUT $protocol $match $src_ip $sport $des_ip $dport -m string --string \"$string\" -m limit --limit 3/sec -j LOG --log-level info --log-prefix \"FW:S:$laction:$desc \"");
			}
		}
	}

	function string_rule() {
		if(file_exists('/hd/configs/strrule.lst')) {
			$file=fopen('/hd/configs/strrule.lst','r');
			while(!feof($file)) {
				$buff=fgets($file,4096);
				$buff=trim($buff);
				if($buff!='') {
					list($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$string,$log,$active,$rule)=explode('|',$buff);
					if($active!=0) {
						$traffic=strtoupper($traffic);
						$action=str_replace('0','DROP',$action);
						$action=str_replace('1','ACCEPT',$action);
						$string=str_replace('','|',$string);
						do_string($src_ip,$src_po,$des_ip,$des_po,$proto,$traffic,$action,$desc,$string,$log);
					}
				}
			}
			fclose($file);
		}
	}

	function echo_dir($dir,$ff,$val) {
		if($handle = opendir("$dir")) { 
   			while(false !== ($file = readdir($handle))) { 
       				if($file != "." && $file != "..") { 
           				shell_exec("echo \"$val\" > $dir/$file/$ff"); 
       				}
   			}
   			closedir($handle); 
		}
	}

	function do_sysctl() {
		$set=get_ini('IPSYSCTL','LIST');
		if($set!=NULL) {
			list($sys0,$sys1,$sys2,$sys3,$sys4,$sys5,$sys6,$sys7,$sys8,$sys9)=preg_split("/,/",$set);
			// Accept PING request
			if($sys0==1) {
				shell_exec("echo \"0\" > /proc/sys/net/ipv4/icmp_echo_ignore_all");
			} else {
				shell_exec("echo \"1\" > /proc/sys/net/ipv4/icmp_echo_ignore_all");
			}

			// Enable dynamic IP address following 
			if($sys1==1) {
				shell_exec("echo \"7\" > /proc/sys/net/ipv4/ip_dynaddr");
			} else {
				shell_exec("echo \"0\" > /proc/sys/net/ipv4/ip_dynaddr");
			}

			// Enable smurf attacks protect 
			if($sys2==1) {
				shell_exec("echo \"1\" > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts");
			} else {
				shell_exec("echo \"0\" > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts");
			}

			// Don't accept source routed packets 
			if($sys3==1) {
				shell_exec("/bin/echo \"0\" > /proc/sys/net/ipv4/conf/all/accept_source_route");
			} else {
				shell_exec("/bin/echo \"1\" > /proc/sys/net/ipv4/conf/all/accept_source_route");
			}

			// Enable Syncookies 
			if($sys4==1) {
				shell_exec("echo \"1\" > /proc/sys/net/ipv4/tcp_syncookies");

			} else {
				shell_exec("echo \"0\" > /proc/sys/net/ipv4/tcp_syncookies");
			}

			// Disable IP spoofing
			if($sys5==1) {
				if(file_exists('/proc/sys/net/ipv4/conf/all/rp_filter')) {
					echo_dir('/proc/sys/net/ipv4/conf/','rp_filter','1');
				}
			} else {
				if(file_exists('/proc/sys/net/ipv4/conf/all/rp_filter')) {
					echo_dir('/proc/sys/net/ipv4/conf/','rp_filter','0');
				}
			}

			// Protect ICMP redirect
			if($sys6==1) {
				if(file_exists('/proc/sys/net/ipv4/conf/all/rp_filter')) {
					echo_dir('/proc/sys/net/ipv4/conf/','accept_redirects','0');
				}
			} else {
				if(file_exists('/proc/sys/net/ipv4/conf/all/rp_filter')) {
					echo_dir('/proc/sys/net/ipv4/conf/','accept_redirects','1');
				}
			}

			// Enable bad error message protection 
			if($sys7==1) {
				shell_exec("echo \"1\" > /proc/sys/net/ipv4/icmp_ignore_bogus_error_responses");
			} else {
				shell_exec("echo \"0\" > /proc/sys/net/ipv4/icmp_ignore_bogus_error_responses");
			}

			// Log packets with impossible addresses
			if($sys8==1) {
				shell_exec("echo \"1\" > /proc/sys/net/ipv4/conf/all/log_martians");
			} else {
				shell_exec("echo \"0\" > /proc/sys/net/ipv4/conf/all/log_martians");
			}

			// Enabling IP forwarding

			if($sys9==1) {
				shell_exec("echo \"1\" > /proc/sys/net/ipv4/ip_forward");
			} else {

				shell_exec("echo \"0\" > /proc/sys/net/ipv4/ip_forward");
			}
		}
		$list=get_ini('IPSYSCTL','PRANGE');
		list($min,$max)=preg_split("/:/",$list);
		if($min=='') $min='1024';
		if($max=='') $max='4999';
		shell_exec("echo -e \"$min\t$max\" > /proc/sys/net/ipv4/ip_local_port_range");
		
	}

?>
