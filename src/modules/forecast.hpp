/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <string>
#include "M5Unified.h"
#include "json_parser.hpp"

struct Forecast {
    std::string reportId;
    std::string reportTime;
    std::string reportNum;
    std::string alertflg;
    std::string calcintensity;
    std::string magnitude;
    std::string depth;
    std::string regionName;
    bool isCancel;
    bool isFinal;
    bool isTraining;

    std::string prevReportTime = "";

    void clear() {
        reportId.clear();
        reportTime.clear();
        alertflg.clear();
        calcintensity.clear();
        magnitude.clear();
        depth.clear();
        regionName.clear();
        isCancel = false;
        isFinal = false;
        isTraining = false;
        prevReportTime.clear();
    }

    void update(const char *jsonString) {
        auto json = JSONValue(jsonString);
        if (json["result"]["status"].stringValue() != "success") return;
        reportId = json["report_id"].stringValue();
        reportTime = json["report_time"].stringValue();
        reportNum = json["report_num"].stringValue();
        alertflg = json["alertflg"].stringValue();
        calcintensity = json["calcintensity"].stringValue();
        magnitude = json["magunitude"].stringValue();
        depth = json["depth"].stringValue();
        regionName = json["region_name"].stringValue();
        isCancel = json["is_cancel"].boolValue();
        isFinal = json["is_final"].boolValue();
        isTraining = json["is_training"].boolValue();
    }

    bool empty() const {
        return reportId.empty();
    }

    uint32_t color() const {
        if (empty() || isCancel) return lgfx::color888(211, 211, 211);
        if (isTraining) return lgfx::color888(0, 0, 255);
        if (alertflg == "警報") return lgfx::color888(255, 0, 0);
        return lgfx::color888(255, 124, 0);
    }

    ForecastType type() const {
        if (alertflg == "警報") return ForecastType::Alert;
        return ForecastType::Normal;
    }

    std::string reportNumString() {
        if (isCancel) return "キャンセル報";
        if (isTraining) return "訓練報";
        if (isFinal) return "最終報";
        return "第" + reportNum + "報";
    }

    bool isStarted() {
        if (reportTime == prevReportTime) return false;
        return prevReportTime.empty();
    }
    bool isUpdated() {
        if (reportTime == prevReportTime) return false;
        return !reportTime.empty();
    }
    void updateReportTime() {
        prevReportTime = reportTime;
    }
};
