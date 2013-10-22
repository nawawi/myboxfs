<?

$RRD_PATH="rrd";
$RRD_SAVE_PATH="./rrd/png";

function get_dev_traffic() {
	if($fd=fopen('/proc/net/dev', 'r')) {
		while($buf = fgets($fd, 4096)) {
        		if(preg_match('/:/', $buf)) {
          			list($dev_name, $stats_list)=preg_split('/:/', $buf, 2);
          			$stats=preg_split('/\s+/',trim($stats_list));
				$dev_name=trim($dev_name);
          			$results[$dev_name]=array();

          			$results[$dev_name]['rx_bytes'] = $stats[0];
          			$results[$dev_name]['rx_packets'] = $stats[1];
          			$results[$dev_name]['rx_errs'] = $stats[2];
          			$results[$dev_name]['rx_drop'] = $stats[3];

          			$results[$dev_name]['tx_bytes'] = $stats[8];
          			$results[$dev_name]['tx_packets'] = $stats[9];
          			$results[$dev_name]['tx_errs'] = $stats[10];
          			$results[$dev_name]['tx_drop'] = $stats[11];

          			$results[$dev_name]['errs'] = $stats[2] + $stats[10];
          			$results[$dev_name]['drop'] = $stats[3] + $stats[11];
        		}
      		}
    	}
	return $results;
}

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
function get_bandwidth_summation($size='bit',$rrd_source, $data_name, $graph_start=86400, $graph_end=300, $rra_step=1, $ds_step=300) {
	if(empty($rrd_source)||!file_exists($rrd_source)) return;

	$summation_timespan_start = $graph_start;
	$summation_timespan_end = $graph_end;

	if(!isset($summation_cache{$rrd_source})) {
		$summation_cache{$rrd_source} = bandwidth_summation($rrd_source, $summation_timespan_start, $summation_timespan_end, $rra_step, $ds_step);
	}

	$summation = 0;
	$summation = $summation_cache{$rrd_source}{$data_name};
	if($size=='bit') $summation=$summation * 8;
	if($summation < 1000) {
		if($size=='bit') {
			$summation_label = "bits";
		} else {
			$summation_label = "bytes";
		}
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


function create_traffic_graph($dev) {
	global $RRD_PATH, $RRD_SAVE_PATH;
	$rrd_source="$RRD_PATH/traffic_$dev".".rrd";
	if(file_exists("$rrd_source")) return 1;

	$_cmd_cre='';
	$_cmd_cre .="$rrd_source --step 300 ";
	$_cmd_cre .="DS:traffic_in:COUNTER:600:0:100000000 ";
	$_cmd_cre .="DS:traffic_out:COUNTER:600:0:100000000 ";
	$_cmd_cre .="RRA:AVERAGE:0.5:1:600 ";
	$_cmd_cre .="RRA:AVERAGE:0.5:6:700 ";
	$_cmd_cre .="RRA:AVERAGE:0.5:24:775 ";
	$_cmd_cre .="RRA:AVERAGE:0.5:288:797 ";
	$_cmd_cre .="RRA:MAX:0.5:1:600 ";
	$_cmd_cre .="RRA:MAX:0.5:6:700 ";
	$_cmd_cre .="RRA:MAX:0.5:24:775 ";
	$_cmd_cre .="RRA:MAX:0.5:288:797 ";

	shell_exec("rrdtool create $_cmd_cre");
}

function update_traffic_graph($dev) {
	global $RRD_PATH, $RRD_SAVE_PATH;
	$rrd_source="$RRD_PATH/traffic_$dev".".rrd";
	if(!file_exists("$rrd_source")) create_traffic_graph($dev);
	$_dev=get_dev_traffic();
	$value_in=trim($_dev[$dev]['rx_bytes']);
	$value_out=trim($_dev[$dev]['tx_bytes']);
	shell_exec("rrdtool update $rrd_source --template traffic_out:traffic_in N:$value_out:$value_in");
	shell_exec("rrdtool tune $rrd_source --minimum traffic_in:0");
	shell_exec("rrdtool tune $rrd_source --minimum traffic_out:0");
}

// step 1 	= Daily (5 Minute Average)  	86400		300 	(5x60)
// step 6 	= Weekly (30 Minute Average)  	604800		1800	(30x60)		
// step 24	= Monthly (2 Hour Average)  	2678400		7200	(2x60x60)
// step  288 	= Yearly (1 Day Average)	33053184	86400	(24x60x60)

function show_traffic_graph($size='bit',$dev,$datestart,$dateend,$rra_step) {
	global $RRD_PATH, $RRD_SAVE_PATH;

	$rrd_source="$RRD_PATH/traffic_$dev".".rrd";
	$rrd_lastupdate=get_rrd_lastupdate();
	$traffic_in_bandwith=get_bandwidth_summation($size,$rrd_source,'traffic_in',$datestart,$dateend,$rra_step);
	$traffic_out_bandwith=get_bandwidth_summation($size,$rrd_source,'traffic_out',$datestart,$dateend,$rra_step);
	$_cmd_val = '';
	$_cmd_val .="--imgformat=PNG ";
	$_cmd_val .="--start=$datestart ";
	$_cmd_val .="--end=$dateend ";
	$_cmd_val .="--title=\"Traffic $dev\" ";
	$_cmd_val .="--rigid ";
	$_cmd_val .="--base=1000 ";
	$_cmd_val .="--height=120 ";
	$_cmd_val .="--width=500 ";
	$_cmd_val .="--alt-autoscale-max ";
	$_cmd_val .="--lower-limit=0 ";
	if($size=='byte') {
		$_cmd_val .="--vertical-label=\"bytes per second\" ";
	} else {
		$_cmd_val .="--vertical-label=\"bits per second\" ";
	}
	$_cmd_val .="DEF:a=\"$rrd_source\":traffic_in:AVERAGE ";
	$_cmd_val .="DEF:b=\"$rrd_source\":traffic_out:AVERAGE ";
	if($size=='byte') {
		$_cmd_val .="AREA:a#00CF00:\"Inbound\" ";
		$_cmd_val .="GPRINT:a:LAST:\" Current\:%8.2lf %s\" ";
		$_cmd_val .="GPRINT:a:AVERAGE:\"Average\:%8.2lf %s\" ";
		$_cmd_val .="GPRINT:a:MAX:\"Maximum\:%8.2lf %s\" ";
		$_cmd_val .="COMMENT:\"Total In:  $traffic_in_bandwith\\n\" ";
		$_cmd_val .="LINE1:b#002A97:\"Outbound\" ";
		$_cmd_val .="GPRINT:b:LAST:\"Current\:%8.2lf %s\" ";
		$_cmd_val .="GPRINT:b:AVERAGE:\"Average\:%8.2lf %s\" ";
		$_cmd_val .="GPRINT:b:MAX:\"Maximum\:%8.2lf %s\" ";

	} else {
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
	}
	$_cmd_val .="COMMENT:\"Total Out: $traffic_out_bandwith\\n\"  ";
	$_cmd_val .="COMMENT:\"\\n\" ";
	$_cmd_val .="COMMENT:\"$rrd_lastupdate\" ";

	shell_exec("rrdtool graph $dev.png $_cmd_val");
}

foreach(get_dev_traffic() as $a => $b) {
	$dev=$a;
	echo "$dev\n";
	create_traffic_graph($dev);
	update_traffic_graph($dev);
	show_traffic_graph('bit',$dev,"-86400","-300",1);
}

?>

