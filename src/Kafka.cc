#include "config.h"

#include "util.h"
#include <errno.h>
#include <string>
#include <cstdlib>
#include <time.h>
#include <librdkafka/rdkafkacpp.h>
#include "BroString.h"
#include "threading/SerialTypes.h"
#include "Kafka.h"
#include "kafkawriter.bif.h"

using namespace logging;
using namespace writer;
using threading::Value;
using threading::Field;
using namespace RdKafka;


class RandomPartitionerCallback : public RdKafka::PartitionerCb {
private:
	unsigned int seed;

public:
	int32_t partitioner_cb(const RdKafka::Topic *topic, const std::string *key,
							int32_t partition_count, void *msg_opaque)
	{
		return (int32_t)rand_r(&seed) % partition_count;
	}

	RandomPartitionerCallback(){
		seed = time(NULL);
	}
};


KafkaWriter::KafkaWriter(WriterFrontend* frontend) : WriterBackend(frontend)
{

	json_formatter = 0;
	//srand(time(NULL));
	producer = NULL;
	topic = NULL;


	// initialize kafka variables...
	broker_name_len = BifConst::KafkaLogger::broker_name->Len();
	broker_name = new char[broker_name_len + 1];
	memcpy(broker_name, BifConst::KafkaLogger::broker_name->Bytes(), broker_name_len);
	broker_name[broker_name_len] = 0;

    topic_name_len = BifConst::KafkaLogger::topic_name->Len();
    topic_name = new char[topic_name_len + 1];
    memcpy(topic_name, BifConst::KafkaLogger::topic_name->Bytes(), topic_name_len);
    topic_name[topic_name_len] = 0;

    compression_codec_len = BifConst::KafkaLogger::compression_codec->Len();
    compression_codec = new char[compression_codec_len + 1];
    memcpy(compression_codec, BifConst::KafkaLogger::compression_codec->Bytes(), compression_codec_len);
    compression_codec[compression_codec_len] = 0;

    // initialize varibles used to store extra data appended to every message
    // (sensor name and log type)
    int sensor_name_len = BifConst::KafkaLogger::sensor_name->Len();
    char* sensor_name = new char[sensor_name_len + 1];
    memcpy(sensor_name, BifConst::KafkaLogger::sensor_name->Bytes(), sensor_name_len);
    sensor_name[sensor_name_len] = 0;

    security_protocol_len = BifConst::KafkaLogger::security_protocol->Len();
    security_protocol = new char[security_protocol_len + 1];
    memcpy(security_protocol, BifConst::KafkaLogger::security_protocol->Bytes(), security_protocol_len);
    security_protocol[security_protocol_len] = 0;

    kerberos_service_name_len = BifConst::KafkaLogger::kerberos_service_name->Len();
    kerberos_service_name = new char[kerberos_service_name_len + 1];
    memcpy(kerberos_service_name, BifConst::KafkaLogger::kerberos_service_name->Bytes(), kerberos_service_name_len);
    kerberos_service_name[kerberos_service_name_len] = 0;

    kerberos_keytab_len = BifConst::KafkaLogger::kerberos_keytab->Len();
    kerberos_keytab = new char[kerberos_keytab_len + 1];
    memcpy(kerberos_keytab, BifConst::KafkaLogger::kerberos_keytab->Bytes(), kerberos_keytab_len);
    kerberos_keytab[kerberos_keytab_len] = 0;

    kerberos_principal_len = BifConst::KafkaLogger::kerberos_principal->Len();
    kerberos_principal = new char[kerberos_principal_len + 1];
    memcpy(kerberos_principal, BifConst::KafkaLogger::kerberos_principal->Bytes(), kerberos_principal_len);
    kerberos_principal[kerberos_principal_len] = 0;

    int type_name_len = strlen(Info().path);
    char* type_name = new char[type_name_len + 1];
    memcpy(type_name, Info().path, type_name_len);
    type_name[type_name_len] = 0;

    client_id_len = BifConst::KafkaLogger::client_id->Len() + strlen(Info().path) + 1;
    client_id = new char[client_id_len + 1];
    memcpy(client_id, BifConst::KafkaLogger::client_id->Bytes(), client_id_len);
    strcat(client_id, "-");
    strcat(client_id, type_name);
    client_id[client_id_len] = 0;

    json_formatter = new threading::formatter::AddingJSON(this,
    								threading::formatter::AddingJSON::TS_MILLIS,
    								sensor_name,
    								type_name,
    								BifConst::KafkaLogger::logstash_style_timestamp
    							   );
}

