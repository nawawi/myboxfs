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
// $Id: package.inc,v 1.00 2003/07/28 1:07 AM nawawi Exp $
//

	$PKK=get_ini('MINISERV','RSA');
	global $PKK;

	function zip_encrypt($fi) {
                global $PKK;
                if(!file_exists($fi)) return 0;
		list($fo,$ext)=preg_split("/\./",$fi);
                shell_exec("uuencode $fi $fi > $fi.encode");
                @unlink("$fi");
                $ret=shell_exec("crypt -e $fi.encode $PKK");
                rename("$fi.encode","$fo");
		shell_exec("tar -zcf $fi $fo");
		@unlink($fo);
                return trim($ret);
        }

	function zip_decrypt($fi) {
		global $PKK;
		if(!file_exists($fi)) return 0;
		shell_exec("tar -zxf $fi");
		list($fo,$ext)=preg_split("/\./",$fi);
		$ret=shell_exec("crypt -d $fo $PKK");
		shell_exec("uudecode $fo -o $fo.decode");
		rename("$fo.decode","$fi");
		@unlink("$fo");
		return trim($ret);
	}

	function get_info($section,$param) {
		$ret=trim(`config -r $section $param /tmp/mybox_update/pkg.info`);
		return $ret;
	}
	function update_md5sum($pkg,$md5) {
		if(!file_exists('/hd/configs/md5sum.lst')) {
			$file=file('/hd/configs/md5sum.lst');
			$handle=fopen('/tmp/md5sum.tmp','a');
			for($x='0';$x<count($file);$x++) {
				$line=trim($file[$x]);
				if($line!='') {
					list($file,$key)=preg_split("/\s+/",$line);
					if($pkg==$file) {
						$file[$x]="$pkg  $md5";
					}
				}
                	}
			foreach($file as $line) {
				fputs($handle,"$line\n");
			}
			fclose($handle);
			shell_exec('mv /tmp/md5sum.tmp /hd/configs/md5sum.lst');
		}
	}

	function check_updateid($what) {
		if(file_exists('/hd/configs/update.lst')) {
			$buff=file('/hd/configs/update.lst');
			foreach($buff as $line) {
				$line=trim($line);
				if($line!='') {
					list($day,$date,$time,$id,$ip)=preg_split("/\|/",$line);
					if($what=="$id") {
						return 1;
					}
				}
			}
			return 0;
		}
		return 0;
	}
?>