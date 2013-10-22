<?

function rrdtool_read($cmd) {
	$_line='';
	if($fi=popen("rrdtool $cmd","r")) {
		while(!feof($fi)) {
			$_line .=fgets($fi,4096);
		}
		pclose($fi);
	}
	return $_line;
}

function rrdtool_write($cmd) {
	shell_exec("rrdtool $cmd");
}

function &rrdtool_function_fetch($rrd_source, $start_time, $end_time) {
	if(empty($rrd_source)||!file_exists($rrd_source)) return;

	$regexps=array();
	$fetch_array=array();
	$data_source_names=array();

	$output = rrdtool_read("fetch $rrd_source AVERAGE -s $start_time -e $end_time");

	$line_one = substr($output, 0, strpos($output, "\n"));

	if (preg_match_all("/\w+/", $line_one, $data_source_names)) {
		if (preg_match("/^timestamp/", $line_one)) {
			array_shift($data_source_names[0]);
		}

		$fetch_array["data_source_names"] = $data_source_names[0];

		for ($i=0;$i<count($fetch_array["data_source_names"]);$i++) {
			$regexps[$i] = '/[0-9]+:\s+';
			for ($j=0;$j<count($fetch_array["data_source_names"]);$j++) {
				if ($j == $i) {
					$regexps[$i] .= '([0-9]{1}\.[0-9]+)e([\+-][0-9]{2,3})';
				}else{
					$regexps[$i] .= '[0-9]{1}\.[0-9]+e[\+-][0-9]{2,3}';
				}

				if ($j < count($fetch_array["data_source_names"])) {
					$regexps[$i] .= '\s+';
				}
			}
			$regexps[$i] .= '/';
		}
	}

	$max_array = array();

	for ($i=0;$i<count($regexps);$i++) {
		$fetch_array["values"][$i] = array();

		if (preg_match_all($regexps[$i], $output, $matches)) {
			for ($j=0; ($j < count($matches[1])); $j++) {
				$line = ($matches[1][$j] * (pow(10,(float)$matches[2][$j])));
				array_push($fetch_array["values"][$i], ($line * 1));

				$max_array[$j][$i] = $line;
			}
		}
	}

	return $fetch_array;
}

function bandwidth_summation($rrd_source, $start_time, $end_time, $rra_steps, $ds_steps) {
	$fetch_array = rrdtool_function_fetch($rrd_source, $start_time, $end_time);

	if ((!isset($fetch_array["data_source_names"])) || (count($fetch_array["data_source_names"]) == 0)) {
		return;
	}

	$return_array = array();
	for ($i=0;$i<count($fetch_array["data_source_names"]);$i++) {
		$sum = 0;

		if (isset($fetch_array["values"][$i])) {
			$values_array = $fetch_array["values"][$i];

			for ($j=0;$j<count($fetch_array["values"][$i]);$j++) {
				$sum += $fetch_array["values"][$i][$j];
			}

			if (count($fetch_array["values"][$i]) != 0) {
				$sum = ($sum * $ds_steps * $rra_steps);
			}else{
				$sum = 0;
			}

			$return_array{$fetch_array["data_source_names"][$i]} = $sum;
		}
	}

	return $return_array;
}
$summation_cache = array();
function get_bandwidth_summation($rrd_source, $data_name, $graph_start=86400, $graph_end=300, $rra_step=1, $ds_step=300) {
	if(empty($rrd_source)||!file_exists($rrd_source)) return;

	$summation_timespan_start = $graph_start;
	$summation_timespan_end = $graph_end;

	if(!isset($summation_cache{$rrd_source})) {
		$summation_cache{$rrd_source} = bandwidth_summation($rrd_source, $summation_timespan_start, $summation_timespan_end, $rra_step, $ds_step);
	}

	$summation = 0;
	$summation = $summation_cache{$rrd_source}{$data_name};
	
	if($summation < 1000) {
		$summation_label = "bytes";
	} elseif($summation < 1000000) {
		$summation_label = "KB";
		$summation /= 1000;
	} elseif($summation < 1000000000) {
		$summation_label = "MB";
		$summation /= 1000000;
	} elseif($summation < 1000000000000) {
		$summation_label = "GB";
		$summation /= 1000000000;
	} else {
		$summation_label = "TB";
		$summation /= 1000000000000;
	}

	if(isset($summation_label)) {
		return round($summation, 2) . " $summation_label";
	} else {
		return round($summation, 2);
	}
}

function get_rrd_lastupdate() {
	return date('d/m/Y h:i:s A T');
}

$_RRD['traffic']['source']='./traffic.rrd';
$_RRD['traffic']['vtitle']='bits per second';
$_RRD['graphsave_path']='';

// step 1 	= Daily (5 Minute Average)  	86400		300 	(5x60)
// step 6 	= Weekly (30 Minute Average)  	604800		1800	(30x60)		
// step 24	= Monthly (2 Hour Average)  	2678400		7200	(2x60x60)
// step  288 	= Yearly (1 Day Average)	33053184	86400	(24x60x60)

