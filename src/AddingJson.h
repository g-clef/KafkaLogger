// See the file "COPYING" in the main distribution directory for copyright.
#ifndef THREADING_FORMATTERS_ADDINGJSON_H
#define THREADING_FORMATTERS_ADDINGJSON_H


#include "threading/Formatter.h"
#include "threading/SerialTypes.h"

/**
  * A thread-safe class for converting values into a JSON representation
  * and vice versa.
  *
  * Modified to add fields to the JSON stream, depending on user-supplied
  * criteria.
  */


namespace threading { namespace formatter {

class AddingJSON : public Formatter {
	public:
		enum TimeFormat {
			TS_EPOCH,	// Doubles that represents seconds from the UNIX epoch.
			TS_ISO8601,	// ISO 8601 defined human readable timestamp format.
			TS_MILLIS	// Milliseconds from the UNIX epoch.  Some consumers need this (e.g., elasticsearch).
			};

		AddingJSON(threading::MsgThread* t, TimeFormat tf, char* argsensor_name, char* argtype_name, bool logstash_format_timestamps);
		~AddingJSON();

		bool AddingDescribe(ODesc* desc, threading::Value* val, const string& name="") const;
		bool Describe(ODesc* desc, threading::Value* val, const string& name = "") const;
		bool Describe(ODesc* desc, int num_fields, const threading::Field* const * fields,
	                      threading::Value** vals) const;
		threading::Value* ParseValue(const string& s, const string& name, TypeTag type, TypeTag subtype = TYPE_ERROR) const;

		void SurroundingBraces(bool use_braces);

	private:
		TimeFormat timestamps;
		bool surrounding_braces;
		const char* sensor_name;
		const char* type_name;
		bool logstash_timestamps;
};

}}

#endif