KafkaWriter::~KafkaWriter()
{
    delete [] broker_name;
    delete [] topic_name;
    delete [] client_id;
    delete [] compression_codec;
    delete [] fixed_fields;
    // I think I need to shut down the connection to the producer before deleting
    // these variables. Also, if I just blindly delete these two, bro segfaults when
    // shutting down. Confess I don't understand what's happening here.
    //delete producer;
    //delete topic;
    delete json_formatter;
}


threading::Field** KafkaWriter::MakeFields(const threading::Field* const* fields, int num_fields, std::string path){
	// create the renamed fields, based on user-supplied config.
	threading::Field** newFields = (threading::Field**)malloc(sizeof(threading::Field*) * (num_fields));

	// what I'd like to do is
	// first, grab the rename table for just this log
	// The config will have a table of table of strings.
	// the first table key is the name of the log (dns, http, etc)
	// the internal table key is the column name, the internal table value
	// will be the name to change that to.
	// loop over the existing fields, look up the field name in the rename table.
	// if it exists, create a new field entry with the new name, otherwise,
	// copy the existing field name in to the new field list.
	//
	// However, I can't get the bro TableVar Lookup to return anything
	// even for tables that I know have data in them. I'm clearly doing
	// something wrong. So,  hardcode the renames in the interest of
	// getting something done.
	//
	// Also, need to remove "."s from names for ElasticSearch.
	//
	for (int i = 0; i < num_fields; i++){
		std::string newName;

		if (strcmp(fields[i]->name, "ts") == 0)
		{
			newName = "timestamp";
		}
		else if (strcmp(fields[i]->name, "id.orig_h") == 0)
		{
			newName = "source_ip";
		}
		else if (strcmp(fields[i]->name, "id.orig_p") == 0)
		{
			newName = "source_port";
		}
		else if (strcmp(fields[i]->name, "id.resp_h") == 0)
		{
			newName = "dest_ip";
		}
		else if (strcmp(fields[i]->name,"id.resp_p") == 0)
		{
			newName = "dest_port";
		}
		else if (strcmp(fields[i]->name, "seen.indicator") == 0)
		{
			newName = "indicator";
		}
		else if (strcmp(fields[i]->name, "seen.indicator_type") == 0)
		{
			newName = "indicator_type";
		}
		else if (strcmp(fields[i]->name, "seen.where") == 0)
		{
			newName = "seen_where";
		}
		else if (strcmp(fields[i]->name, "seen.node") == 0)
		{
			newName = "seen_node";
		}
		else if (strcmp(fields[i]->name, "data_channel.orig_h") == 0) //ftp
		{
			newName = "data_channel_orig_h";
		}
		else if (strcmp(fields[i]->name, "data_channel.passive") == 0)
		{
			newName = "data_channel_passive";
		}
		else if (strcmp(fields[i]->name, "data_channel.resp_h") == 0)
		{
			newName = "data_channel_resp_h";
		}
		else if (strcmp(fields[i]->name, "data_channel.resp_p") == 0)
		{
			newName = "data_channel_resp_p";
		}
		else if (strcmp(fields[i]->name, "bound.host") == 0) //socks
		{
			newName = "bound_host";
		}
		else if (strcmp(fields[i]->name, "bounds_host") == 0)
		{
			newName = "bounds_host";
		}
		else if (strcmp(fields[i]->name, "bound.name") == 0)
		{
			newName = "bound_name";
		}

		if (newName.empty()){
			newFields[i] = new threading::Field(fields[i]->name,
												fields[i]->secondary_name,
												fields[i]->type,
												fields[i]->subtype,
												true);
		}
		else {
			newFields[i]= new threading::Field(newName.c_str(),
												fields[i]->secondary_name,
												fields[i]->type,
												fields[i]->subtype,
												true);
		}

	}

	return newFields;
}

