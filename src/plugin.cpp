// Copyright 2012 Las Venturas Playground. All rights reserved.
// Use of this source code is governed by the GPLv2 license, a copy of which can
// be found in the LICENSE file.

#include <vector>
#include <string>
#include <memory.h>
#include <algorithm>
#include <stdio.h>

#include "sdk/plugin.h"
#include "json.h"

#define CHECK_PARAMS_SAFE(count) \
	CHECK_PARAMS(count); \
	if (allocator == 0 || current_root == 0 || params[1] == 0 || allocator->contains(params[1]) == false) \
	    return 0;

logprintf_t logprintf;
const int AllocatorBlockSize = 8192;

block_allocator* allocator = 0;
json_value* current_root = 0;
char* file_contents = 0;

// native JSON_parse(filename[]);
static cell AMX_NATIVE_CALL n_json_parse(AMX* amx, cell* params) {
	CHECK_PARAMS(1);

  char* fileName = 0;
  if (file_contents != 0)
    delete file_contents;

	file_contents = 0;

	amx_StrParam(amx, params[1], fileName);
	if (fileName == 0 || strlen(fileName) == 0 || fileName[0] == '/' || fileName[0] == '.')
		return 0;

	FILE* file = fopen(fileName, "rb");
	if (file == 0) // invalid filename
		return 0;

	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize > 1024 * 1024 * 10) // 10 megabytes
		return 0;

	file_contents = (char*) malloc(fileSize + 1);
	fread(file_contents, 1, fileSize, file);
	file_contents[fileSize] = 0;

	char* errorPosition = 0;
	char* errorDescription = 0;
	int errorLine = 0;

	allocator = new block_allocator(AllocatorBlockSize);
	current_root = json_parse(file_contents, &errorPosition, &errorDescription, &errorLine, allocator);
	if (current_root == 0) {
		logprintf("  [LVP] JSON Reader: Unable to parse line %d of file %s.", errorLine, fileName);
		delete file_contents;
		delete allocator;

    file_contents = 0;
		allocator = 0;
		return 0;
	}

  if (errorDescription)
    logprintf("  [LVP] JSON Reader: %s (%s:%d).", errorDescription, errorLine);

	return reinterpret_cast<cell>(current_root);
}

// native JSON_close();
static cell AMX_NATIVE_CALL n_json_close(AMX* amx, cell* params) {
	CHECK_PARAMS(0);
	if (allocator != 0) {
		delete allocator;
		allocator = 0;
	}

  if (file_contents != 0) {
    delete file_contents;
    file_contents = 0;
  }

	if (current_root != 0) {
		current_root = 0;
	}

	return 1;
}

// native JSON_next(Node: node);
static cell AMX_NATIVE_CALL n_json_next(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(1);
	json_value* node = reinterpret_cast<json_value*>(params[1]);
	return reinterpret_cast<cell>(node->next_sibling);
}

// native JSON_find(Node: node, name[]);
static cell AMX_NATIVE_CALL n_json_find(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(2);
	json_value* node = reinterpret_cast<json_value*>(params[1]);

	char* nodeName = 0;
	amx_StrParam(amx, params[2], nodeName);
	if (nodeName == 0 || strlen(nodeName) == 0)
		return 0;

	json_value* child = node->first_child;
	while (child) {
		if (child->name == 0)
			continue;

		if (!strcmp(nodeName, child->name))
			return reinterpret_cast<cell>(child);

		child = child->next_sibling;
	}

	return 0;
}

// native JSON_firstChild(Node: node);
static cell AMX_NATIVE_CALL n_json_firstChild(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(1);
	json_value* node = reinterpret_cast<json_value*>(params[1]);
	return reinterpret_cast<cell>(node->first_child);
}

