#include "kyoshin_forecast.hpp"
#include "json_parser.hpp"

void KyoshinForecast::clear() {
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

void KyoshinForecast::update(const char *jsonString) {
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
