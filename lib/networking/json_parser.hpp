/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once

#include <string>
#include "cJSON.h"

struct JSONValue {
    cJSON *value;
    bool shouldDelete;
    JSONValue(cJSON *value) : value(value), shouldDelete(false) {}
    JSONValue(std::string value) : value(cJSON_Parse(value.c_str())), shouldDelete(true) {}
    ~JSONValue() { if (shouldDelete) cJSON_Delete(value); }

    bool isNull() { return !value || cJSON_IsNull(value); }

    bool isBool() { return cJSON_IsBool(value); }
    bool boolValue() { return cJSON_IsTrue(value); }

    bool isNumber() { return cJSON_IsNumber(value); }
    int intValue() { return isNumber() ? value->valueint : 0; }
    double doubleValue() { return isNumber() ? value->valuedouble : 0; }

    bool isString() { return cJSON_IsString(value); }
    std::string stringValue() { return isString() ? value->valuestring : ""; }

    bool isArray() { return cJSON_IsArray(value); }
    JSONValue operator[](int index) {
        if (!isArray()) return JSONValue(nullptr);
        return JSONValue(cJSON_GetArrayItem(value, index));
    }

    bool isObject() { return cJSON_IsObject(value); }
    JSONValue operator[](std::string key) {
        if (!isObject() || !cJSON_HasObjectItem(value, key.c_str())) return JSONValue(nullptr);
        return JSONValue(cJSON_GetObjectItem(value, key.c_str()));
    }

    int size() {
        if (isArray()) return cJSON_GetArraySize(value);
        return 0;
    }
};
