<?php
/*
 +-------------------------------------------------------------------------+
 | Copyright (C) 2004 Ian Berry                                            |
 |                                                                         |
 | This program is free software; you can redistribute it and/or           |
 | modify it under the terms of the GNU General Public License             |
 | as published by the Free Software Foundation; either version 2          |
 | of the License, or (at your option) any later version.                  |
 |                                                                         |
 | This program is distributed in the hope that it will be useful,         |
 | but WITHOUT ANY WARRANTY; without even the implied warranty of          |
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           |
 | GNU General Public License for more details.                            |
 +-------------------------------------------------------------------------+
 | cacti: a php-based graphing solution                                    |
 +-------------------------------------------------------------------------+
 | Most of this code has been designed, written and is maintained by       |
 | Ian Berry. See about.php for specific developer credit. Any questions   |
 | or comments regarding this code should be directed to:                  |
 | - iberry@raxnet.net                                                     |
 +-------------------------------------------------------------------------+
 | - raXnet - http://www.raxnet.net/                                       |
 +-------------------------------------------------------------------------+
*/

define("RRD_NL", " \\\n");

function escape_command($command) {
	return ereg_replace("(\\\$|`)", "", $command);
}

function rrd_init() {
	$rrd_struc["fd"] = popen(read_config_option("path_rrdtool") . " -", "w");

	return $rrd_struc;
}

function rrd_close($rrd_struc) {
	/* close the rrdtool file descriptor */
	pclose($rrd_struc["fd"]);
}

function rrd_get_fd(&$rrd_struc, $fd_type) {
	if (sizeof($rrd_struc) == 0) {
		return 0;
	}else{
		return $rrd_struc["fd"];
	}
}

function rrdtool_execute($command_line, $log_to_stdout, $output_flag, $rrd_struc = array(), $logopt = "WEBLOG") {
	global $config;

	if (!is_numeric($output_flag)) {
		$output_flag = RRDTOOL_OUTPUT_STDOUT;
	}

	/* WIN32: before sending this command off to rrdtool, get rid
	of all of the '\' characters. Unix does not care; win32 does.
	Also make sure to replace all of the fancy \'s at the end of the line,
	but make sure not to get rid of the "\n"'s that are supposed to be
	in there (text format) */
	$command_line = str_replace("\\\n", " ", $command_line);

	/* output information to the log file if appropriate */
	if (read_config_option("log_verbosity") >= POLLER_VERBOSITY_DEBUG) {
		cacti_log("CACTI2RRD: " . read_config_option("path_rrdtool") . " $command_line", $log_to_stdout, $logopt);
	}

	/* if we want to see the error output from rrdtool; make sure to specify this */
	if (($output_flag == RRDTOOL_OUTPUT_STDERR) && (sizeof($rrd_struc) == 0)) {
		$command_line .= " 2>&1";
	}

	/* use popen to eliminate the zombie issue */
	if ($config["cacti_server_os"] == "unix") {
		/* an empty $rrd_struc array means no fp is available */
		if (sizeof($rrd_struc) == 0) {
			$fp = popen(read_config_option("path_rrdtool") . escape_command(" $command_line"), "r");
		}else{
			fwrite(rrd_get_fd($rrd_struc, RRDTOOL_PIPE_CHILD_READ), escape_command(" $command_line") . "\r\n");
			fflush(rrd_get_fd($rrd_struc, RRDTOOL_PIPE_CHILD_READ));
		}
	}elseif ($config["cacti_server_os"] == "win32") {
		/* an empty $rrd_struc array means no fp is available */
		if (sizeof($rrd_struc) == 0) {
			$fp = popen(read_config_option("path_rrdtool") . escape_command(" $command_line"), "rb");
		}else{
			fwrite(rrd_get_fd($rrd_struc, RRDTOOL_PIPE_CHILD_READ), escape_command(" $command_line") . "\r\n");
			fflush(rrd_get_fd($rrd_struc, RRDTOOL_PIPE_CHILD_READ));
		}
	}

	switch ($output_flag) {
		case RRDTOOL_OUTPUT_NULL:
			return; break;
		case RRDTOOL_OUTPUT_STDOUT:
			if (isset($fp)) {
				$line = "";
				while (!feof($fp)) {
					$line .= fgets($fp, 4096);
				}

				return $line;
			}

			break;
		case RRDTOOL_OUTPUT_STDERR:
			if (isset($fp)) {
				$output = fgets($fp, 1000000);

				if (substr($output, 1, 3) == "PNG") {
					return "OK";
				}

				if (substr($output, 0, 5) == "GIF87") {
					return "OK";
				}

				print $output;
			}

			break;
		case RRDTOOL_OUTPUT_GRAPH_DATA:
			if (isset($fp)) {
				return fpassthru($fp);
			}

			break;
	}
}

