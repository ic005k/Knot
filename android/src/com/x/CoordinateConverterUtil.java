package com.x;

import com.tencent.tencentmap.mapsdk.maps.model.LatLng;

/**
 * 坐标转换工具类（独立于Activity，可随时调用）
 */
public class CoordinateConverterUtil {

    /**
     * WGS84转GCJ02（复用原TencentMapActivity的正确逻辑）
     */
    public static LatLng wgs84ToGcj02(double wgsLat, double wgsLng) {
        if (outOfChina(wgsLat, wgsLng)) {
            return new LatLng(wgsLat, wgsLng);
        }

        double dLat = transformLat(wgsLng - 105.0, wgsLat - 35.0);
        double dLng = transformLng(wgsLng - 105.0, wgsLat - 35.0);
        double radLat = (wgsLat / 180.0) * Math.PI;
        double magic = Math.sin(radLat);
        magic = 1 - 0.00669342162296594323 * magic * magic;
        double sqrtMagic = Math.sqrt(magic);
        dLat =
            (dLat * 180.0) /
            (((6335552.717000417 +
                        (20014120.176965646 - 3018127.966634887 * magic) *
                        magic) /
                    sqrtMagic /
                    magic) *
                Math.PI);
        dLng =
            (dLng * 180.0) /
            ((6378245.0 / sqrtMagic) * Math.cos(radLat) * Math.PI);
        double gcjLat = wgsLat + dLat;
        double gcjLng = wgsLng + dLng;
        return new LatLng(gcjLat, gcjLng);
    }

    /**
     * 判断坐标是否在中国境外
     */
    private static boolean outOfChina(double lat, double lng) {
        return !(lng > 73.66 && lng < 135.05 && lat > 3.86 && lat < 53.55);
    }

    /**
     * 纬度转换辅助计算
     */
    private static double transformLat(double x, double y) {
        double ret =
            -100.0 +
            2.0 * x +
            3.0 * y +
            0.2 * y * y +
            0.1 * x * y +
            0.2 * Math.sqrt(Math.abs(x));
        ret +=
            ((20.0 * Math.sin(6.0 * x * Math.PI) +
                    20.0 * Math.sin(2.0 * x * Math.PI)) *
                2.0) /
            3.0;
        ret +=
            ((20.0 * Math.sin(y * Math.PI) +
                    40.0 * Math.sin((y / 3.0) * Math.PI)) *
                2.0) /
            3.0;
        ret +=
            ((160.0 * Math.sin((y / 12.0) * Math.PI) +
                    320 * Math.sin((y * Math.PI) / 30.0)) *
                2.0) /
            3.0;
        return ret;
    }

    /**
     * 经度转换辅助计算
     */
    private static double transformLng(double x, double y) {
        double ret =
            300.0 +
            x +
            2.0 * y +
            0.1 * x * x +
            0.1 * x * y +
            0.1 * Math.sqrt(Math.abs(x));
        ret +=
            ((20.0 * Math.sin(6.0 * x * Math.PI) +
                    20.0 * Math.sin(2.0 * x * Math.PI)) *
                2.0) /
            3.0;
        ret +=
            ((20.0 * Math.sin(x * Math.PI) +
                    40.0 * Math.sin((x / 3.0) * Math.PI)) *
                2.0) /
            3.0;
        ret +=
            ((150.0 * Math.sin((x / 12.0) * Math.PI) +
                    300.0 * Math.sin((x / 30.0) * Math.PI)) *
                2.0) /
            3.0;
        return ret;
    }
}
