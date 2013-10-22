<?

if(!function_exists("readline")) {
	function readline($prompt="") {
		echo "$prompt";
		$out="";
		$key="";
		$key=fgetc(STDIN);
		while($key!="\n") {
			$out .=$key;
			$key=fread(STDIN, 1);
		}
		return $out;
	}
}
ob_implicit_flush(true);

function str_encrypt($string, $epad='!#$5047a88a7963275e43790ee796dcb7ec#$!') {
	$pad=base64_decode($epad);
	$encrypted='';
	for($i=0;$i<strlen($string);$i++) {
		$encrypted .= chr(ord($string[$i]) ^ ord($pad[$i]));
	}
	return base64_encode($encrypted);
}

function str_decrypt($string, $epad='!#$5047a88a7963275e43790ee796dcb7ec#$!') {
	$pad=base64_decode($epad);
	$encrypted=base64_decode($string);
	$decrypted='';
	for($i=0;$i<strlen($encrypted);$i++) {
		$decrypted .= chr(ord($encrypted[$i]) ^ ord($pad[$i]));
	}
	return $decrypted;
}

function myreadline($str) {
	$ret=readline($str);
	return trim($ret);
}
// global
$_NUM=1;
$_ORG="Tracenetwork Corporation Sdn. Bhd.";

$_RELEASED=time();
$_EXPIRED="none";
srand(rand());
$_MFS_KEY=strtoupper(md5(time().time()."awierockmohdnawawi").sha1(time().time()."awierockmohdnawawi"));
$_MYBOX_SERIAL=array();
$_MYBOX_SERIAL[1]='MFS-1000';
$_MYBOX_SERIAL[2]='MFS-2000';
$_MYBOX_SERIAL[3]='MFS-3000';
$_MYBOX_SERIAL[4]='MFS-4000';

/* 
IPS = Intrusion Prevention System
ITS = Internet Traffic Shaper
ILB = Internet Load Balancing
PPTP = PPTP Virtual Private Network
AUP = Auto Update
IPM = IP Management
UL = User limit
*/

// password
$_PASS=myreadline("\nEnter password: ");
if($_PASS=='' || $_PASS!="bgd321*") exit("Exit!");

// get info
echo <<<_END_
+=============================================================================+
| MYBOX FIREWALL SYSTEM (c) Tracenetwork Corporation Sdn. Bhd.                |
|                           http://www.mybox.net.my info@mybox.net.my         |
+=============================================================================+

MyBox Firewall System License Generator v2.1
You can quit anytime by pressing ctrl+c

Available license type:

_END_;
foreach($_MYBOX_SERIAL as $n => $p) {
	echo "[$n] $p\n";
}
$_NUM1=myreadline("\nSelect type [default: {$_MYBOX_SERIAL[$_NUM]}]: ");
if($_NUM1 > 4 ) exit("Abort\n");
if($_NUM1!='') $_NUM=$_NUM1;
echo "License type => {$_MYBOX_SERIAL[$_NUM]}\n\n";

$_ORG1=myreadline("License owner [defult: $_ORG]: ");
if($_ORG1!='') $_ORG=$_ORG1;

$_RELEASED1=myreadline("Set license release date [e.g; YYYYMMDD 20051002. Default: current]: ");
if($_RELEASED1!='') {
	$_RELEASED=strtotime($_RELEASED1);
	if(strlen($_RELEASED) < 8) exit("Invalid longtime $_RELEASED\n");
}

$_EXPIRED1=myreadline("Set license expiration date [e.g; YYYYMMDD 20051002. Default: none]: ");
if($_EXPIRED1!='') {
	$_EXPIRED=strtotime($_EXPIRED1);
	if(strlen($_EXPIRED) < 8) exit("Invalid longtime $_EXPIRED\n");
}
$_LIMIT=0;
$_LIMIT1=myreadline("Set user/ip limit [Default: unlimited]: ");
if($_LIMIT1!='') {
	$_LIMIT=$_LIMIT1;
}

$_MYBOX_LICENSE=array();
$_MYBOX_LICENSE['pkg']="{$_MYBOX_SERIAL[$_NUM]}";
$_MYBOX_LICENSE['org']="$_ORG";
$_MYBOX_LICENSE['released']="$_RELEASED";
$_MYBOX_LICENSE['expired']="$_EXPIRED";
$_MYBOX_LICENSE['url']="updates.mybox.net.my";
$_MYBOX_LICENSE['UL']=$_LIMIT;

