// See the file "COPYING" in the main distribution directory for copyright.
//
// Log writer for writing to a Kafka broker
//
// This is experimental code that is not yet ready for production usage.
//
// check on these ifndef. Having this in both here and plugin.h causes
// compile problems.
//#ifndef BRO_PLUGIN_KAFKA_KAFKAWRITER
//#define BRO_PLUGIN_KAFKA_KAFKAWRITER

#include "logging/WriterBackend.h"
#include "logging/WriterFrontend.h"
#include "threading/formatters/JSON.h"
#include <librdkafka/rdkafkacpp.h>
#include "Type.h"
#include "AddingJson.h"
#include <string>

namespace logging { namespace writer {

class KafkaWriter : public WriterBackend {
public:
	KafkaWriter(WriterFrontend* frontend);
	~KafkaWriter();
	
	static string LogExt();
	static WriterBackend* Instantiate(WriterFrontend* frontend)
		{return new KafkaWriter(frontend); }
		

protected:

	virtual bool DoInit(const WriterInfo& info, int num_fields, const threading::Field* const* fields);
	virtual bool DoWrite(int num_fields, const threading::Field* const* fields, threading::Value** vals);
	virtual bool DoSetBuf(bool enabled);
	virtual bool DoRotate(const char* rotated_path, double open, double close, bool terminating);
	virtual bool DoFlush(double network_time);
	virtual bool DoFinish(double network_time);
	virtual bool DoHeartbeat(double network_time, double current_time);
	

private:

	threading::Field** MakeFields(const threading::Field* const* fields, int num_fields, std::string path);
	long transfer_timeout;
	
	// kafka configurations
	char* broker_name;
	int broker_name_len;
	char* topic_name;
	int topic_name_len;
	char* client_id;
	int client_id_len;
	char* compression_codec;
	int compression_codec_len;
	RdKafka::Conf *conf;
	RdKafka::Conf *tconf;
	RdKafka::Producer *producer;
	RdKafka::Topic *topic;
	uint64 batch_size;


	//varible to hold renamed fields
	threading::Field** fixed_fields;
	
	// formatter to turn fields/values into json.
	threading::formatter::AddingJSON* json_formatter;
	
};

}
}

//#endif
