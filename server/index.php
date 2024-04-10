<?php

include('settings.php');

define('HTML_PREFIX', '<!doctype html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" /></head><body>');
define('HTML_POSTFIX', '</body></html>');

function get_timestring() {
	return date("c");
}

function clean_ethernet_address($address) {
	return filter_var($address, FILTER_VALIDATE_MAC);
}

function add_log_entry($data) {
	$f = fopen(DATA_PATH, "a");
	fwrite($f, get_timestring()."\t$data\n");
	fclose($f);
}

function get_date($datetime) {
	$datetime_arr = explode(' ', $datetime, 2);
	if (sizeof($datetime_arr) > 1) {
		return $datetime_arr[0];
	}

	return null;
}

function get_device_statistics($device) {
	$statistics = [];

	$f = fopen(DATA_PATH, "r");

	while (($line = fgets($f)) !== false) {
		$line_array = explode("\t", $line, 2);
		$line_data = json_decode($line_array[1], true);
		if (strtoupper(@$line_data['device']) == $device) {
			$this_date = get_date(@$line_data['timestamp']);
			if ($this_date == '2000-00-00') {
				$statistics['no-gps']++;
			} elseif ($this_date) {
				$statistics[$this_date]++;
			} else {
				$statistics['no-date']++;
			}
		}
	}

	fclose($f);

	return $statistics;
}

function statistics_to_table($statistics) {
	$html = [];

	array_push($html, "<table>");
	array_push($html, "<tr><th>Date</th><th>Log entries</th></tr>");

	$dates = array_keys($statistics);
	sort($dates);

	foreach ($dates as $this_date) {
		array_push($html, "<tr><td>$this_date</td><td>$statistics[$this_date]</td></tr>");
	}

	array_push($html, "</table>");

	return implode("\n", $html);
}

function show_device_status($device) {
	$device_stats = get_device_statistics($device);

	echo(statistics_to_table($device_stats));
}

function show_query_page() {
	echo(HTML_PREFIX);
	echo('<h1>Enter device code</h1>
		<form method="post">
		<input type="text" name="device">
		<input type="submit" value="Show device data">
		</form>
		</body></html>');
	echo(HTML_POSTFIX);
}

if (@$_POST['data']) {
	add_log_entry($_POST['data']);
	header("Content-type: text/plain");
	echo("Data saved.\n");
	http_response_code(200);
}
elseif (@$_POST['device']) {
	$device = clean_ethernet_address(strtoupper($_POST['device']));
	if ($device) {
		header("Content-type: text/html");
		echo(HTML_PREFIX);
		echo("<h1>$device</h1>");
		show_device_status($device);
		echo(HTML_POSTFIX);
		http_response_code(200);
	} else {
		header("Content-type: text/plain");
		echo("Check your device code");
		http_response_code(200);
	}
}
else {
	header("Content-type: text/html");
	show_query_page();
	http_response_code(200);
}

?>
