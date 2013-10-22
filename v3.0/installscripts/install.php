#!/bin/php -f
<?
# +-----------------------------------------------------------------------------+
# | Copyright (c) 2002-2006 Tracenetwork Corporation Sdn. Bhd.           	|
# +-----------------------------------------------------------------------------+
# | This source file is belongs to nawawi, the author.                    	|
# | Any changes/use of the source, please email to the author.           	|
# +-----------------------------------------------------------------------------+
# | Authors: nawawi, Mohd nawawi Mohamad Jamili <nawawi@tracenetwork.com.my 	|
# +-----------------------------------------------------------------------------+
#

putenv("PATH=/bin");
putenv("TERM=linux");
@umask(077);
@chdir("/");

$_VERSION="2.1";
$_TITLE="MyBox Firewall System v$_VERSION Installer - http://www.mybox.net.my";
$_TITLE_SM="MyBox v$_VERSION";
$_COPY="(c) Tracenetwork Corporation Sdn. Bhd.";
$_LOG1="/dev/tty3";
$_LOG2="/dev/tty4";
$_SYSOK=0;
$_CDROM="";

$_SWAPS="2048";
$_DISKSET=",101,,*\n,$SWAPS,S\n;,\n";

// language
// English
$_LANG=array();
$_LANG[0][0]="Welcome to Mybox Firewall System v$_VERSION.";
$_LANG[0][1]="OK";
$_LANG[0][2]="ERROR";
$_LANG[0][3]="Checking storage disk:";
$_LANG[0][4]="Checking CDROM:";
$_LANG[0][5]="Checking NIC:";
$_LANG[0][6]="Checking system requirement failed. You need at least 128M memory, 4GB storage and 2 network cards.";
$_LANG[0][7]="Checking system requirement OK.";
$_LANG[0][8]="Mounting CDROM";

// Malay
$_LANG[1][0]="Selamat datang ke Mybox Firewall System v$_VERSION.";
$_LANG[1][1]="OK";
$_LANG[1][2]="RALAT";
$_LANG[1][3]="Memeriksa ruang storan:";
$_LANG[1][4]="Memeriksa CDROM:";
$_LANG[1][5]="Memeriksa NIC:";
$_LANG[1][6]="Memeriksa keperluan sistem. Anda memerlukan sekurang-kurangnya 128M memori, 4GB storan dan 2 network kad.";
$_LANG[1][7]="Memeriksa keperluan sistem OK.";
$_LANG[1][8]="Mounting CDROM";

@file_put_contents("/proc/sys/kernel/printk","0 0 0 0");
declare(ticks = 1);
ob_implicit_flush(true);

@mkdir("/cdrom");

// function
function do_log($text) {
	global $_LOG1;
        if($fp=fopen($_LOG1,"w")) {
                fwrite($fp,$text);
                fclose($fp);
        }
}

function exec_cmd($cmd) {
	global $_LOG2;
	$ret=1;
	if($cmd!='') {
		system("$cmd >$_LOG2 2>&1", $ret);
		do_log("* $cmd\n");
	}
	return $ret;
}

function findcdroms(&$cdrom) {
	global $_SYSOK;
	$cdrom='';
	$d=dir("/sys/block");
	while(($e=$d->read())) {
		if(($e==".")||($e=="..")) continue;
		if(preg_match("/^hd|^sd|^sr|^scd/",$e,$mm)) {
			if((file_exists("/sys/block/$e/removable") && file_get_contents("/sys/block/$e/removable")==1)) {
				if($cdrom=='') $cdrom="/dev/$e";
			}
		}
        }
        $d->close();
	if($cdrom=='') {
		$cdrom="Not found";
		$_SYSOK++;
	}
}

function finddisks(&$disk,&$size) {
	global $_SYSOK;
	$disk='';$hd='';
	$d=dir("/sys/block");
	while(($e=$d->read())) {
		if(($e==".")||($e=="..")) continue;
		if(preg_match("/^hda|^sda/",$e,$mm)) {
			if((file_exists("/sys/block/$e/removable") && file_get_contents("/sys/block/$e/removable")==0)) {
				if($disk=='') {
					$disk="/dev/$e";$hd="$e";
					if(file_exists("/sys/block/$e/size")) {
						$size=trim(file_get_contents("/sys/block/$e/size"));
						$size=($size / 2 / 1024 / 1024);
					}
				}
			}
		}
        }
        $d->close();
	if($disk=='') {
		$disk="Not found";
		$_SYSOK++;
	}
	if($size < 1) $_SYSOK++;
}

