/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "config_internal.hpp"

DEF_CONFIG_ENUM(MapRegion, Japan, Noto);
DEF_CONFIG_ENUM(RealtimeImgType, RealtimeShindo, PGA, PGV, PGD, RSP0125, RSP0250, RSP0500, RSP1000, RSP2000, RSP4000);
inline const char *displayString(RealtimeImgType value) {
    switch (value.value) {
    case RealtimeImgType::RealtimeShindo: return "リアルタイム震度";
    case RealtimeImgType::PGA           : return "最大加速度";
    case RealtimeImgType::PGV           : return "最大速度";
    case RealtimeImgType::PGD           : return "最大変位";
    case RealtimeImgType::RSP0125       : return "0.125Hz速度応答";
    case RealtimeImgType::RSP0250       : return "0.25Hz速度応答";
    case RealtimeImgType::RSP0500       : return "0.5Hz速度応答";
    case RealtimeImgType::RSP1000       : return "1.0Hz速度応答";
    case RealtimeImgType::RSP2000       : return "2.0Hz速度応答";
    case RealtimeImgType::RSP4000       : return "4.0Hz速度応答";
    default                             : return "Unknown";
    }
}

struct MapRegionConfig {
    const char *identifier;
    const char *baseMapUrl;
    const char *surfaceUrlFormats[RealtimeImgType::count];
    const char *boreholeUrlFormats[RealtimeImgType::count];
    const char *psWaveUrlFormat;
};

struct ServerConfig {
    int imgWidth;
    int imgHeight;
    int timeOffset;
    int updateInterval;
    int idleUpdateInterval;
    MapRegionConfig regions[MapRegion::count];
    const char *forecastUrlFormat;
};

