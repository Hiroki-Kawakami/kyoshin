#pragma once
#include <string>
#include "kyoshin_monitor_config.hpp"

KYOSHIN_DEF_ENUM(KyoshinForecastType, Normal, Alert);

struct KyoshinForecast {
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

    void clear();
    void update(const char *jsonString);

    bool empty() const {
        return reportId.empty();
    }

    uint32_t color() const {
        if (empty() || isCancel) return 0xd3d3d3;
        if (isTraining) return 0x0000ff;
        if (alertflg == "警報") return 0xff0000;
        return 0xff7800;
    }

    KyoshinForecastType type() const {
        if (alertflg == "警報") return KyoshinForecastType::Alert;
        return KyoshinForecastType::Normal;
    }

    std::string reportNumString() const {
        if (isCancel) return "キャンセル報";
        if (isTraining) return "訓練報";
        if (isFinal) return "最終報";
        return "第" + reportNum + "報";
    }

    bool isStarted() const {
        if (reportTime == prevReportTime) return false;
        return prevReportTime.empty();
    }
    bool isUpdated() const {
        if (reportTime == prevReportTime) return false;
        return !reportTime.empty();
    }
    void updateReportTime() {
        prevReportTime = reportTime;
    }
};
