
#ifndef BRO_PLUGIN_KAFKA_KAFKAWRITER
#define BRO_PLUGIN_KAFKA_KAFKAWRITER

#include <plugin/Plugin.h>

namespace plugin {
	namespace Kafka_KafkaWriter {

		class Plugin : public ::plugin::Plugin
			{
			protected:
				// Overridden from plugin::Plugin.
				virtual plugin::Configuration Configure();
			};

		extern Plugin plugin;

	}
}

#endif
