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
// $Id: inifunc.inc,v 1.00 2003/11/25 11:27 PM nawawi Exp $
//
	function getini($section,$param,$file) {
		if(!file_exists($file)) {
			return null;
		}
		$insection='0';
		$buff=@file($file);
		foreach($buff as $line) {
			$line=trim($line);
			if($line!='') {
				if(preg_match("/^\[$section\]/",$line)) {
					$insection='1';
				}
				if($insection==1) {
					list($p1,$p2)=preg_split('/=/',$line);
					$p1=trim($p1);
					$p2=trim($p2);
					if($p1=="$param") {
						list($p3,$p4)=preg_split('/;/',$p2);
						if($p3=='') $p3=null;
						flush();
						return trim($p3);
					}
					unset($p1,$p2,$p3,$p4);
				}
			}
		}
		unset($buff);
		flush();
		return null;
	}


	function writeini($section,$param,$data,$file) {
		$insection='0';
		$wasinsection='0';
		$str=array();
		$buff=@file($file);
		foreach($buff as $line) {
			$line=trim($line);
			if($line!='' && $line{0}!=';') {
				if($insection==0) {
					if(preg_match("/^\[$section\]/",$line)) {
						$insection='1';
						$wasinsection='1';
					}
					array_push($str,$line);
				} else {
					if($line{0}=='[') {
						$insection='0';
						array_push($str,"$param=$data");
						array_push($str,$line);
					} else {
						list($p1,$p2)=preg_split("/=/",$line);
						$p1=trim($p1);
						$p2=trim($p2);
						if($p1!=$param) {
							array_push($str,$line);
						}
					}

				}
			}
		} 
		unset($buff);
		if($insection==1) {
			array_push($str,"$param=$data");
		}
		if($wasinsection==0) {
			array_push($str,"\n[$section]");
			array_push($str,"$param=$data");
		}
		$handle=@fopen($file,'w');
		@flock($handle,LOCK_EX);
		$inc='0';
		foreach($str as $line) {
			$line=trim($line);
			if($line!='') {
				if($line{0}=='[') {
					@fputs($handle,"\n$line\n");
				} else {
					@fputs($handle,"$line\n");
				}
			}
		}
		@flock($handle,LOCK_UN);
		@fflush($handle);
		@fclose($handle);
		flush();
	}
?>
