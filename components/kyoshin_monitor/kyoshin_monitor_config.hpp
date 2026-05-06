#pragma once
#include <cstdint>

#define KYOSHIN_DEF_ENUM(name, ...)                                                   \
    struct name {                                                                     \
        enum Value { __VA_ARGS__, count };                                            \
        Value value;                                                                  \
        constexpr name() : value(static_cast<Value>(0)) {}                            \
        constexpr name(Value value) : value(value) {}                                 \
        constexpr name(int value) : value(static_cast<Value>(value)) {}               \
        name next() { return name(static_cast<Value>((value + 1) % count)); }         \
        name prev() { return name(static_cast<Value>((value + count - 1) % count)); } \
        bool operator==(const name &other) const { return value == other.value; }     \
        bool operator!=(const name &other) const { return value != other.value; }     \
    }

KYOSHIN_DEF_ENUM(MapRegion, Japan, Noto);
KYOSHIN_DEF_ENUM(RealtimeImgType, RealtimeShindo, PGA, PGV, PGD, RSP0125, RSP0250, RSP0500, RSP1000, RSP2000, RSP4000);

struct MapRegionConfig {
    const char *identifier;
    const char *baseMapUrl;
    const char *surfaceUrlFormats[RealtimeImgType::count];
    const char *boreholeUrlFormats[RealtimeImgType::count];
    const char *psWaveUrlFormat;
};

struct ServerConfig {
    const char *base_url;
    int imgWidth;
    int imgHeight;
    MapRegionConfig regions[MapRegion::count];
    const char *forecastUrlFormat;
    const char *latestUrl;
};

