<?php

class MPG123DC
{
	var $server_host;
	var $server_port;
	var $client_path;
	
	function MPG123DC($host, $port, $client)
	{
		if(!file_exists($client))
			return -1;
		
		$server_host=$host;
		$server_port=$port;
		$client_path=$client;		
	}
	
	function getStatus()
	{
		return "Connected to $server_host:$server_port thru $client_path";
	}
}

?>