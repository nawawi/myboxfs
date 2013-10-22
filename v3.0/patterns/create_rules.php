<?
include_once("config.php");

function mybox_append_to_file($filename,$text) {
	if(extension_loaded('dio')) {
		if($fp=dio_open($filename, O_RDWR | O_CREAT | O_APPEND)) {
 			dio_write($fp,$text,strlen($text));
			dio_close($fp);
			@chmod($filename,0600);
		}
	} else {
		if($fp=fopen($filename,"a")) {
 			fwrite($fp,$text);
			fclose($fp);
		}
	}
}

function mybox_fget_contents($file) {
	if(file_exists($file)) return trim(file_get_contents($file));
}
function mybox_mkdirr($pathname, $mode = 0700) {
	if(is_dir($pathname) || empty($pathname)) return true;
	if(is_file($pathname)) return false;
	$next_pathname = substr($pathname, 0, strrpos($pathname, DIRECTORY_SEPARATOR));
	if(mybox_mkdirr($next_pathname, $mode)) {
        	if(!file_exists($pathname)) {
			return mkdir($pathname, $mode);
        	}
    	}
    	return false;
}

function mybox_unlink($fileglob) {
	if(is_string($fileglob)) {
		if(is_file($fileglob)) {
			return unlink($fileglob);
        	} elseif(is_dir($fileglob)) {
			$ok=mybox_unlink("$fileglob/*");
            		if($ok==false) {
				return false;
			}
 			return rmdir($fileglob);
		} else {
			$matching=glob($fileglob);
			if($matching===false) return false;  
 			$rcs=array_map('mybox_unlink', $matching);
			if(@in_array(false, $rcs)) return false;
		}      
	} elseif(is_array($fileglob)) {
		$rcs=array_map('mybox_unlink', $fileglob);
		if(@in_array(false, $rcs)) return false;
	} else {
		return false;
	}
	return true;
}

function escape_str($str) {
        if($str!='') {
                return htmlentities($str,ENT_QUOTES);
        }
        return $str;
}


function proc_file($line,$file) {
	$sig='';$msg='';
	if(preg_match("/^alert\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+\((.*)/",$line,$mm)) {
		unset($mm[0]);
		//print_r($mm);
		if($mm[7]!='') {
			$buf=preg_split("/;/",$mm[7]);
			//print_r($buf);
			if(count($buf)!=0) {
				if(preg_match("/msg\:\"(.*)\"/",$buf[0],$mx)) {
					if($mx[1]!='') {
						if(preg_match("/(\S+)\s+(.*)/",$mx[1],$mx2)) {
							$sig=$mx2[1];
							$msg=escape_str($mx2[2]);
							if($sig=="BLEEDING-EDGE" || $sig=="COMMUNITY") {
								if(preg_match("/(\S+)\s+(\S+)\s+(.*)/i",$mx[1],$mx3)) {
									if(!preg_match("/{$mx3[2]}/i","$file")) {
										$sig=strtoupper($file);
										$msg=escape_str("{$mx3[2]} {$mx3[3]}");
									} else {
										$sig=strtoupper($mx3[2]);
										$msg=escape_str($mx3[3]);
									}
								}
							}							
						}
					}
				}
			}
		}		
	}
	return array($sig,$msg);
}

function proc_line($line,$msg) {
	if(preg_match("/^alert\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+\((.*)/",$line,$mm)) {
		unset($mm[0]);
		if($mm[7]!='') {
			$buf=preg_split("/;/",$mm[7]);
			if(count($buf)!=0) {
				if(preg_match("/msg\:\"(.*)\"/",$buf[0],$mx)) {
					if($mx[1]!='') {
						$msg1=$mx[1];
						$msg2=html_entity_decode($msg,ENT_QUOTES);
						$ret=str_replace($msg1,$msg2,$line);
					}
				}
			}
		}		
	}
	return "$ret";
}

function get_sid($line) {
	if(preg_match("/\s+sid:(\d+)\;/",$line,$mm)) {
		return $mm[1];
	}
	return;
}

function create_imp2p($line) {
	if(preg_match("/\(msg\:\"CHAT\s+(\S+)\s+/",$line,$mm)) {
		return strtolower($mm[1]);
	}
	return;
}