inline constexpr ServerConfig KYOSHIN_SERVER_CONFIG = {
    .base_url = "http://www.kmoni.bosai.go.jp",
    // .base_url = "http://192.168.0.103:8080",
    .imgWidth = 352,
    .imgHeight = 400,
    .regions = {
        {
            .identifier = "japan",
            .baseMapUrl = "/data/map_img/CommonImg/base_map_w.gif",
            .surfaceUrlFormats = {
                "/data/map_img/RealTimeImg/jma_s/%Y%m%d/%Y%m%d%H%M%S.jma_s.gif",
                "/data/map_img/RealTimeImg/acmap_s/%Y%m%d/%Y%m%d%H%M%S.acmap_s.gif",
                "/data/map_img/RealTimeImg/vcmap_s/%Y%m%d/%Y%m%d%H%M%S.vcmap_s.gif",
                "/data/map_img/RealTimeImg/dcmap_s/%Y%m%d/%Y%m%d%H%M%S.dcmap_s.gif",
                "/data/map_img/RealTimeImg/rsp0125_s/%Y%m%d/%Y%m%d%H%M%S.rsp0125_s.gif",
                "/data/map_img/RealTimeImg/rsp0250_s/%Y%m%d/%Y%m%d%H%M%S.rsp0250_s.gif",
                "/data/map_img/RealTimeImg/rsp0500_s/%Y%m%d/%Y%m%d%H%M%S.rsp0500_s.gif",
                "/data/map_img/RealTimeImg/rsp1000_s/%Y%m%d/%Y%m%d%H%M%S.rsp1000_s.gif",
                "/data/map_img/RealTimeImg/rsp2000_s/%Y%m%d/%Y%m%d%H%M%S.rsp2000_s.gif",
                "/data/map_img/RealTimeImg/rsp4000_s/%Y%m%d/%Y%m%d%H%M%S.rsp4000_s.gif",
            },
            .boreholeUrlFormats = {
                "/data/map_img/RealTimeImg/jma_b/%Y%m%d/%Y%m%d%H%M%S.jma_b.gif",
                "/data/map_img/RealTimeImg/acmap_b/%Y%m%d/%Y%m%d%H%M%S.acmap_b.gif",
                "/data/map_img/RealTimeImg/vcmap_b/%Y%m%d/%Y%m%d%H%M%S.vcmap_b.gif",
                "/data/map_img/RealTimeImg/dcmap_b/%Y%m%d/%Y%m%d%H%M%S.dcmap_b.gif",
                "/data/map_img/RealTimeImg/rsp0125_b/%Y%m%d/%Y%m%d%H%M%S.rsp0125_b.gif",
                "/data/map_img/RealTimeImg/rsp0250_b/%Y%m%d/%Y%m%d%H%M%S.rsp0250_b.gif",
                "/data/map_img/RealTimeImg/rsp0500_b/%Y%m%d/%Y%m%d%H%M%S.rsp0500_b.gif",
                "/data/map_img/RealTimeImg/rsp1000_b/%Y%m%d/%Y%m%d%H%M%S.rsp1000_b.gif",
                "/data/map_img/RealTimeImg/rsp2000_b/%Y%m%d/%Y%m%d%H%M%S.rsp2000_b.gif",
                "/data/map_img/RealTimeImg/rsp4000_b/%Y%m%d/%Y%m%d%H%M%S.rsp4000_b.gif",
            },
            .psWaveUrlFormat = "/data/map_img/PSWaveImg/eew/%Y%m%d/%Y%m%d%H%M%S.eew.gif",
        },
        {
            .identifier = "noto",
            .baseMapUrl = "/data/map_img/CommonImg_noto/base_map_w.gif",
            .surfaceUrlFormats = {
                "/data/map_img/RealTimeImg_noto/jma_s/%Y%m%d/%Y%m%d%H%M%S.jma_s.gif",
                "/data/map_img/RealTimeImg_noto/acmap_s/%Y%m%d/%Y%m%d%H%M%S.acmap_s.gif",
                "/data/map_img/RealTimeImg_noto/vcmap_s/%Y%m%d/%Y%m%d%H%M%S.vcmap_s.gif",
                "/data/map_img/RealTimeImg_noto/dcmap_s/%Y%m%d/%Y%m%d%H%M%S.dcmap_s.gif",
                "/data/map_img/RealTimeImg_noto/rsp0125_s/%Y%m%d/%Y%m%d%H%M%S.rsp0125_s.gif",
                "/data/map_img/RealTimeImg_noto/rsp0250_s/%Y%m%d/%Y%m%d%H%M%S.rsp0250_s.gif",
                "/data/map_img/RealTimeImg_noto/rsp0500_s/%Y%m%d/%Y%m%d%H%M%S.rsp0500_s.gif",
                "/data/map_img/RealTimeImg_noto/rsp1000_s/%Y%m%d/%Y%m%d%H%M%S.rsp1000_s.gif",
                "/data/map_img/RealTimeImg_noto/rsp2000_s/%Y%m%d/%Y%m%d%H%M%S.rsp2000_s.gif",
                "/data/map_img/RealTimeImg_noto/rsp4000_s/%Y%m%d/%Y%m%d%H%M%S.rsp4000_s.gif",
            },
            .boreholeUrlFormats = {
                "/data/map_img/RealTimeImg_noto/jma_b/%Y%m%d/%Y%m%d%H%M%S.jma_b.gif",
                "/data/map_img/RealTimeImg_noto/acmap_b/%Y%m%d/%Y%m%d%H%M%S.acmap_b.gif",
                "/data/map_img/RealTimeImg_noto/vcmap_b/%Y%m%d/%Y%m%d%H%M%S.vcmap_b.gif",
                "/data/map_img/RealTimeImg_noto/dcmap_b/%Y%m%d/%Y%m%d%H%M%S.dcmap_b.gif",
                "/data/map_img/RealTimeImg_noto/rsp0125_b/%Y%m%d/%Y%m%d%H%M%S.rsp0125_b.gif",
                "/data/map_img/RealTimeImg_noto/rsp0250_b/%Y%m%d/%Y%m%d%H%M%S.rsp0250_b.gif",
                "/data/map_img/RealTimeImg_noto/rsp0500_b/%Y%m%d/%Y%m%d%H%M%S.rsp0500_b.gif",
                "/data/map_img/RealTimeImg_noto/rsp1000_b/%Y%m%d/%Y%m%d%H%M%S.rsp1000_b.gif",
                "/data/map_img/RealTimeImg_noto/rsp2000_b/%Y%m%d/%Y%m%d%H%M%S.rsp2000_b.gif",
                "/data/map_img/RealTimeImg_noto/rsp4000_b/%Y%m%d/%Y%m%d%H%M%S.rsp4000_b.gif",
            },
            .psWaveUrlFormat = "/data/map_img/PSWaveImg_noto/eew/%Y%m%d/%Y%m%d%H%M%S.eew.gif",
        },
    },
    .forecastUrlFormat = "/webservice/hypo/eew/%Y%m%d%H%M%S.json",
    .latestUrl = "/webservice/server/pros/latest.json",
};
