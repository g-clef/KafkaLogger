#include <librdkafka/rdkafkacpp.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "RoundRobinPartitioner.h"
#include <time.h>

using namespace RdKafka;


RoundRobinPartitionerCallback::RoundRobinPartitionerCallback(){
	srand(time(NULL));
}

int32_t RoundRobinPartitionerCallback::partitioner_cb(const RdKafka::Topic *topic,
													  const std::string *key,
													  int32_t partition_count,
													  void *msg_opaque) {
	// dead simple partitioner...simply chooses random partition
	return ((int32_t)rand()) % partition_count;
}

int32_t RoundRobinPartitionerCallback::partitioner_cb(const RdKafka::Topic *topic,
														const void *keydata,
														size_t keylen,
														int32_t partition_count,
														void *msg_opaque)
{
	return ((int32_t)rand()) % partition_count;
}

