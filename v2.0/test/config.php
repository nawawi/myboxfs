<?

function ignore_signal($signo) {}

function add_history($line) {
	if(isset($line) && $line !== "") {
		readline_add_history($line);
	}
}

function close_exit($id=0) {
	printf("\033[H\033[J");
	echo "bye!\n";
	exit($id);
}

function show_title($title='Main menu') {
	printf("\033[H\033[J");
	$line=str_repeat("=",75);
	echo "$line\n";
	echo "| Mybox - $title\n";
	echo "$line\n\n";
}

$nic[]="1|WAN|eth0|192.168.0.1|255.255.255.0|On|aa";
$nic[]="1|LAN|eth1|192.168.1.1|255.255.255.0|On|aa";
$nic[]="1|DMZ|eth2|192.168.2.1|255.255.255.0|On|aa";
$nic[]="1|FOV|eth2|192.168.2.1|255.255.255.0|On|aa";

function setup_nic() {
	global $nic;
	while(1) {
		show_title('Network Setup -> Static Interfaces');
		printf("%-1s%-5s%-6s%-8s%-17s%-17s%-7s%-15s\n",'','Id','Name','Device','IP Address','IP Netmask','State','Note');
		printf("%-1s%-5s%-6s%-8s%-17s%-17s%-7s%-15s\n",'','---','----','------','---------------','---------------','-----','-------------');
		foreach($nic as $p) {
			list($id,$name,$dev,$ip,$nmask,$state,$note)=explode("|",$p);
			printf("%-1s%-5s%-6s%-8s%-17s%-17s%-7s%-15s\n",'',$id,$name,$dev,$ip,$nmask,$state,$note);
		}
		printf("%-1s%-5s%-6s%-8s%-17s%-17s%-7s%-15s\n",'','---','----','------','---------------','---------------','-----','-------------');
		echo "\n[1] Edit\n";
		echo "[2] Save\n";
		echo "[3] Add New\n\n";
		echo "[x] Back\n\n";
		$line=readline("option> ");
		add_history($line);
		switch($line) {
			case 'x': break 2;
		}
	}	
}

function network_setup() {
	while(1) {
		show_title('Network Setup');
		echo "[1] Static Interfaces\n";
		echo "[2] Virtual Interfaces\n";
		echo "[3] Vlan Interfaces\n\n";
		echo "[r] Reload setting\n";
		echo "[b] Menu\n";
		echo "[x] Exit\n\n";
		$line=readline("option> ");
		add_history($line);
		switch($line) {
			case '1': setup_nic(); break;
			case '2': setup_vip(); break;
			case '3': setup_vlan(); break;
			case 'r': reload('network'); break;
			case 'b': break 2;
			case 'x': close_exit(0);
		}
	}
}

function system_setup() {
	return 1;
}

function main() {
	pcntl_signal(SIGINT, "ignore_signal");
	while(1) {
		show_title();
		echo "[1] System Setup\n";
		echo "[2] Network Setup\n\n";
		echo "[x] Exit\n\n";
		$line=readline("option> ");
		add_history($line);
		switch($line) {
			case '1': system_setup(); break;
			case '2': network_setup(); break;
			case 'x': close_exit(0);
		}
	}
}

main();

?>