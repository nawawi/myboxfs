<?
/*
+----------------------------------------------+
| addrs   bits   pref   class  mask            |
+----------------------------------------------+
|            0    /32          255.255.255.255 |
|     2      1    /31          255.255.255.254 |
|     4      2    /30          255.255.255.252 |
|     8      3    /29          255.255.255.248 |
|    16      4    /28          255.255.255.240 |
|    32      5    /27          255.255.255.224 |
|    64      6    /26          255.255.255.192 |
|   128      7    /25          255.255.255.128 |
|   256      8    /24      1C  255.255.255.0   |
|   512      9    /23      2C  255.255.254.0   |
|    1K     10    /22      4C  255.255.252.0   |
|    2K     11    /21      8C  255.255.248.0   |
|    4K     12    /20     16C  255.255.240.0   |
|    8K     13    /19     32C  255.255.224.0   |
|   16K     14    /18     64C  255.255.192.0   |
|   32K     15    /17    128C  255.255.128.0   |
|   64K     16    /16      1B  255.255.0.0     |
|  128K     17    /15      2B  255.254.0.0     |
|  256K     18    /14      4B  255.252.0.0     |
|  512K     19    /13      8B  255.248.0.0     |
|    1M     20    /12     16B  255.240.0.0     |
|    2M     21    /11     32B  255.224.0.0     |
|    4M     22    /10     64B  255.192.0.0     |
|    8M     23     /9    128B  255.128.0.0     |
|   16M     24     /8      1A  255.0.0.0       |
|   32M     25     /7      2A  254.0.0.0       |
|   64M     26     /6      4A  252.0.0.0       |
|  128M     27     /5      8A  248.0.0.0       |
|  256M     28     /4     16A  240.0.0.0       |
|  512M     29     /3     32A  224.0.0.0       |
| 1024M     30     /2     64A  192.0.0.0       |
| 2048M     31     /1    128A  128.0.0.0       |
| 4096M     32     /0    256A  0.0.0.0         |
+----------------------------------------------+
*/

function binnmtowm($binin){
	$binin=rtrim($binin, "0");
	if(!ereg("0",$binin) ){
		return str_pad(str_replace("1","0",$binin), 32, "1");
	} else {
		return "1010101010101010101010101010101010101010";
	}
}

function bintocdr($binin){
	return strlen(rtrim($binin,"0"));
}

function bintodq($binin) {
	if ($binin=="N/A") return $binin;
	$binin=explode(".", chunk_split($binin,8,"."));
	for ($i=0; $i<4 ; $i++) {
		$dq[$i]=bindec($binin[$i]);
	}
        return implode(".",$dq) ;
}

function binwmtonm($binin){
	$binin=rtrim($binin, "1");
	if (!ereg("1",$binin)){
		return str_pad(str_replace("0","1",$binin), 32, "0");
	} else return "1010101010101010101010101010101010101010";
}

function cdrtobin($cdrin){
	return str_pad(str_pad("", $cdrin, "1"), 32, "0");
}

function dqtobin($dqin) {
        $dq = explode(".",$dqin);
        for ($i=0; $i<4 ; $i++) {
           $bin[$i]=str_pad(decbin($dq[$i]), 8, "0", STR_PAD_LEFT);
        }
        return implode("",$bin);
}

function check_ipv4($ip) {
	// 0 = false; 1 = true;
	if(preg_match('/^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$/',$ip)) {
		if(!ereg('^0.',$ip)) {
			foreach(explode(".",$dq_host) as $octet ){
 				if($octet > 255){ 
					return 0;
				}
			}
		}
		return 1;
	}
	return 0;
}

function ipcalc($ip,$nmask='255.255.255.0') {
	if(!check_ipv4($ip)) return array();
	$dq_host=$ip;
	$ret=array();
	if(preg_match("/^[0-9]{1,2}$/",$nmask)) {
		if(!($nmask >= 0 && $nmask <= 32)) return $ret;
		$bin_nmask=cdrtobin($nmask);
		$bin_wmask=binnmtowm($nmask);
		$_prefix=$nmask;
	} elseif(preg_match('/^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$/',$nmask)) {
		$bin_nmask=dqtobin($nmask);
		$bin_wmask=binnmtowm($bin_nmask);
		if(ereg("0",rtrim($bin_nmask, "0"))) {
			$bin_wmask=dqtobin($nmask);
			$bin_nmask=binwmtonm($bin_wmask);
			if(ereg("0",rtrim($bin_nmask, "0"))) {
				return $ret;
			}
		}
		$cdr_nmask=bintocdr($bin_nmask);
		$_prefix=$cdr_nmask;
	} else {
		return $ret;
	}
	$bin_host=dqtobin($dq_host);
	$bin_bcast=(str_pad(substr($bin_host,0,$cdr_nmask),32,1));
	$bin_net=(str_pad(substr($bin_host,0,$cdr_nmask),32,0));
	$bin_first=(str_pad(substr($bin_net,0,31),32,1));
	$bin_last=(str_pad(substr($bin_bcast,0,31),32,0));
	$host_total=(bindec(str_pad("",(32-$cdr_nmask),1)) - 1);
	if($host_total <= 0) {
		$bin_first="N/A" ; $bin_last="N/A" ; $host_total="N/A";
		if ($bin_net === $bin_bcast) $bin_bcast="N/A";
	}
	if(ereg('^(00001010)|(101011000001)|(1100000010101000)',$bin_net)) {
		$_type='Private';
	} else {
		$_type='Public';
	}
	$_ipaddr=$dq_host;
	$_netmask=bintodq($bin_nmask);
	$_wildcard=bintodq($bin_wmask);
	$_network=bintodq($bin_net);
	$_broadcast=bintodq($bin_bcast);
	$_hostmin=bintodq($bin_first);
	$_hostmax=bintodq($bin_last);
	$_hostnet=$host_total;
	
	$ret['ip']=$_ipaddr;
	$ret['netmask']=$_netmask;
	$ret['wildcard']=$_wildcard;
	$ret['network']=$_network;
	$ret['broadcast']=$_broadcast;
	$ret['hostmin']=$_hostmin;
	$ret['hostmax']=$_hostmax;
	$ret['total']=$_hostnet;
	$ret['prefix']=$_prefix;
	$ret['type']=$_type;
	return $ret;
}


?>
