
#include "Plugin.h"
#include "Kafka.h"

namespace plugin { namespace Kafka_KafkaWriter { Plugin plugin; } }

using namespace plugin::Kafka_KafkaWriter;

plugin::Configuration Plugin::Configure()
	{
	AddComponent(new ::logging::Component("KafkaWriter", ::logging::writer::KafkaWriter::Instantiate));
	
	plugin::Configuration config;
	config.name = "Kafka::KafkaWriter";
	config.description = "Kafka Json writer.";
	config.version.major = 0;
	config.version.minor = 1;
	return config;
	}