bool KafkaWriter::DoInit(const WriterInfo& info, int num_fields, const threading::Field* const* fields)
{
	conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
	// note: hard-coding to use the random partitioner right now...
	RandomPartitionerCallback* part_cb = new RandomPartitionerCallback();
	std::string errstr;

	// set up kafka connection (get brokers, set partitioner, etc)
	if (conf->set("metadata.broker.list", broker_name, errstr) != RdKafka::Conf::CONF_OK){
		reporter->Error("Failed to set metatdata.broker.list: %s", errstr.c_str());
		return false;
	}
    int default_batch_size_len = BifConst::KafkaLogger::default_batch_size->Len();
    char* default_batch_size = new char[default_batch_size_len + 1];
    memcpy(default_batch_size, BifConst::KafkaLogger::default_batch_size->Bytes(), default_batch_size_len);
    default_batch_size[default_batch_size_len] = 0;

    int max_batch_size_len = BifConst::KafkaLogger::max_batch_size->Len();
    char* max_batch_size = new char[max_batch_size_len + 1];
    memcpy(max_batch_size, BifConst::KafkaLogger::max_batch_size->Bytes(), max_batch_size_len);
    max_batch_size[max_batch_size_len] = 0;

    int max_batch_interval_len = BifConst::KafkaLogger::max_batch_interval->Len();
    char* max_batch_interval = new char[max_batch_interval_len + 1];
    memcpy(max_batch_interval, BifConst::KafkaLogger::max_batch_interval->Bytes(), max_batch_interval_len);
    max_batch_interval[max_batch_interval_len] = 0;

	conf->set("compression.codec", compression_codec, errstr);
	conf->set("client.id", client_id, errstr);
	conf->set("batch.num.messages", default_batch_size, errstr);
	conf->set("queue.buffering.max.messages", max_batch_size, errstr);
	conf->set("queue.buffering.max.ms", max_batch_interval, errstr);
	conf->set("producer.type", "async", errstr);

	if (strcmp("SASL_SSL", security_protocol) == 0) {
		//if ssl is supported without kerberos then this will have 
		//  to change to enable SSL but I'm not sure it make sense
		conf->set("security.protocol", security_protocol, errstr);
	}
	
	if ( (strcmp("SASL_SSL", security_protocol) ==0 ) || (strcmp("SASL_PLAINTEXT", security_protocol) ==0 ) ){
		//SASL is enabled and we need to setup the kerberos options.

		conf->set("sasl.kerberos.service.name", kerberos_service_name, errstr);
		conf->set("sasl.kerberos.keytab", kerberos_keytab, errstr);
		conf->set("sasl.kerberos.principal", kerberos_principal, errstr);
	}

	if (tconf->set("partitioner_cb",  part_cb, errstr) != RdKafka::Conf::CONF_OK){
		reporter->Error("failed to set partitioner for Kafka. %s", errstr.c_str());
	}

	producer = RdKafka::Producer::create(conf, errstr);
	if (!producer) {
		reporter->Error("Failed to create producer");
		return false;
	}
	topic = RdKafka::Topic::create(producer, topic_name, tconf, errstr);

	if (!topic) {
		reporter->Error("Failed to create topic.");
		return false;
	}

	// set up lookups and renamed fields.
	fixed_fields = MakeFields(fields, num_fields, Info().path);

	return true;
}


bool KafkaWriter::DoWrite(int num_fields, const Field* const * fields, Value** vals)
    {
	ODesc buffer;

    // this may look silly, but as of this writing, Kafka's default
    // partitioning is poor. if you do not supply a key, kafka will never
    // call your partition function, even if one is specified in the config.
    // What it will do instead is choose a partition at random when it starts
    // up, and send everything to that partition. So, you need to supply a
    // partition key if you want your partitioner to be used
    const std::string partition_key = "this is a key to trigger partitioning.";


	buffer.Clear();

    json_formatter->Describe(&buffer, num_fields, fixed_fields, vals);
    const char* bytes = (const char*)buffer.Bytes();
    std::string errstr;

	// actually send the data to Kafka.
    RdKafka::ErrorCode resp = producer->produce(topic,
    											RdKafka::Topic::PARTITION_UA,
    											RdKafka::Producer::MSG_COPY /* Copy payload */,
    											const_cast<char *>(bytes),
    											strlen(bytes),
    											&partition_key,
    											NULL);
    if (resp != RdKafka::ERR_NO_ERROR) {
        errstr = RdKafka::err2str(resp);
        reporter->Error("Produce failed: %s", errstr.c_str());
        reporter->Error("failed line: %s", bytes);
    }

    // Note: this bit here means that even if the send to kafka fails, we're just going to
    // drop the messages. Such is life.
    producer->poll(0);

    return true;
    }



bool KafkaWriter::DoSetBuf(bool enabled)
    {
    // Nothing to do.
    return true;
    }

bool KafkaWriter::DoFlush(double network_time)
    {
    // Nothing to do.
    return true;
    }

bool KafkaWriter::DoFinish(double network_time)
    {
    RdKafka::wait_destroyed(5000);
    return true;
    }

bool KafkaWriter::DoHeartbeat(double network_time, double current_time)
    {
	//nothing to do...all timing handled inside Kafka.
    return true;
    }

bool KafkaWriter::DoRotate(const char* rotated_path, double open, double close, bool terminating)
    {
    // Nothing to do.
    FinishedRotation();
    return true;
    }