/*
eth0 in 	= .1.3.6.1.2.1.2.2.1.10.2 	interfaces.ifTable.ifEntry.ifInOctets.2
eth0 out	= .1.3.6.1.2.1.2.2.1.16.2 	interfaces.ifTable.ifEntry.ifOutOctets.2

eth1 in 	= .1.3.6.1.2.1.2.2.1.10.3	interfaces.ifTable.ifEntry.ifInOctets.3
eth1 out	= .1.3.6.1.2.1.2.2.1.16.3	interfaces.ifTable.ifEntry.ifOutOctets.3

eth2 in		= .1.3.6.1.2.1.2.2.1.10.4	interfaces.ifTable.ifEntry.ifInOctets.4
eth2 out	= .1.3.6.1.2.1.2.2.1.16.4 	interfaces.ifTable.ifEntry.ifOutOctets.4

eth3 in		= .1.3.6.1.2.1.2.2.1.10.5	interfaces.ifTable.ifEntry.ifInOctets.5
eth3 out	= .1.3.6.1.2.1.2.2.1.16.5	interfaces.ifTable.ifEntry.ifOutOctets.5
*/

function do_traffic_graph($graph_name,$title,$datestart,$dateend,$rra_step,$type,$savename="graph") {
	global $_RRD;
	$rrd_source="$graph_name.rrd";
	$rrd_name_in='traffic_in';
	$rrd_name_out='traffic_out';
	$rrd_save_path=$_RRD['graphsave_path'];
	$rrd_htitle=$title;
	$rrd_vtitle=$_RRD['traffic']['vtitle'];
	$rrd_lastupdate=get_rrd_lastupdate();
	$traffic_in_bandwith=get_bandwidth_summation($rrd_source,$rrd_name_in,$datestart,$dateend,$rra_step);
	$traffic_out_bandwith=get_bandwidth_summation($rrd_source,$rrd_name_out,$datestart,$dateend,$rra_step);
	$_cmd_val = '';
	$_cmd_val .="--imgformat=PNG ";
	$_cmd_val .="--start=$datestart ";
	$_cmd_val .="--end=$dateend ";
	$_cmd_val .="--title=\"$rrd_htitle \" ";
	$_cmd_val .="--rigid ";
	$_cmd_val .="--base=1000 ";
	$_cmd_val .="--height=120 ";
	$_cmd_val .="--width=500 ";
	$_cmd_val .="--alt-autoscale-max ";
	$_cmd_val .="--lower-limit=0 ";
	$_cmd_val .="--vertical-label=\"$rrd_vtitle\" ";
	$_cmd_val .="DEF:a=\"$rrd_source\":$rrd_name_in:AVERAGE ";
	$_cmd_val .="DEF:b=\"$rrd_source\":$rrd_name_out:AVERAGE ";
	$_cmd_val .="CDEF:cdefa=a,8,* ";
	$_cmd_val .="CDEF:cdeff=b,8,* ";
	$_cmd_val .="AREA:cdefa#00CF00:\"Inbound\" ";
	$_cmd_val .="GPRINT:cdefa:LAST:\" Current\\:%8.2lf %s\" ";
	$_cmd_val .="GPRINT:cdefa:AVERAGE:\"Average\\:%8.2lf %s\" ";
	$_cmd_val .="GPRINT:cdefa:MAX:\"Maximum\\:%8.2lf %s\"  ";
	$_cmd_val .="COMMENT:\"Total In:  $traffic_in_bandwith\\n\"  ";
	$_cmd_val .="LINE1:cdeff#002A97:\"Outbound\" ";
	$_cmd_val .="GPRINT:cdeff:LAST:\"Current\\:%8.2lf %s\" ";
	$_cmd_val .="GPRINT:cdeff:AVERAGE:\"Average\\:%8.2lf %s\"  ";
	$_cmd_val .="GPRINT:cdeff:MAX:\"Maximum\\:%8.2lf %s\"  ";
	$_cmd_val .="COMMENT:\"Total Out: $traffic_out_bandwith\\n\"  ";
	$_cmd_val .="COMMENT:\"\\n\" ";
	$_cmd_val .="COMMENT:\"$rrd_lastupdate\" ";
	$_cmd_cre = '';
	if($type=='create' && !file_exists($rrd_source)) {
		$_cmd_cre .="$rrd_source --step 300 ";
		$_cmd_cre .="DS:$rrd_name_in:COUNTER:600:0:100000000 ";
		$_cmd_cre .="DS:$rrd_name_out:COUNTER:600:0:100000000 ";
		$_cmd_cre .="RRA:AVERAGE:0.5:1:600 ";
		$_cmd_cre .="RRA:AVERAGE:0.5:6:700 ";
		$_cmd_cre .="RRA:AVERAGE:0.5:24:775 ";
		$_cmd_cre .="RRA:AVERAGE:0.5:288:797 ";
		$_cmd_cre .="RRA:MAX:0.5:1:600 ";
		$_cmd_cre .="RRA:MAX:0.5:6:700 ";
		$_cmd_cre .="RRA:MAX:0.5:24:775 ";
		$_cmd_cre .="RRA:MAX:0.5:288:797 ";
		rrdtool_write("create $_cmd_cre");
	} elseif($type=='update') {
		rrdtool_write("update $rrd_source --template $rrd_name_in N:$value_in");
		rrdtool_write("update $rrd_source --template $rrd_name_out N:$value_out");
		rrdtool_write("tune $rrd_source --minimum $rrd_name_in:0");
		rrdtool_write("tune $rrd_source --minimum $rrd_name_out:0");
	} elseif($type=='show') {
		return rrdtool_write(" graph - $_cmd_val");
	} elseif($type=='save') {
		rrdtool_write(" graph $rrd_save_path$savename.png $_cmd_val");
	}
}

?>