// native JSON_getName(Node: node, buffer[], bufferSize);
static cell AMX_NATIVE_CALL n_json_getName(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(3);
	json_value* node = reinterpret_cast<json_value*>(params[1]);

	cell* address = 0;
	int length = 0;
	amx_GetAddr(amx, params[2], &address);
	if (node->name == 0) {
		amx_SetString(address, "", 0, 0, params[3]);
	} else {
		amx_SetString(address, node->name, 0, 0, params[3]);
		length = strlen(node->name);
	}

	return length;
}

// native JSON_getType(Node: node);
static cell AMX_NATIVE_CALL n_json_getType(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(1);
	json_value* node = reinterpret_cast<json_value*>(params[1]);
	return static_cast<cell>(node->type);
}

// native JSON_readString(Node: node, buffer[], bufferSize);
static cell AMX_NATIVE_CALL n_json_readString(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(3);
	json_value* node = reinterpret_cast<json_value*>(params[1]);

	cell* address = 0;
	int length = 0;
	amx_GetAddr(amx, params[2], &address);
	if (node->type != JSON_STRING) {
		amx_SetString(address, "", 0, 0, params[3]);
	} else {
		amx_SetString(address, node->string_value, 0, 0, params[3]);
		length = strlen(node->string_value);
	}

	return length;
}

// native JSON_readInteger(Node: node, &value);
static cell AMX_NATIVE_CALL n_json_readInteger(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(2);
	json_value* node = reinterpret_cast<json_value*>(params[1]);

	cell* address = 0;
	amx_GetAddr(amx, params[2], &address);
	if (node->type == JSON_INT || node->type == JSON_BOOL) {
		*address = node->int_value;
		return 1;
	} else if (node->type == JSON_FLOAT) {
		*address = (int)(node->float_value);
		return 1;
	} else {
		*address = 0;
	}

	return 0;
}

// native JSON_readFloat(Node: node, &Float: value);
static cell AMX_NATIVE_CALL n_json_readFloat(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(2);
	json_value* node = reinterpret_cast<json_value*>(params[1]);

	cell* address = 0;
	float value = 0;
	amx_GetAddr(amx, params[2], &address);
	if (node->type == JSON_INT || node->type == JSON_BOOL) {
		value = (float) node->int_value;
		*address = amx_ftoc(value);
		return 1;
	} else if (node->type == JSON_FLOAT) {
		*address = amx_ftoc(node->float_value);
		return 1;
	} else {
		*address = amx_ftoc(value);
	}

	return 0;
}

// native JSON_readBoolean(Node: node, &bool: value);
static cell AMX_NATIVE_CALL n_json_readBoolean(AMX* amx, cell* params) {
	CHECK_PARAMS_SAFE(2);
	json_value* node = reinterpret_cast<json_value*>(params[1]);

	cell* address = 0;
	amx_GetAddr(amx, params[2], &address);
	if (node->type == JSON_INT || node->type == JSON_BOOL) {
		*address = node->int_value == 0 ? 0 : 1;
		return 1;
	} else if (node->type == JSON_FLOAT) {
		*address = node->float_value == 0 ? 0 : 1;
		return 1;
	} else {
		*address = 0;
	}

	return 0;
}


AMX_NATIVE_INFO pluginNativeFunctions[] = {
	{ "JSON_parse", n_json_parse },
	{ "JSON_next", n_json_next },
	{ "JSON_find", n_json_find },
	{ "JSON_firstChild", n_json_firstChild },
	{ "JSON_getName", n_json_getName },
	{ "JSON_getType", n_json_getType },
	{ "JSON_readString", n_json_readString },
	{ "JSON_readInteger", n_json_readInteger },
	{ "JSON_readFloat", n_json_readFloat },
	{ "JSON_readBoolean", n_json_readBoolean },
	{ "JSON_close", n_json_close },
	{ 0, 0 }
};

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("  [LVP] JSON Reader plugin loaded, using a block size of %d bytes.", AllocatorBlockSize);
	return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
	logprintf("  [LVP] JSON Reader plugin unloaded.");
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) {
	return amx_Register(amx, pluginNativeFunctions, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) {
	return AMX_ERR_NONE;
}
