#pragma once

#include <stdexcept>
#include <json/value.h>
#include <json/json.h>
#include <fstream>

namespace util::json {
	Json::Value read_config(const char *filename, std::string config_name="") {
		std::ifstream rawJson(filename, std::ifstream::binary);

		Json::Value parsedJson;

		Json::CharReaderBuilder builder;
		builder["collectComments"] = true;

	    JSONCPP_STRING err;
    	if (!Json::parseFromStream(builder, rawJson, &parsedJson, &err)) {
    		std::cout << "error: " << err << std::endl;
    		throw std::runtime_error(err);
    		return parsedJson;
    	}

		if (config_name.empty()) {
			return parsedJson;
		} else {
			return parsedJson[config_name.c_str()];
		}
	}
}