#include <librdkafka/rdkafkacpp.h>

class RoundRobinPartitionerCallback : public RdKafka::PartitionerCb {
	//private:
	//	int32_t partition_num;


	public:
		int32_t partitioner_cb(const RdKafka::Topic *topic,
								const std::string *key,
								int32_t partition_count,
								void *msg_opaque);

		int32_t partitioner_cb(const RdKafka::Topic *topic,
								const void *keydata,
								size_t keylen,
								int32_t partition_cnt,
								void *msg_opaque);


		RoundRobinPartitionerCallback();
};