function findnic(&$nic,&$nics) {
	global $_SYSOK;
	$nic=0;$nics='';
	exec("awk -F: '/eth.:/{print $1}' /proc/net/dev",$out,$ret);
	if(!$ret) {
		$nic=count($out);
		if($nic!=0) {
			foreach($out as $ls) $nics .="$ls ";
		}
	}
	$nics=trim($nics);
	if($nic < 2) $_SYSOK++;
}

function abort() {
	@newt_finished();
	@newt_cls();
	system("clear");
	exec_cmd("umount -r /strg");usleep(20);
	exec_cmd("umount -r /boot");usleep(20);
	exec_cmd("umount -r /cdrom");
	sleep(1);
	exec_cmd("eject");
	usleep(2);
	exec_cmd("reboot");
}

function errorbox($txt) {
	global $_LANG,$_lang;
	newt_win_message("{$_LANG[$_lang][2]}","{$_LANG[$_lang][1]}", $txt);
	abort();
}

function statuswindow($width,$height,$title,$txt) {
	$rc=newt_centered_window($width,$height,$title);
	$t=newt_textbox(1, 1, $width - 2, $height - 2, NEWT_TEXTBOX_WRAP);
 	newt_textbox_set_text($t,$txt);
	$f=newt_form(null, null, 0);
	newt_form_add_component($f,$t);
	newt_draw_form($f);
	newt_refresh();
	newt_form_destroy($f);
}

function runcommandwithstatus($cmd,$txt) {
	global $_TITLE_SM,$_LANG,$_lang;
        statuswindow(60, 4, $_TITLE_SM, $txt);
        $rc=exec_cmd($cmd);
	if($rc) errorbox("{$_LANG[$_lang][2]} running command $cmd");
        newt_pop_window();sleep(1);
        return $rc;
}

function text_status($t,$txt) {
	newt_textbox_set_text($t,$txt);
	$f=newt_form(null, null, 0);
	newt_form_add_component($f,$t);
	newt_draw_form($f);
	newt_refresh();
}
function sys_check() {
	global $_SYSOK, $_TITLE_SM,$_LANG,$_lang,$_CDROM;

	$width=60;
	$height=10;
	$rc=newt_centered_window($width,$height,$_TITLE_SM);
	$t=newt_textbox(1, 1, $width - 2, $height - 2, NEWT_TEXTBOX_WRAP);
	finddisks($_disk,$_size);
	$txt="{$_LANG[$_lang][3]} $_disk ($_size GB)";
	text_status($t,$txt);sleep(1);
	findcdroms($_cdrom);
	$_CDROM="$_cdrom";
	$txt="$txt\n{$_LANG[$_lang][4]} $_cdrom (/cdrom)";
	text_status($t,$txt);sleep(1);
	findnic($_nic,$_nics);
	$txt="$txt\n{$_LANG[$_lang][5]} $_nics ($_nic nic)";
	text_status($t,$txt);sleep(1);
	if($_SYSOK) {
		errorbox("{$_LANG[$_lang][6]}");
	}
	$txt="$txt\n{$_LANG[$_lang][7]}";
	text_status($t,$txt);sleep(1);
	newt_form_destroy($f);
}

// newt
echo "Running MyBox Firewall System Installer...";

newt_init();
newt_cls();

newt_draw_root_text(1, 0, "$_TITLE");
newt_push_help_line (null);
newt_draw_root_text(1, 1, "$_COPY");
$options=array("English","Malay");
$option=0;$_lang=0;
$rc=newt_win_menu("Language selection",
		"Select the language you wish to use for the installation process", 
		30, 0, 20, 6, 
		$options,&$option,"{$_LANG[$_lang][1]}");
$_lang=$option;
newt_win_message("$_TITLE_SM","{$_LANG[$_lang][1]}","{$_LANG[$_lang][0]}");
//-----------
sys_check();
sleep(1);
if($_CDROM!='') runcommandwithstatus("/bin/mount -t iso9660 $_CDROM /cdrom","{$_LANG[$_lang][8]} $_CDROM (/cdrom)");
sleep(5);
//-----------
newt_finished();

abort();
exit;
?>