if($_NUM==1) {
	$_MYBOX_LICENSE['IPS']=0;
	$_MYBOX_LICENSE['ITS']=0;
	$_MYBOX_LICENSE['ILB']=0;
	$_MYBOX_LICENSE['PPTP']=0;
	$_MYBOX_LICENSE['AUP']=1;
	$_MYBOX_LICENSE['IPM']=1;
}

if($_NUM==2) {
	$_MYBOX_LICENSE['IPS']=1;
	$_MYBOX_LICENSE['ITS']=1;
	$_MYBOX_LICENSE['ILB']=0;
	$_MYBOX_LICENSE['PPTP']=0;
	$_MYBOX_LICENSE['AUP']=1;
	$_MYBOX_LICENSE['IPM']=1;
}

if($_NUM==3) {
	$_MYBOX_LICENSE['IPS']=1;
	$_MYBOX_LICENSE['ITS']=1;
	$_MYBOX_LICENSE['ILB']=0;
	$_MYBOX_LICENSE['PPTP']=1;
	$_MYBOX_LICENSE['AUP']=1;
	$_MYBOX_LICENSE['IPM']=1;
}

if($_NUM==4) {
	$_MYBOX_LICENSE['IPS']=1;
	$_MYBOX_LICENSE['ITS']=1;
	$_MYBOX_LICENSE['ILB']=1;
	$_MYBOX_LICENSE['PPTP']=1;
	$_MYBOX_LICENSE['AUP']=1;
	$_MYBOX_LICENSE['IPM']=1;
}

if($_MYBOX_LICENSE['AUP']==1) {
	$_confirm=myreadline("Set other auto update url [y/n]: ");
	$_confirm=strtolower($_confirm);
	if($_confirm=="y") {
		$_URL1=myreadline("Set update url [default: {$_MYBOX_LICENSE['url']}]: ");
		if($_URL1!='') {
			$_MYBOX_LICENSE['url']=$_URL1;
			$_MYBOX_LICENSE['name']="none";
			$_URLP1=myreadline("Set update access [eg; user|password]: ");
			if($_URLP1!='') {
				$_MYBOX_LICENSE['name']=str_encrypt("$_URLP1","!5047a88a7963275e43790ee796dcb7ec!");
			}
		}
	}
} 
$_FKEY=strtoupper(md5(time().time()."{$_MYBOX_LICENSE['org']}".time().time()));
$_MYBOX_LICENSE['fkey']=$_FKEY;

// start encrypt
$_MFS_DATA=array();
$_MFS_DATA['hash1']=$_MFS_KEY;
$_SS=serialize($_MYBOX_LICENSE);
$_SS=str_encrypt($_SS,$_MFS_KEY);
$_MFS_DATA['hash2']=$_SS;
$_LIC1=serialize($_MFS_DATA);
$_LIC2=str_encrypt($_LIC1);
// end encrypt

$YES[0]="NO";
$YES[1]="YES";
if($_MYBOX_LICENSE['UL']==0) {
	$_ULIMIT="Unlimited";
} else {
	$_ULIMIT=$_MYBOX_LICENSE['UL'];
}
$_LR=date('d-M-Y h:i:s A',$_MYBOX_LICENSE['released']);
if($_MYBOX_LICENSE['expired']!="none") {
	$_LE==date('d-M-Y h:i:s A',$_MYBOX_LICENSE['expired']);
} else {
	$_LE="No expiration";
}
echo <<<_INFO_



=================================================================
General
-----------------------------------------------------------------
License Ref      : {$_MYBOX_LICENSE['fkey']}
License Id       : {$_MYBOX_LICENSE['pkg']}
License Owner    : {$_MYBOX_LICENSE['org']}
License Release  : $_LR
License Expired  : $_LE
Update URL       : {$_MYBOX_LICENSE['url']}
User Limit       : $_ULIMIT

Module
-----------------------------------------------------------------
Intrusion Prevention System   : {$YES[$_MYBOX_LICENSE['IPS']]}
Internet Traffic Shaper       : {$YES[$_MYBOX_LICENSE['ITS']]}
Internet Load Balancing       : {$YES[$_MYBOX_LICENSE['ILB']]}
PPTP VPN Access               : {$YES[$_MYBOX_LICENSE['PPTP']]}
Auto Update                   : {$YES[$_MYBOX_LICENSE['AUP']]}
IP Management                 : {$YES[$_MYBOX_LICENSE['IPM']]}

License key
-----------------------------------------------------------------

$_LIC2


=================================================================



_INFO_;

if(file_put_contents("mfs-license.exc","$_LIC2")) {
	echo "\n\n=> Saving license key to -> mfs-license.exc file\n";
}

?>