function rrdtool_function_create($local_data_id, $show_source, $rrd_struc) {
	global $config;

	include ($config["include_path"] . "/config_arrays.php");

	$data_source_path = get_data_source_path($local_data_id, true);

	/* ok, if that passes lets check to make sure an rra does not already
	exist, the last thing we want to do is overright data! */
	if ($show_source != true) {
		if (file_exists($data_source_path) == true) {
			return -1;
		}
	}

	/* the first thing we must do is make sure there is at least one
	rra associated with this data source... *
	UPDATE: As of version 0.6.6, we are splitting this up into two
	SQL strings because of the multiple DS per RRD support. This is
	not a big deal however since this function gets called once per
	data source */

	$rras = db_fetch_assoc("select
		data_template_data.rrd_step,
		rra.x_files_factor,
		rra.steps,
		rra.rows,
		rra_cf.consolidation_function_id,
		(rra.rows*rra.steps) as rra_order
		from data_template_data
		left join data_template_data_rra on data_template_data.id=data_template_data_rra.data_template_data_id
		left join rra on data_template_data_rra.rra_id=rra.id
		left join rra_cf on rra.id=rra_cf.rra_id
		where data_template_data.local_data_id=$local_data_id
		and (rra.steps is not null or rra.rows is not null)
		order by rra_cf.consolidation_function_id,rra_order");

	/* if we find that this DS has no RRA associated; get out */
	if (sizeof($rras) <= 0) {
		cacti_log("ERROR: There are no RRA's assigned to local_data_id: $local_data_id.");
		return false;
	}

	/* create the "--step" line */
	$create_ds = RRD_NL . "--step ". $rras[0]["rrd_step"] . " " . RRD_NL;

	/* query the data sources to be used in this .rrd file */
	$data_sources = db_fetch_assoc("select
		data_template_rrd.id,
		data_template_rrd.rrd_heartbeat,
		data_template_rrd.rrd_minimum,
		data_template_rrd.rrd_maximum,
		data_template_rrd.data_source_type_id
		from data_template_rrd
		where data_template_rrd.local_data_id=$local_data_id");

	/* ONLY make a new DS entry if:
	- There is multiple data sources and this item is not the main one.
	- There is only one data source (then use it) */

	if (sizeof($data_sources) > 0) {
	foreach ($data_sources as $data_source) {
		/* use the cacti ds name by default or the user defined one, if entered */
		$data_source_name = get_data_source_item_name($data_source["id"]);

		$create_ds .= "DS:$data_source_name:" . $data_source_types{$data_source["data_source_type_id"]} . ":" . $data_source["rrd_heartbeat"] . ":" . $data_source["rrd_minimum"] . ":" . (empty($data_source["rrd_maximum"]) ? "U" : $data_source["rrd_maximum"]) . RRD_NL;
	}
	}

	$create_rra = "";
	/* loop through each available RRA for this DS */
	foreach ($rras as $rra) {
		$create_rra .= "RRA:" . $consolidation_functions{$rra["consolidation_function_id"]} . ":" . $rra["x_files_factor"] . ":" . $rra["steps"] . ":" . $rra["rows"] . RRD_NL;
	}

	if ($show_source == true) {
		return read_config_option("path_rrdtool") . " create" . RRD_NL . "$data_source_path$create_ds$create_rra";
	}else{
		rrdtool_execute("create $data_source_path $create_ds$create_rra", true, RRDTOOL_OUTPUT_STDOUT, $rrd_struc, "POLLER");
	}
}

function rrdtool_function_update($update_cache_array, $rrd_struc) {
	while (list($rrd_path, $rrd_fields) = each($update_cache_array)) {
		$create_rrd_file = false;

		/* create the rrd if one does not already exist */
		if (!file_exists($rrd_path)) {
			rrdtool_function_create($rrd_fields["local_data_id"], false, $rrd_struc);

			$create_rrd_file = true;
		}

		if ((is_array($rrd_fields["times"])) && (sizeof($rrd_fields["times"]) > 0)) {
			ksort($rrd_fields["times"]);

			while (list($update_time, $field_array) = each($rrd_fields["times"])) {
				if (empty($update_time)) {
					/* default the rrdupdate time to now */
					$current_rrd_update_time = "N";
				}else if ($create_rrd_file == true) {
					/* for some reason rrdtool will not let you update using times less than the
					rrd create time */
					$current_rrd_update_time = "N";
				}else{
					$current_rrd_update_time = $update_time;
				}

				$i = 0; $rrd_update_template = ""; $rrd_update_values = $current_rrd_update_time . ":";
				while (list($field_name, $value) = each($field_array)) {
					$rrd_update_template .= $field_name;

					/* if we have "invalid data", give rrdtool an Unknown (U) */
					if ((!isset($value)) || (!is_numeric($value))) {
						$value = "U";
					}

					$rrd_update_values .= $value;

					if (($i+1) < count($field_array)) {
						$rrd_update_template .= ":";
						$rrd_update_values .= ":";
					}

					$i++;
				}

				rrdtool_execute("update $rrd_path --template $rrd_update_template $rrd_update_values", true, RRDTOOL_OUTPUT_STDOUT, $rrd_struc, "POLLER");
			}
		}
	}
}

function rrdtool_function_tune($rrd_tune_array) {
	global $config;

	include($config["include_path"] . "/config_arrays.php");

	$data_source_name = get_data_source_item_name($rrd_tune_array["data_source_id"]);
	$data_source_type = $data_source_types{$rrd_tune_array["data-source-type"]};
	$data_source_path = get_data_source_path($rrd_tune_array["data_source_id"], true);

	if ($rrd_tune_array["heartbeat"] != "") {
		$rrd_tune .= " --heartbeat $data_source_name:" . $rrd_tune_array["heartbeat"];
	}

	if ($rrd_tune_array["minimum"] != "") {
		$rrd_tune .= " --minimum $data_source_name:" . $rrd_tune_array["minimum"];
	}

	if ($rrd_tune_array["maximum"] != "") {
		$rrd_tune .= " --maximum $data_source_name:" . $rrd_tune_array["maximum"];
	}

	if ($rrd_tune_array["data-source-type"] != "") {
		$rrd_tune .= " --data-source-type $data_source_name:" . $data_source_type;
	}

	if ($rrd_tune_array["data-source-rename"] != "") {
		$rrd_tune .= " --data-source-rename $data_source_name:" . $rrd_tune_array["data-source-rename"];
	}

	if ($rrd_tune != "") {
		if (file_exists($data_source_path) == true) {
			$fp = popen(read_config_option("path_rrdtool") . " tune $data_source_path $rrd_tune", "r");
			pclose($fp);

			if (read_config_option("log_verbosity") >= POLLER_VERBOSITY_DEBUG) {
				cacti_log("CACTI2RRD: " . read_config_option("path_rrdtool") . " tune $data_source_path $rrd_tune");
			}
		}
	}
}

/* rrdtool_function_fetch - given a data source, return all of its data in an array
   @arg $local_data_id - the data source to fetch data for
   @arg $start_time - the start time to use for the data calculation. this value can
     either be absolute (unix timestamp) or relative (to now)
   @arg $end_time - the end time to use for the data calculation. this value can
     either be absolute (unix timestamp) or relative (to now)
   @arg $resolution - the accuracy of the data measured in seconds
   @returns - (array) an array containing all data in this data source broken down
	 by each data source item. the maximum of all data source items is included in
	 an item called 'ninety_fifth_percentile_maximum' */
function &rrdtool_function_fetch($local_data_id, $start_time, $end_time) {
	if (empty($local_data_id)) {
		return;
	}

	$regexps = array();
	$fetch_array = array();

	$data_source_path = get_data_source_path($local_data_id, true);

	/* build and run the rrdtool fetch command with all of our data */
	$output = rrdtool_execute("fetch $data_source_path AVERAGE -s $start_time -e $end_time", false, RRDTOOL_OUTPUT_STDOUT);

	/* grab the first line of the output which contains a list of data sources
	in this .rrd file */
	$line_one = substr($output, 0, strpos($output, "\n"));

	/* loop through each data source in this .rrd file ... */
	if (preg_match_all("/\w+/", $line_one, $data_source_names)) {
		/* version 1.0.49 changed the output slightly */
		if (preg_match("/^timestamp/", $line_one)) {
			array_shift($data_source_names[0]);
		}

		$fetch_array["data_source_names"] = $data_source_names[0];

		/* build a unique regexp to match each data source individually when
		passed to preg_match_all() */
		for ($i=0;$i<count($fetch_array["data_source_names"]);$i++) {
			$regexps[$i] = '/[0-9]+:\s+';

			for ($j=0;$j<count($fetch_array["data_source_names"]);$j++) {
				/* it seems that at least some versions of the Windows RRDTool binary pads
				the exponent to 3 digits, rather than 2 on every Unix version that I have
				ever seen */
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

	/* loop through each regexp determined above (or each data source) */
	for ($i=0;$i<count($regexps);$i++) {
		$fetch_array["values"][$i] = array();

		/* match the regexp against the rrdtool fetch output to get a mantisa and
		exponent for each line */
		if (preg_match_all($regexps[$i], $output, $matches)) {
			for ($j=0; ($j < count($matches[1])); $j++) {
				$line = ($matches[1][$j] * (pow(10,(float)$matches[2][$j])));
				array_push($fetch_array["values"][$i], ($line * 1));

				$max_array[$j][$i] = $line;
			}
		}
	}

	if (isset($fetch_array["data_source_names"])) {
		$next_index = count($fetch_array["data_source_names"]);

		$fetch_array["data_source_names"][$next_index] = "ninety_fifth_percentile_maximum";

		/* calculate the max for each row */
		for ($i=0; $i<count($max_array); $i++) {
			$fetch_array["values"][$next_index][$i] = max($max_array[$i]);
		}
	}

	return $fetch_array;
}

function rrdtool_function_graph($local_graph_id, $rra_id, $graph_data_array, $rrd_struc = array()) {
	global $config;

	include_once($config["library_path"] . "/cdef.php");
	include_once($config["library_path"] . "/graph_variables.php");
	include($config["include_path"] . "/config_arrays.php");

	/* before we do anything; make sure the user has permission to view this graph,
	if not then get out */
	if ((read_config_option("global_auth") == "on") && (isset($_SESSION["sess_user_id"]))) {
		$access_denied = !(is_graph_allowed($local_graph_id));

		if ($access_denied == true) {
			return "GRAPH ACCESS DENIED";
		}
	}

	/* find the step and how often this graph is updated with new data */
	$ds_step = db_fetch_cell("select
		data_template_data.rrd_step
		from data_template_data,data_template_rrd,graph_templates_item
		where graph_templates_item.task_item_id=data_template_rrd.id
		and data_template_rrd.local_data_id=data_template_data.local_data_id
		and graph_templates_item.local_graph_id=$local_graph_id
		limit 0,1");
	$ds_step = empty($ds_step) ? 300 : $ds_step;

	/* if no rra was specified, we need to figure out which one RRDTool will choose using
	 * "best-fit" resolution fit algorithm */
	if (empty($rra_id)) {
		if ((empty($graph_data_array["graph_start"])) || (empty($graph_data_array["graph_end"]))) {
			$rra["rows"] = 600;
			$rra["steps"] = 1;
			$rra["timespan"] = 86400;
		}else{
			/* get a list of RRAs related to this graph */
			$rras = get_associated_rras($local_graph_id);

			if (sizeof($rras) > 0) {
				foreach ($rras as $unchosen_rra) {
					/* the timespan specified in the RRA "timespan" field may not be accurate */
					$real_timespan = ($ds_step * $unchosen_rra["steps"] * $unchosen_rra["rows"]);

					/* make sure the current start/end times fit within each RRA's timespan */
					if ( (($graph_data_array["graph_end"] - $graph_data_array["graph_start"]) <= $real_timespan) && ((time() - $graph_data_array["graph_start"]) <= $real_timespan) ) {
						/* is this RRA better than the already chosen one? */
						if ((isset($rra)) && ($unchosen_rra["steps"] < $rra["steps"])) {
							$rra = $unchosen_rra;
						}else if (!isset($rra)) {
							$rra = $unchosen_rra;
						}
					}
				}
			}

			if (!isset($rra)) {
				$rra["rows"] = 600;
				$rra["steps"] = 1;
			}
		}
	}else{
		$rra = db_fetch_row("select timespan,rows,steps from rra where id=$rra_id");
	}

	$seconds_between_graph_updates = ($ds_step * $rra["steps"]);

	$graph = db_fetch_row("select
		graph_local.host_id,
		graph_local.snmp_query_id,
		graph_local.snmp_index,
		graph_templates_graph.title_cache,
		graph_templates_graph.vertical_label,
		graph_templates_graph.auto_scale,
		graph_templates_graph.auto_scale_opts,
		graph_templates_graph.auto_scale_log,
		graph_templates_graph.auto_scale_rigid,
		graph_templates_graph.auto_padding,
		graph_templates_graph.base_value,
		graph_templates_graph.upper_limit,
		graph_templates_graph.lower_limit,
		graph_templates_graph.height,
		graph_templates_graph.width,
		graph_templates_graph.image_format_id,
		graph_templates_graph.unit_value,
		graph_templates_graph.unit_exponent_value,
		graph_templates_graph.export
		from graph_templates_graph,graph_local
		where graph_local.id=graph_templates_graph.local_graph_id
		and graph_templates_graph.local_graph_id=$local_graph_id");

	/* lets make that sql query... */
	$graph_items = db_fetch_assoc("select
		graph_templates_item.id as graph_templates_item_id,
		graph_templates_item.cdef_id,
		graph_templates_item.text_format,
		graph_templates_item.value,
		graph_templates_item.hard_return,
		graph_templates_item.consolidation_function_id,
		graph_templates_item.graph_type_id,
		graph_templates_gprint.gprint_text,
		colors.hex,
		data_template_rrd.id as data_template_rrd_id,
		data_template_rrd.local_data_id,
		data_template_rrd.rrd_minimum,
		data_template_rrd.rrd_maximum,
		data_template_rrd.data_source_name,
		data_template_rrd.local_data_template_rrd_id
		from graph_templates_item
		left join data_template_rrd on graph_templates_item.task_item_id=data_template_rrd.id
		left join colors on graph_templates_item.color_id=colors.id
		left join graph_templates_gprint on graph_templates_item.gprint_id=graph_templates_gprint.id
		where graph_templates_item.local_graph_id=$local_graph_id
		order by graph_templates_item.sequence");

	/* +++++++++++++++++++++++ GRAPH OPTIONS +++++++++++++++++++++++ */

	/* define some variables */
	$scale = "";
	$rigid = "";
	$unit_exponent_value = "";
	$graph_legend = "";
	$graph_defs = "";
	$txt_graph_items = "";
	$text_padding = "";
	$greatest_text_format = 0;
	$last_graph_type = "";

		if ($graph["auto_scale"] == "on") {
		if ($graph["auto_scale_opts"] == "1") {
			$scale = "--alt-autoscale" . RRD_NL;
		}elseif ($graph["auto_scale_opts"] == "2") {
			$scale = "--alt-autoscale-max" . RRD_NL;
			$scale .= "--lower-limit=" . $graph["lower_limit"] . RRD_NL;
		}

		if ($graph["auto_scale_log"] == "on") {
			$scale .= "--logarithmic" . RRD_NL;
		}
	}else{
		$scale =  "--upper-limit=" . $graph["upper_limit"] . RRD_NL;
		$scale .= "--lower-limit=" . $graph["lower_limit"] . RRD_NL;
	}

	if ($graph["auto_scale_rigid"] == "on") {
		$rigid = "--rigid" . RRD_NL;
	}

	if (!empty($graph["unit_value"])) {
		$unit_value = "--unit=" . $graph["unit_value"] . RRD_NL;
	}

	if (ereg("^[0-9]+$", $graph["unit_exponent_value"])) {
		$unit_exponent_value = "--units-exponent=" . $graph["unit_exponent_value"] . RRD_NL;
	}

	/*
	 * optionally you can specify and array that overrides some of the db's values, lets set
	 * that all up here
	 */

	/* override: graph start time */
	if ((!isset($graph_data_array["graph_start"])) || ($graph_data_array["graph_start"] == "0")) {
		$graph_start = -($rra["timespan"]);
	}else{
		$graph_start = $graph_data_array["graph_start"];
	}

	/* override: graph end time */
	if ((!isset($graph_data_array["graph_end"])) || ($graph_data_array["graph_end"] == "0")) {
		$graph_end = -($seconds_between_graph_updates);
	}else{
		$graph_end = $graph_data_array["graph_end"];
	}

	/* override: graph height (in pixels) */
	if (isset($graph_data_array["graph_height"])) {
		$graph_height = $graph_data_array["graph_height"];
	}else{
		$graph_height = $graph["height"];
	}

	/* override: graph width (in pixels) */
	if (isset($graph_data_array["graph_width"])) {
		$graph_width = $graph_data_array["graph_width"];
	}else{
		$graph_width = $graph["width"];
	}

	/* override: skip drawing the legend? */
	if (isset($graph_data_array["graph_nolegend"])) {
		$graph_legend = "--no-legend" . RRD_NL;
	}else{
		$graph_legend = "";
	}

	/* export options */
	if (isset($graph_data_array["export"])) {
		$graph_opts = read_config_option("path_html_export") . "/" . $graph_data_array["export_filename"] . RRD_NL;
	}else{
		if (empty($graph_data_array["output_filename"])) {
				$graph_opts = "-" . RRD_NL;
		}else{
			$graph_opts = $graph_data_array["output_filename"] . RRD_NL;
		}
	}

	/* setup date format */
	$date_fmt = read_graph_config_option("default_date_format");
	$datechar = read_graph_config_option("default_datechar");

	if ($datechar == GDC_HYPHEN) {
		$datechar = "-";
	}else {
		$datechar = "/";
	}

	switch ($date_fmt) {
		case GD_MO_D_Y:
			$graph_date = "m" . $datechar . "d" . $datechar . "Y H:i:s";
			break;
		case GD_MN_D_Y:
			$graph_date = "M" . $datechar . "d" . $datechar . "Y H:i:s";
			break;
		case GD_D_MO_Y:
			$graph_date = "d" . $datechar . "m" . $datechar . "Y H:i:s";
			break;
		case GD_D_MN_Y:
			$graph_date = "d" . $datechar . "M" . $datechar . "Y H:i:s";
			break;
		case GD_Y_MO_D:
			$graph_date = "Y" . $datechar . "m" . $datechar . "d H:i:s";
			break;
		case GD_Y_MN_D:
			$graph_date = "Y" . $datechar . "M" . $datechar . "d H:i:s";
			break;
	}

	/* display the timespan for zoomed graphs */
	if ((isset($graph_data_array["graph_start"])) && (isset($graph_data_array["graph_end"]))) {
		if (($graph_data_array["graph_start"] < 0) && ($graph_data_array["graph_end"] < 0)) {
			$graph_legend .= "COMMENT:\"From " . date($graph_date, time()+$graph_data_array["graph_start"]) . " To " . date($graph_date, time()+$graph_data_array["graph_end"]) . "\\c\"" . RRD_NL . "COMMENT:\"\\n\"" . RRD_NL;
		}else if (($graph_data_array["graph_start"] >= 0) && ($graph_data_array["graph_end"] >= 0)) {
			$graph_legend .= "COMMENT:\"From " . date($graph_date, $graph_data_array["graph_start"]) . " To " . date($graph_date, $graph_data_array["graph_end"]) . "\\c\"" . RRD_NL . "COMMENT:\"\\n\"" . RRD_NL;
		}
	}

	/* basic graph options */
	// awie 03/12/2004
	if(preg_match("/\|query_[a-zA-Z0-9_]+\|/", $graph["title_cache"],$_mm)) {
		$_title=$_mm[0];
		$graph["title_cache"]=str_replace($_title,'',$graph["title_cache"]);
	}
	$graph_opts .=
		"--imgformat=" . $image_types{$graph["image_format_id"]} . RRD_NL .
		"--start=$graph_start" . RRD_NL .
		"--end=$graph_end" . RRD_NL .
		"--title=\"" . $graph["title_cache"] . "\"" . RRD_NL .
		"$rigid" .
		"--base=" . $graph["base_value"] . RRD_NL .
		"--height=$graph_height" . RRD_NL .
		"--width=$graph_width" . RRD_NL .
		"$scale" .
		"$unit_exponent_value" .
		"$graph_legend" .
		"--vertical-label=\"" . $graph["vertical_label"] . "\"" . RRD_NL;

	$i = 0;
	if (sizeof($graph_items > 0)) {
	foreach ($graph_items as $graph_item) {
		if ((ereg("(AREA|STACK|LINE[123])", $graph_item_types{$graph_item["graph_type_id"]})) && ($graph_item["data_source_name"] != "")) {
			/* use a user-specified ds path if one is entered */
			$data_source_path = get_data_source_path($graph_item["local_data_id"], true);

			/* FOR WIN32: Escape all colon for drive letters (ex. D\:/path/to/rra) */
			$data_source_path = str_replace(":", "\:", $data_source_path);

			if (!empty($data_source_path)) {
				/* NOTE: (Update) Data source DEF names are created using the graph_item_id; then passed
				to a function that matches the digits with letters. rrdtool likes letters instead
				of numbers in DEF names; especially with CDEF's. cdef's are created
				the same way, except a 'cdef' is put on the beginning of the hash */
				$graph_defs .= "DEF:" . generate_graph_def_name(strval($i)) . "=\"$data_source_path\":" . $graph_item["data_source_name"] . ":" . $consolidation_functions{$graph_item["consolidation_function_id"]} . RRD_NL;

				//print "ds: " . $graph_item["data_template_rrd_id"] . "<br>";
				$cf_ds_cache{$graph_item["data_template_rrd_id"]}{$graph_item["consolidation_function_id"]} = "$i";

				$i++;
			}
		}

		/* +++++++++++++++++++++++ LEGEND: TEXT SUBSITUTION (<>'s) +++++++++++++++++++++++ */

		/* note the current item_id for easy access */
		$graph_item_id = $graph_item["graph_templates_item_id"];

		/* the following fields will be searched for graph variables */
		$variable_fields = array(
			"text_format" => array(
				"process_no_legend" => false
				),
			"value" => array(
				"process_no_legend" => true
				)
			);

		/* loop through each field that we want to substitute values for:
		currently: text format and value */
		while (list($field_name, $field_array) = each($variable_fields)) {
			/* certain fields do not require values when the legend is not to be shown */
			if (($field_array["process_no_legend"] == false) && (isset($graph_data_array["graph_nolegend"]))) {
				continue;
			}

			$graph_variables[$field_name][$graph_item_id] = $graph_item[$field_name];

			/* date/time substitution */
			if (strstr($graph_variables[$field_name][$graph_item_id], "|date_time|")) {
				$graph_variables[$field_name][$graph_item_id] = str_replace("|date_time|", date('D d M H:i:s T Y', strtotime(db_fetch_cell("select value from settings where name='date'"))), $graph_variables[$field_name][$graph_item_id]);
			}

			/* data query variables */
			if (preg_match("/\|query_[a-zA-Z0-9_]+\|/", $graph_variables[$field_name][$graph_item_id])) {
				/* default to the graph data query information from the graph */
				if (empty($graph_item["local_data_id"])) {
					$graph_variables[$field_name][$graph_item_id] = substitute_snmp_query_data($graph_variables[$field_name][$graph_item_id], $graph["host_id"], $graph["snmp_query_id"], $graph["snmp_index"]);
				/* use the data query information from the data source if possible */
				}else{
					$data_local = db_fetch_row("select snmp_index,snmp_query_id,host_id from data_local where id='" . $graph_item["local_data_id"] . "'");
					$graph_variables[$field_name][$graph_item_id] = substitute_snmp_query_data($graph_variables[$field_name][$graph_item_id], $data_local["host_id"], $data_local["snmp_query_id"], $data_local["snmp_index"]);
				}
				if(!$_cnt) $graph_variables[$field_name][$graph_item_id]='unknown';
			}

			/* 95th percentile */
			if (preg_match_all("/\|95:(bits|bytes):(\d):(current|total|max)(:(\d))?\|/", $graph_variables[$field_name][$graph_item_id], $matches, PREG_SET_ORDER)) {
				foreach ($matches as $match) {
					$graph_variables[$field_name][$graph_item_id] = str_replace($match[0], variable_ninety_fifth_percentile($match, $graph_item, $graph_items, $graph_start, $graph_end), $graph_variables[$field_name][$graph_item_id]);
				}
			}

			/* bandwidth summation */
			if (preg_match_all("/\|sum:(\d|auto):(current|total):(\d):(\d+|auto)\|/", $graph_variables[$field_name][$graph_item_id], $matches, PREG_SET_ORDER)) {
				foreach ($matches as $match) {
					$graph_variables[$field_name][$graph_item_id] = str_replace($match[0], variable_bandwidth_summation($match, $graph_item, $graph_items, $graph_start, $graph_end, $rra["steps"], $ds_step), $graph_variables[$field_name][$graph_item_id]);
				}
			}
		}

		/* if we are not displaying a legend there is no point in us even processing the auto padding,
		text format stuff. */
		if (!isset($graph_data_array["graph_nolegend"])) {
			/* set hard return variable if selected (\n) */
			if ($graph_item["hard_return"] == "on") {
				$hardreturn[$graph_item_id] = "\\n";
			}else{
				$hardreturn[$graph_item_id] = "";
			}

			/* +++++++++++++++++++++++ LEGEND: AUTO PADDING (<>'s) +++++++++++++++++++++++ */

			/* PADDING: remember this is not perfect! its main use is for the basic graph setup of:
			AREA - GPRINT-CURRENT - GPRINT-AVERAGE - GPRINT-MAXIMUM \n
			of course it can be used in other situations, however may not work as intended.
			If you have any additions to this small peice of code, feel free to send them to me. */
			if ($graph["auto_padding"] == "on") {
				/* only applies to AREA and STACK */
				if (ereg("(AREA|STACK|LINE[123])", $graph_item_types{$graph_item["graph_type_id"]})) {
					$text_format_lengths{$graph_item["data_template_rrd_id"]} = strlen($graph_variables["text_format"][$graph_item_id]);

					if ((strlen($graph_variables["text_format"][$graph_item_id]) > $greatest_text_format) && ($graph_item_types{$graph_item["graph_type_id"]} != "COMMENT")) {
						$greatest_text_format = strlen($graph_variables["text_format"][$graph_item_id]);
					}
				}
			}
		}
	}
	}

		/* +++++++++++++++++++++++ GRAPH ITEMS: CDEF's +++++++++++++++++++++++ */

	$i = 0;
	reset($graph_items);

	if (sizeof($graph_items) > 0) {
	foreach ($graph_items as $graph_item) {
		/* first we need to check if there is a DEF for the current data source/cf combination. if so,
		we will use that */
		if (isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}{$graph_item["consolidation_function_id"]})) {
			$cf_id = $graph_item["consolidation_function_id"];
		}else{
		/* if there is not a DEF defined for the current data source/cf combination, then we will have to
		improvise. choose the first available cf in the following order: AVERAGE, MAX, MIN, LAST */
			if (isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}[1])) {
				$cf_id = 1; /* CF: AVERAGE */
			}elseif (isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}[3])) {
				$cf_id = 3; /* CF: MAX */
			}elseif (isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}[2])) {
				$cf_id = 2; /* CF: MIN */
			}elseif (isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}[4])) {
				$cf_id = 4; /* CF: LAST */
			}else{
				$cf_id = 1; /* CF: AVERAGE */
			}
		}

		/* make cdef string here; a note about CDEF's in cacti. A CDEF is neither unique to a
		data source of global cdef, but is unique when those two variables combine. */
		$cdef_graph_defs = ""; $cdef_total_ds = ""; $cdef_total = ""; $cdef_similar_ds = ""; $cdef_similar = "";

		if ((!empty($graph_item["cdef_id"])) && (!isset($cdef_cache{$graph_item["cdef_id"]}{$graph_item["data_template_rrd_id"]}[$cf_id]))) {
			$cdef_string = get_cdef($graph_item["cdef_id"]);

			/* create cdef string for "total all data sources" if requested */
			if (ereg("ALL_DATA_SOURCES_(NO)?DUPS", $cdef_string)) {
				$item_count = 0;
				for ($t=0;($t<count($graph_items));$t++) {
					if ((ereg("(AREA|STACK|LINE[123])", $graph_item_types{$graph_items[$t]["graph_type_id"]})) && (!empty($graph_items[$t]["data_template_rrd_id"]))) {
						/* if the user screws up CF settings, PHP will generate warnings if left unchecked */
						if (isset($cf_ds_cache{$graph_items[$t]["data_template_rrd_id"]}[$cf_id])) {
							$def_name = generate_graph_def_name(strval($cf_ds_cache{$graph_items[$t]["data_template_rrd_id"]}[$cf_id]));
							$cdef_total_ds .= "TIME," . (time() - $seconds_between_graph_updates) . ",GT,$def_name,$def_name,UN,0,$def_name,IF,IF,"; /* convert unknowns to '0' first */
							$item_count++;
						}
					}
				}

				/* if there is only one item to total, don't even bother with the summation. otherwise
				cdef=a,b,c,+,+ is fine. */
				if ($item_count == 1) {
					$cdef_total = str_replace(",", "", $cdef_total_ds);
				}else{
					$cdef_total = $cdef_total_ds . str_repeat("+,", max(0, ($item_count - 2))) . "+";
				}
			}

			/* create cdef string for "total similar data sources" if requested */
			if (ereg("SIMILAR_DATA_SOURCES_(NO)?DUPS", $cdef_string) ) {
				$sources_seen = array();
				$item_count = 0;

				for ($t=0;($t<count($graph_items));$t++) {
					if ((ereg("(AREA|STACK|LINE[123])", $graph_item_types{$graph_items[$t]["graph_type_id"]})) && (!empty($graph_items[$t]["data_template_rrd_id"])) && ($graph_item["data_source_name"] == $graph_items[$t]["data_source_name"]) && ($graph_item["graph_templates_item_id"] != $graph_items[$t]["graph_templates_item_id"])) {
						/* if the user screws up CF settings, PHP will generate warnings if left unchecked */
						if (isset($cf_ds_cache{$graph_items[$t]["data_template_rrd_id"]}[$cf_id]) && (!isset($sources_seen{$graph_items[$t]["data_template_rrd_id"]}))) {
							$def_name = generate_graph_def_name(strval($cf_ds_cache{$graph_items[$t]["data_template_rrd_id"]}[$cf_id]));
							$cdef_similar_ds .= "TIME," . (time() - $seconds_between_graph_updates) . ",GT,$def_name,$def_name,UN,0,$def_name,IF,IF,"; /* convert unknowns to '0' first */
							$sources_seen{$graph_items[$t]["data_template_rrd_id"]} = 1;
							$item_count++;
						}
					}
				}

				/* if there is only one item to total, don't even bother with the summation. otherwise
				cdef=a,b,c,+,+ is fine. */
				if ($item_count == 1) {
					$cdef_similar = str_replace(",", "", $cdef_similar_ds);
				}else{
					$cdef_similar = $cdef_similar_ds . str_repeat("+,", ($item_count - 2)) . "+";
				}
			}

			$cdef_string = str_replace("CURRENT_DATA_SOURCE", generate_graph_def_name(strval((isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}[$cf_id]) ? $cf_ds_cache{$graph_item["data_template_rrd_id"]}[$cf_id] : "0"))), $cdef_string);
			$cdef_string = str_replace("ALL_DATA_SOURCES_NODUPS", $cdef_total, $cdef_string);
			$cdef_string = str_replace("SIMILAR_DATA_SOURCES_NODUPS", $cdef_similar, $cdef_string);

			/* data source item variables */
			$cdef_string = str_replace("CURRENT_DS_MINIMUM_VALUE", (empty($graph_item["rrd_minimum"]) ? "0" : $graph_item["rrd_minimum"]), $cdef_string);
			$cdef_string = str_replace("CURRENT_DS_MAXIMUM_VALUE", (empty($graph_item["rrd_maximum"]) ? "0" : $graph_item["rrd_maximum"]), $cdef_string);
			$cdef_string = str_replace("CURRENT_GRAPH_MINIMUM_VALUE", (empty($graph["lower_limit"]) ? "0" : $graph["lower_limit"]), $cdef_string);
			$cdef_string = str_replace("CURRENT_GRAPH_MAXIMUM_VALUE", (empty($graph["upper_limit"]) ? "0" : $graph["upper_limit"]), $cdef_string);

			/* make the initial "virtual" cdef name: 'cdef' + [a,b,c,d...] */
			$cdef_graph_defs .= "CDEF:cdef" . generate_graph_def_name(strval($i)) . "=";
			$cdef_graph_defs .= $cdef_string;
			$cdef_graph_defs .= " \\\n";

			/* the CDEF cache is so we do not create duplicate CDEF's on a graph */
			$cdef_cache{$graph_item["cdef_id"]}{$graph_item["data_template_rrd_id"]}[$cf_id] = "$i";
		}

		/* add the cdef string to the end of the def string */
		$graph_defs .= $cdef_graph_defs;

		/* note the current item_id for easy access */
		$graph_item_id = $graph_item["graph_templates_item_id"];

		/* if we are not displaying a legend there is no point in us even processing the auto padding,
		text format stuff. */
		if ((!isset($graph_data_array["graph_nolegend"])) && ($graph["auto_padding"] == "on") && (isset($text_format_lengths{$graph_item["data_template_rrd_id"]}))) {
			/* we are basing how much to pad on area and stack text format,
			not gprint. but of course the padding has to be displayed in gprint,
			how fun! */

			$pad_number = ($greatest_text_format - $text_format_lengths{$graph_item["data_template_rrd_id"]});
			//cacti_log("MAX: $greatest_text_format, CURR: $text_format_lengths[$item_dsid], DSID: $item_dsid");
			$text_padding = str_pad("", $pad_number);

			/* two GPRINT's in a row screws up the padding, lets not do that */
			if (($graph_item_types{$graph_item["graph_type_id"]} == "GPRINT") && ($last_graph_type == "GPRINT")) {
				$text_padding = "";
			}

			$last_graph_type = $graph_item_types{$graph_item["graph_type_id"]};
		}

		/* we put this in a variable so it can be manipulated before mainly used
		if we want to skip it, like below */
		$current_graph_item_type = $graph_item_types{$graph_item["graph_type_id"]};

		/* IF this graph item has a data source... get a DEF name for it, or the cdef if that applies
		to this graph item */
		if ($graph_item["cdef_id"] == "0") {
			if (isset($cf_ds_cache{$graph_item["data_template_rrd_id"]}[$cf_id])) {
				$data_source_name = generate_graph_def_name(strval($cf_ds_cache{$graph_item["data_template_rrd_id"]}[$cf_id]));
			}else{
				$data_source_name = "";
			}
		}else{
			$data_source_name = "cdef" . generate_graph_def_name(strval($cdef_cache{$graph_item["cdef_id"]}{$graph_item["data_template_rrd_id"]}[$cf_id]));
		}

		/* to make things easier... if there is no text format set; set blank text */
		if (!isset($graph_variables["text_format"][$graph_item_id])) {
			$graph_variables["text_format"][$graph_item_id] = "";
		}

		if (!isset($hardreturn[$graph_item_id])) {
			$hardreturn[$graph_item_id] = "";
		}

		/* +++++++++++++++++++++++ GRAPH ITEMS +++++++++++++++++++++++ */

		/* most of the calculations have been done above. now we have for print everything out
		in an RRDTool-friendly fashion */
		if (ereg("^(AREA|STACK|LINE[123])$", $graph_item_types{$graph_item["graph_type_id"]})) {
			$graph_variables["text_format"][$graph_item_id] = str_replace(":", "\:", $graph_variables["text_format"][$graph_item_id]); /* escape colons */
			$txt_graph_items .= $graph_item_types{$graph_item["graph_type_id"]} . ":" . $data_source_name . "#" . $graph_item["hex"] . ":" . "\"" . $graph_variables["text_format"][$graph_item_id] . $hardreturn[$graph_item_id] . "\" ";
		}elseif ($graph_item_types{$graph_item["graph_type_id"]} == "COMMENT") {
			$txt_graph_items .= $graph_item_types{$graph_item["graph_type_id"]} . ":\"" . $graph_variables["text_format"][$graph_item_id] . $hardreturn[$graph_item_id] . "\" ";
		}elseif (($graph_item_types{$graph_item["graph_type_id"]} == "GPRINT") && (!isset($graph_data_array["graph_nolegend"]))) {
			$graph_variables["text_format"][$graph_item_id] = str_replace(":", "\:", $graph_variables["text_format"][$graph_item_id]); /* escape colons */
			$txt_graph_items .= $graph_item_types{$graph_item["graph_type_id"]} . ":" . $data_source_name . ":" . $consolidation_functions{$graph_item["consolidation_function_id"]} . ":\"$text_padding" . $graph_variables["text_format"][$graph_item_id] . $graph_item["gprint_text"] . $hardreturn[$graph_item_id] . "\" ";
		}elseif ($graph_item_types{$graph_item["graph_type_id"]} == "HRULE") {
			$graph_variables["text_format"][$graph_item_id] = str_replace(":", "\:", $graph_variables["text_format"][$graph_item_id]); /* escape colons */
			$graph_variables["value"][$graph_item_id] = str_replace(":", "\:", $graph_variables["value"][$graph_item_id]); /* escape colons */
			$txt_graph_items .= $graph_item_types{$graph_item["graph_type_id"]} . ":" . $graph_variables["value"][$graph_item_id] . "#" . $graph_item["hex"] . ":\"" . $graph_variables["text_format"][$graph_item_id] . $hardreturn[$graph_item_id] . "\" ";
		}elseif ($graph_item_types{$graph_item["graph_type_id"]} == "VRULE") {
			$graph_variables["text_format"][$graph_item_id] = str_replace(":", "\:", $graph_variables["text_format"][$graph_item_id]); /* escape colons */

			$value_array = explode(":", $graph_item["value"]);

			if ($value_array[0] < 0) {
				$value = date("U") - (-3600 * $value_array[0]) - 60 * $value_array[1];
			}else{
				$value = date("U", mktime($value_array[0],$value_array[1],0));
			}

			$txt_graph_items .= $graph_item_types{$graph_item["graph_type_id"]} . ":" . $value . "#" . $graph_item["hex"] . ":\"" . $graph_variables["text_format"][$graph_item_id] . $hardreturn[$graph_item_id] . "\" ";
		}

		$i++;

		if ($i < sizeof($graph_items)) {
			$txt_graph_items .= RRD_NL;
		}
	}
	}

	/* either print out the source or pass the source onto rrdtool to get us a nice PNG */
	if (isset($graph_data_array["print_source"])) {
		print "<PRE>" . read_config_option("path_rrdtool") . " graph $graph_opts$graph_defs$txt_graph_items</PRE>";
	}else{
		if (isset($graph_data_array["export"])) {
			rrdtool_execute("graph $graph_opts$graph_defs$txt_graph_items", false, RRDTOOL_OUTPUT_NULL, $rrd_struc);
			return 0;
		}else{
			if (isset($graph_data_array["output_flag"])) {
				$output_flag = $graph_data_array["output_flag"];
			}else{
				$output_flag = RRDTOOL_OUTPUT_GRAPH_DATA;
			}

			return rrdtool_execute("graph $graph_opts$graph_defs$txt_graph_items", false, $output_flag, $rrd_struc);
		}
	}
}

?>
