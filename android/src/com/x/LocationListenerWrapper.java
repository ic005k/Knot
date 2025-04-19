package com.x;

import android.content.Intent;
import java.util.Iterator;
import java.util.Locale;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import android.util.Log;

import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.location.LocationListenerCompat;
import androidx.core.location.LocationManagerCompat;
import androidx.core.location.LocationRequestCompat;
import android.location.LocationRequest;
import android.location.LocationProvider;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.provider.Settings;
import androidx.appcompat.app.AlertDialog;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import android.os.SystemClock;

import android.content.Context;

import android.location.Location;

import android.location.LocationListener;

import android.location.LocationManager;

import android.os.Bundle;

public class LocationListenerWrapper implements LocationListenerCompat {
    private static final String TAG = "QtKnot";
    public static boolean zh_cn;
    private Context myContext;
    private LocationListenerCompat locationListener1;

    private LocationManager locationManager;
    private double latitude = 0;
    private double longitude = 0;
    private Executor executor;

    private boolean isTracking = false;
    private long startTime = 0L;
    private long movingTime;
    private float totalDistance = 0f;
    private float maxSpeed = 0f;
    private float mySpeed = 0f;
    private float totalClimb = 0f;
    private Location previousLocation;
    private double previousAltitude;

    private String strGpsStatus = "GPS Status";
    private String strRunTime = "00:00:00";
    private String strAltitude = "Altitude";
    private String strTotalDistance = "0 km";
    private String strMaxSpeed = "Max Speed";
    private String strTotalClimb = "Total Climb";
    private String strAverageSpeed = "0 km/h";

    public static boolean isZh(Context context) {
        Locale locale = context.getResources().getConfiguration().locale;
        String language = locale.getLanguage();
        if (language.endsWith("zh"))
            zh_cn = true;
        else
            zh_cn = false;

        return zh_cn;
    }

    public LocationListenerWrapper(Context context) {
        myContext = context;
        locationListener1 = this;

        // 初始化LocationManager
        locationManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        executor = Executors.newSingleThreadExecutor();
        isZh(context);
    }

