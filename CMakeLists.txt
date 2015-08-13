set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

cmake_minimum_required(VERSION 2.8)

project(Plugin)

include(BroPlugin)

find_package(LibRDKafka)

if ( LIBRDKAFKA_FOUND )
	include_directories(BEFORE ${LibRDKafka_INCLUDE_DIR})

	bro_plugin_begin(Kafka KafkaWriter)
	bro_plugin_cc(src/AddingJson.cc)
	bro_plugin_cc(src/Kafka.cc)
	bro_plugin_cc(src/Plugin.cc)
	bro_plugin_bif(src/kafkawriter.bif)
	bro_plugin_dist_files(README CHANGES COPYING VERSION)
	bro_plugin_link_library(${LibRDKafka_LIBRARIES})
	bro_plugin_link_library(${LibRDKafka_C_LIBRARIES})
	bro_plugin_end()
	message(STATUS "LibRDKafka prefix	: ${LibRDKafka_ROOT_DIR}")
	message(STATUS "LibRDKafka Library	: ${LibRDKafka_LIBRARIES}")
else ()
	message(FATAL_ERROR "LibRDKafka not found.")
endif ()
