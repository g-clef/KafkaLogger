module KafkaLogger;

export {

	# Logstash requires quotes around timestamp values, even if they 
	# are doubles/floats. If you're consuming this data via logstash
	# you will want this to be set to "T". If you're consuming
	# these logs with something else, that's expecting timestamps
	# to be raw double values, set this to "F".
	const logstash_style_timestamp = F &redef;

	# the name of the kafka topic to connect to & send messages to.
	const topic_name = "logs" &redef; 
	
	# the name of the kafka broker to send messages to. 
	# This is a comma-separated list of kafka brokers.
	# format: hostname1:port,hostname2:port
	# no spaces.
	const broker_name = "localhost" &redef;
	
	# this is the name to add to each message to identify
	# this sensor in the logs. It will be added to a "sensor"
	# field in the submitted json. 
	const sensor_name = "brosensor" &redef;
	
	# This is used internally by Kafka to trace requests and
	# errors. Kafka advises setting it to identify the 
	# application making the requests/produces.
	const client_id = "bro" &redef;
	
	# codec to use. 0=none, 1=gzip, 2=snappy.
	const compression_codec = "0" &redef; 
	
	# controls for how often to send messages up to 
	# kafka. Default is either every 500 messages,
	# every 10MB of buffer size, or every 10 seconds.
	# Any of these conditions hitting will cause 
	# the produce up to Kafka.
	# if more than 10,000 messages are queued up, 
	# the kafka producer will begin dropping messages.
	# max_batch_interval is the maximun number of milliseconds
	# that kafka will wait before sending logs.
	const default_batch_size = "500" &redef;
	const max_batch_size = "10000" &redef; 
	const max_batch_interval = "1000" &redef;
	

	# These are the names of the log files that will be sent in the 
	# json as the "type" field. These are also the keys used in the
	# column_rename variable above.
	const log_names = table(["AppStats::LOG"] = "app_stats",
							["Barnyard2::LOG"] = "barnyard2",
							["CaptureLoss::LOG"] = "capture_loss",
							["Cluster::LOG"] = "cluster",
							["Communication::LOG"] = "communication",
							["Conn::LOG"] = "conn",
							["DHCP::LOG"] = "dhcp",
							["DNP3::LOG"] = "dnps",
							["DNS::LOG"] = "dns",
							["DPD::LOG"] = "dpd",
							["Files::LOG"] = "files",
							["FTP::LOG"] = "ftp",
							["HTTP::LOG"] = "http",
							["Intel::LOG"] = "intel",
							["IRC::LOG"] = "irc",
							["KRB::LOG"] = "kerberos",
							["Known::CERTS_LOG"] = "known_certs",
							["Known::DEVICES_LOG"] = "known_devices",
							["Known::HOSTS_LOG"] = "known_hosts",
							["Known::MODBUS_LOG"] = "known_modbus",
							["Known::SERVICES_LOG"] = "known_services",
							["LoadedScripts::LOG"] = "loaded_scripts",
							["Modbus::LOG"] = "modbus",
							["Modbus::REGISTER_CHANGE_LOG"] = "modbus_register_change_log",
							["mysql::LOG"] = "mysql",
							["Notice::LOG"] = "notice",
							["Notice::ALARM_LOG"] = "notice_alarm",
							["PacketFilter::LOG"] = "packet_filter",
							["PE::LOG"] = "pe",
							["RADIUS::LOG"] = "radius",
							["RDP::LOG"] = "rdp",
							["Reporter::LOG"] = "reporter",
							["Signatures::LOG"] = "signatures",
							["SIP::LOG"] = "sip",
							["SMTP::LOG"] = "smtp",
							["SNMP::LOG"] = "snmp",
							["SOCKS::LOG"] = "socks",
							["Software::LOG"] = "software",
							["SSH::LOG"] = "ssh",
							["SSL::LOG"] = "ssl",
							["Stats::LOG"] = "stats",
							["Syslog::LOG"] = "syslog",
							["Traceroute::LOG"] = "traceroute",
							["Tunnel::LOG"] = "tunnel",
							["Unified2::LOG"] = "unified2",
							["Weird::LOG"] = "weird",
							["X509::LOG"] = "x509"
							) &redef;
	
}