    // 使用LocationListenerCompat定义位置监听器
    // private final LocationListenerCompat locationListener1 = new
    // LocationListenerCompat() {
    @Override
    public void onLocationChanged(@NonNull Location location) {
        // 位置更新时触发
        latitude = location.getLatitude();
        longitude = location.getLongitude();
        updateTrackingData(location);

    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {
        switch (status) {
            case LocationProvider.AVAILABLE:
                strGpsStatus = "GPS: Available";
                break;
            case LocationProvider.OUT_OF_SERVICE:
                strGpsStatus = "GPS: Out of Service";
                break;
            case LocationProvider.TEMPORARILY_UNAVAILABLE:
                strGpsStatus = "GPS: Temporarily Unavailable";
                break;
        }
        Log.i(TAG, "GPS Status: " + strGpsStatus);
    }

    @Override
    public void onProviderEnabled(@NonNull String provider) {
        Log.d(TAG, "Provider enabled: " + provider);
    }

    @Override
    public void onProviderDisabled(@NonNull String provider) {
        Log.d(TAG, "Provider disabled: " + provider);
    }
    // };

    private final GpsStatus.Listener gpsStatusListener = new GpsStatus.Listener() {
        @Override
        public void onGpsStatusChanged(int event) {
            if (locationManager != null) {
                GpsStatus gpsStatus = locationManager.getGpsStatus(null);
                switch (event) {
                    case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                        Iterable<GpsSatellite> satellites = gpsStatus.getSatellites();
                        Iterator<GpsSatellite> it = satellites.iterator();
                        int satelliteCount = 0;
                        StringBuilder statusText = new StringBuilder();
                        while (it.hasNext()) {
                            GpsSatellite satellite = it.next();
                            satelliteCount++;
                            statusText.append("卫星 ").append(satelliteCount).append(" 强度: ")
                                    .append(satellite.getSnr()).append("\n");
                        }
                        statusText.insert(0, "可见卫星数量: ").append(satelliteCount).append("\n");
                        strGpsStatus = statusText.toString();
                        break;
                    case GpsStatus.GPS_EVENT_FIRST_FIX:
                        // 首次定位成功
                        break;
                    case GpsStatus.GPS_EVENT_STARTED:
                        // GPS启动
                        break;
                    case GpsStatus.GPS_EVENT_STOPPED:
                        // GPS停止
                        break;
                }
            }
        }
    };

    public String getGpsStatus() {
        return strTotalDistance + "\n" + strRunTime + "\n" + strAverageSpeed + "\n" + strMaxSpeed + "\n" + strAltitude
                + "\n" + strTotalClimb + "\n" + strGpsStatus;
    }

    public double startGpsUpdates() {

        latitude = 0;
        longitude = 0;
        startTime = System.currentTimeMillis();
        totalDistance = 0f;
        maxSpeed = 0f;
        mySpeed = 0f;
        totalClimb = 0f;
        previousLocation = null;
        movingTime = 0;
        previousAltitude = 0f;

        if (locationManager != null) {

            // 创建 LocationRequestCompat 对象
            LocationRequestCompat locationRequest = new LocationRequestCompat.Builder(2000L) // 最小时间间隔
                    .setMinUpdateDistanceMeters(1.0f) // 最小距离间隔
                    .build();
            // 使用 LocationManagerCompat 请求位置更新（兼容 Android 6.0+）
            LocationManagerCompat.requestLocationUpdates(
                    locationManager,
                    LocationManager.GPS_PROVIDER, // 使用 GPS 提供者
                    locationRequest,
                    executor,
                    locationListener1);

            // 添加GPS状态侦听
            if (locationManager != null) {
                locationManager.addGpsStatusListener(gpsStatusListener);
            }

            return 1;

        }
        return 0;
    }

    public double getTotalDistance() {

        return totalDistance;

    }

    public double getMySpeed() {

        return mySpeed;

    }

    public double getLatitude() {

        return latitude;

    }

    public double getLongitude() {

        return longitude;

    }

    // 停止 GPS 更新
    public double stopGpsUpdates() {
        if (locationManager != null && locationListener1 != null) {
            try {
                // locationManager.removeUpdates(locationListener1);
                LocationManagerCompat.removeUpdates(locationManager, locationListener1);

                // 停止GPS状态侦听
                if (locationManager != null) {
                    locationManager.removeGpsStatusListener(gpsStatusListener);
                }
            } catch (SecurityException e) {
                e.printStackTrace();
            }

        }
        return totalDistance;
    }

    // 更新运动数据
    private void updateTrackingData(Location currentLocation) {
        if (previousLocation != null) {
            if (currentLocation.getSpeed() > 0) {
                // 运动状态
                long currentTime = System.currentTimeMillis();
                movingTime += currentTime - startTime;
                startTime = currentTime;

                // 计算距离
                totalDistance += previousLocation.distanceTo(currentLocation) / 1000; // 转换为公里

                // 计算最大速度
                mySpeed = currentLocation.getSpeed() * 3.6f;// 转换为 km/h
                if (mySpeed > maxSpeed) {
                    maxSpeed = mySpeed;
                }

                // 计算爬升
                double currentAltitude = currentLocation.getAltitude();
                if (currentAltitude > previousAltitude) {
                    totalClimb += currentAltitude - previousAltitude;
                }
                previousAltitude = currentAltitude;
            } else {
                // 静止状态，更新开始时间
                startTime = System.currentTimeMillis();
            }
        } else {
            previousAltitude = currentLocation.getAltitude();
        }

        // 更新 UI
        updateUI(currentLocation);

        previousLocation = currentLocation;
    }

    private void updateUI(Location location) {
        // 运动距离
        strTotalDistance = String.format("%.2f km", totalDistance);

        // 运动时间
        long seconds = movingTime / 1000;
        strRunTime = String.format("%02d:%02d:%02d", seconds / 3600, (seconds % 3600) / 60, seconds % 60);

        // 平均速度
        double avgSpeed = totalDistance / (movingTime / 3600000f);
        strAverageSpeed = String.format("%.2f km/h", avgSpeed);

        // 最大速度
        if (zh_cn)
            strMaxSpeed = String.format("最大速度: %.2f km/h", maxSpeed);
        else
            strMaxSpeed = String.format("Max Speed: %.2f km/h", maxSpeed);

        // 海拔
        if (zh_cn)
            strAltitude = String.format("海拔: %.2f 米", location.getAltitude());
        else
            strAltitude = String.format("Altitude: %.2f 米", location.getAltitude());

        // 爬升
        if (zh_cn)
            strTotalClimb = String.format("累计爬升: %.2f 米", totalClimb);
        else
            strTotalClimb = String.format("Total Climb: %.2f 米", totalClimb);
    }

}
