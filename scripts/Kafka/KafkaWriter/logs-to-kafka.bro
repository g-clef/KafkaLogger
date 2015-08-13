module KafkaLogger;

export {
	# redefine this in your script to identify the logs
	# that should be sent up to bro. 
	# for example: 
	#
	# redef KafkaLogger::logs_to_send = set(HTTP::LOG, Conn::Log, DNS::LOG);
	#
	# that will send the HTTP, Conn, and DNS logs up to Kafka.
	#
	const logs_to_send: set[Log::ID] &redef;
}

event bro_init() &priority=-5
{
	
	for (stream_id in Log::active_streams)
	{
	    if (stream_id !in logs_to_send){
	        next;
	    }
	    # note: the filter name is different for each log, to cause Bro to instantiate
	    # a new Writer instance for each log. The bro folks might want me
	    # to do this with a single writer instance, but the mechanics of
	    # modifying the field names and adding fields made it quite complicated,
	    # so I opted to make one log writer per bro log going to Kafka.
	    # this means there will be multiple connections from bro to the kafka
	    # server, one per log file. 
	    local streamString = fmt("%s", stream_id);
	    local pathname = fmt("%s", KafkaLogger::log_names[streamString]);
	    local filter: Log::Filter = [$name = fmt("kafka-%s",stream_id),
	    							 $writer = Log::WRITER_KAFKAWRITER,
	    							 $path = pathname
	    							];
	    Log::add_filter(stream_id, filter);
	
	}

}
	