#define SERVER "http://www.kmoni.bosai.go.jp"
// #define SERVER "http://192.168.1.86:8000"
inline constexpr ServerConfig SERVER_CONFIG = {
    .imgWidth = 352,
    .imgHeight = 400,
    .timeOffset = -2000, // msec
    .updateInterval = 1, // sec
    .idleUpdateInterval = 10, // sec
    .regions = {
        {
            .identifier = "japan",
            .baseMapUrl = SERVER "/data/map_img/CommonImg/base_map_w.gif",
            .surfaceUrlFormats = {
                SERVER "/data/map_img/RealTimeImg/jma_s/%Y%m%d/%Y%m%d%H%M%S.jma_s.gif",
                SERVER "/data/map_img/RealTimeImg/acmap_s/%Y%m%d/%Y%m%d%H%M%S.acmap_s.gif",
                SERVER "/data/map_img/RealTimeImg/vcmap_s/%Y%m%d/%Y%m%d%H%M%S.vcmap_s.gif",
                SERVER "/data/map_img/RealTimeImg/dcmap_s/%Y%m%d/%Y%m%d%H%M%S.dcmap_s.gif",
                SERVER "/data/map_img/RealTimeImg/rsp0125_s/%Y%m%d/%Y%m%d%H%M%S.rsp0125_s.gif",
                SERVER "/data/map_img/RealTimeImg/rsp0250_s/%Y%m%d/%Y%m%d%H%M%S.rsp0250_s.gif",
                SERVER "/data/map_img/RealTimeImg/rsp0500_s/%Y%m%d/%Y%m%d%H%M%S.rsp0500_s.gif",
                SERVER "/data/map_img/RealTimeImg/rsp1000_s/%Y%m%d/%Y%m%d%H%M%S.rsp1000_s.gif",
                SERVER "/data/map_img/RealTimeImg/rsp2000_s/%Y%m%d/%Y%m%d%H%M%S.rsp2000_s.gif",
                SERVER "/data/map_img/RealTimeImg/rsp4000_s/%Y%m%d/%Y%m%d%H%M%S.rsp4000_s.gif",
            },
            .boreholeUrlFormats = {
                SERVER "/data/map_img/RealTimeImg/jma_b/%Y%m%d/%Y%m%d%H%M%S.jma_b.gif",
                SERVER "/data/map_img/RealTimeImg/acmap_b/%Y%m%d/%Y%m%d%H%M%S.acmap_b.gif",
                SERVER "/data/map_img/RealTimeImg/vcmap_b/%Y%m%d/%Y%m%d%H%M%S.vcmap_b.gif",
                SERVER "/data/map_img/RealTimeImg/dcmap_b/%Y%m%d/%Y%m%d%H%M%S.dcmap_b.gif",
                SERVER "/data/map_img/RealTimeImg/rsp0125_b/%Y%m%d/%Y%m%d%H%M%S.rsp0125_b.gif",
                SERVER "/data/map_img/RealTimeImg/rsp0250_b/%Y%m%d/%Y%m%d%H%M%S.rsp0250_b.gif",
                SERVER "/data/map_img/RealTimeImg/rsp0500_b/%Y%m%d/%Y%m%d%H%M%S.rsp0500_b.gif",
                SERVER "/data/map_img/RealTimeImg/rsp1000_b/%Y%m%d/%Y%m%d%H%M%S.rsp1000_b.gif",
                SERVER "/data/map_img/RealTimeImg/rsp2000_b/%Y%m%d/%Y%m%d%H%M%S.rsp2000_b.gif",
                SERVER "/data/map_img/RealTimeImg/rsp4000_b/%Y%m%d/%Y%m%d%H%M%S.rsp4000_b.gif",
            },
            .psWaveUrlFormat = SERVER "/data/map_img/PSWaveImg/eew/%Y%m%d/%Y%m%d%H%M%S.eew.gif",
        },
        {
            .identifier = "noto",
            .baseMapUrl = SERVER "/data/map_img/CommonImg_noto/base_map_w.gif",
            .surfaceUrlFormats = {
                SERVER "/data/map_img/RealTimeImg_noto/jma_s/%Y%m%d/%Y%m%d%H%M%S.jma_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/acmap_s/%Y%m%d/%Y%m%d%H%M%S.acmap_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/vcmap_s/%Y%m%d/%Y%m%d%H%M%S.vcmap_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/dcmap_s/%Y%m%d/%Y%m%d%H%M%S.dcmap_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp0125_s/%Y%m%d/%Y%m%d%H%M%S.rsp0125_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp0250_s/%Y%m%d/%Y%m%d%H%M%S.rsp0250_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp0500_s/%Y%m%d/%Y%m%d%H%M%S.rsp0500_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp1000_s/%Y%m%d/%Y%m%d%H%M%S.rsp1000_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp2000_s/%Y%m%d/%Y%m%d%H%M%S.rsp2000_s.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp4000_s/%Y%m%d/%Y%m%d%H%M%S.rsp4000_s.gif",
            },
            .boreholeUrlFormats = {
                SERVER "/data/map_img/RealTimeImg_noto/jma_b/%Y%m%d/%Y%m%d%H%M%S.jma_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/acmap_b/%Y%m%d/%Y%m%d%H%M%S.acmap_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/vcmap_b/%Y%m%d/%Y%m%d%H%M%S.vcmap_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/dcmap_b/%Y%m%d/%Y%m%d%H%M%S.dcmap_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp0125_b/%Y%m%d/%Y%m%d%H%M%S.rsp0125_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp0250_b/%Y%m%d/%Y%m%d%H%M%S.rsp0250_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp0500_b/%Y%m%d/%Y%m%d%H%M%S.rsp0500_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp1000_b/%Y%m%d/%Y%m%d%H%M%S.rsp1000_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp2000_b/%Y%m%d/%Y%m%d%H%M%S.rsp2000_b.gif",
                SERVER "/data/map_img/RealTimeImg_noto/rsp4000_b/%Y%m%d/%Y%m%d%H%M%S.rsp4000_b.gif",
            },
            .psWaveUrlFormat = SERVER "/data/map_img/PSWaveImg_noto/eew/%Y%m%d/%Y%m%d%H%M%S.eew.gif",
        },
    },
    .forecastUrlFormat = SERVER "/webservice/hypo/eew/%Y%m%d%H%M%S.json",
};