function read_rules_file($file) {
	global $_FIGNORE, $_FRENA, $_FRENB, $_F, $_SIGNORE, $_CDIS;
	global $_ADIS, $_RDIS;

	$fbase=basename($file);
	$buff=file("$file");
	$next=array();
	$done=array();
	$sidd=array();
	if(count($buff)!=0) {
		foreach($buff as $line) {
			$line=trim($line);
			if($line{0}=="#" || $line{0}=="") continue;
			$next[]="$line";
		}
	}
	unset($line,$buff);
	if(count($next)!=0) {
		$fbase=basename($fbase,".rules");
		$_note='';
		$_note=$_F[$fbase];
		if($_note!='') $_note=escape_str($_note);
		foreach($next as $line) {
			$rule='';
			$line=preg_replace($_FRENA,$_FRENB,$line);
			$sid='';$mx=array();
			$sid=get_sid($line);
			if($sid=='') {
				echo "SID EMPTY $file\n";
				continue;
			}
			if($_SIGNORE[$sid]==1) {
				echo "SKIP SID $sid-> $file\n";
                                       continue;
			}
			if($sidd[$sid]==1) {
				echo "SID $sid $file already exist\n";
				continue;
			}
			$sidd[$sid]=1;
			$mx=array();
			$mx=proc_file($line,$fbase);
			if($mx[0]!='' && $mx[1]!='') {
				$linex=proc_line($line,"{$mx[0]} {$mx[1]}");
				if($_ADIS[$fbase]==1) {
					if(!is_dir("./sig/anti-malware")) mybox_mkdirr("./sig/anti-malware");
					mybox_append_to_file("./sig/anti-malware/rules","$linex\n");
					continue;
				}
				if($_RDIS[$fbase]==1) {
					if(!is_dir("./sig/im_p2p/$fbase")) mybox_mkdirr("./sig/im_p2p/$fbase");
					$imd=create_imp2p($linex);
					$fff="rules";
					if($imd!='') $fff="$imd";
					mybox_append_to_file("./sig/im_p2p/$fbase/$fff","$linex\n");
					$imd='';
					continue;
				}
				if($_CDIS[$fbase]==1) $fbase="#{$fbase}";
				if(!is_dir("./sig/rules/$fbase")) mybox_mkdirr("./sig/rules/$fbase");
				if(!file_exists("./sig/rules/$fbase/info")) file_put_contents("./sig/rules/$fbase/info","$_note\n");
				mybox_append_to_file("./sig/rules/$fbase/rules","$linex\n");
			}
		}
	}
	unset($line);
}

function read_rules_dir() {
	global $_FIGNORE;
	mybox_unlink("./sig/*");
	mybox_mkdirr("./sig/rules");
	mybox_mkdirr("./sig/lib");
	$dir=glob("./snort/rules/*.rules");
	if(count($dir)!=0) {
		foreach($dir as $f) {
			$b=basename($f);
			if($_FIGNORE[$b]==1) continue;
			echo "READ FILE $f\n";
			read_rules_file($f);
		}
	}	
	unset($dir,$f);
	$dir=glob("./snort/so_rules/*.rules");
	if(count($dir)!=0) {
		foreach($dir as $f) {
			$b=basename($f);
			if($_FIGNORE[$b]==1) continue;
			echo "READ FILE $f\n";
			read_rules_file($f);
		}
	}	
	unset($dir,$f);
	system("cp -av ./snort/so_rules/*.so ./sig/lib");
	system("cp -v ./snort/rules/unicode.map ./sig");
	$dir=glob("./bleeding/*.rules");
	if(count($dir)!=0) {
		foreach($dir as $f) {
			$b=basename($f);
			if($_FIGNORE[$b]==1) continue;
			echo "READ FILE $f\n";
			read_rules_file($f);
		}
	}	
	unset($dir,$f);
}
function cclass() {
	$buff=file("./snort/rules/classification.config");
	$line='';
	$a_array=array();
	if(count($buff)!=0) {
        	foreach($buff as $lx) {
                	$lx=trim($lx);
                	if($lx{0}=="#") continue;
                	if($lx=='') continue;
                	if(preg_match("/config\s+classification\:\s+(\S+)\,(.*?)\,(\d+)/",$lx,$mm) || 
				preg_match("/config\s+classification\:\s+(\S+)\,(.*?)\,\s+(\d+)/",$lx,$mm) ||
				preg_match("/config\s+classification\:\s+(\S+)\,\s+(.*?)\,(\d+)/",$lx,$mm) ||
				preg_match("/config\s+classification\:\s+(\S+)\,\s+(.*?)\,\s+(\d+)/",$lx,$mm)) {
                        	$line .="$lx\n";
                        	$a_array[$mm[1]][0]=$mm[2];
                        	$a_array[$mm[1]][1]=$mm[3];
                	}
        	}
        	file_put_contents("./sig/classification.config","$line");
        	file_put_contents("./sig/classification.array",serialize($a_array));
	}
}

chdir("$_ROOT_DIR");
read_rules_dir();
cclass();
?>
