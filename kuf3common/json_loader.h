#pragma once

#include "rapidjson/document.h"

class json_loader
{
public:
	json_loader() {};
	virtual ~json_loader() {};

	virtual bool load() = 0;
	rapidjson::Document file_load(const char* szFileName);
};