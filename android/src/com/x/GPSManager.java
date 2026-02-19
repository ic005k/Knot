// 新建GPSManager.java文件
package com.x;

import android.content.Context;
import android.location.Location;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.Build;
import android.util.Log;
import androidx.core.content.ContextCompat;
import androidx.core.location.LocationListenerCompat;
import androidx.core.location.LocationManagerCompat;
import androidx.core.location.LocationRequestCompat;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class GPSManager {

    private static final String TAG = "GPSManager";
    private static GPSManager instance;
    private final Context appContext;

    // GPS核心资源
    private LocationManager locationManager;
    private ExecutorService gpsExecutor;
    private ExecutorService networkExecutor;
    private LocationListenerCompat gpsLocationListener;
    private LocationListenerCompat networkLocationListener;

    // GPS状态标记
    private boolean isGpsRunning = false;
    private boolean isGpsReady = false;
    private Queue<Float> accuracyHistory = new ConcurrentLinkedQueue<>();
    private static final int ACCURACY_WINDOW_SIZE = 5;
    private static final float GPS_GOOD_THRESHOLD = 20.0f;
    private static final float GPS_BAD_THRESHOLD = 30.0f;

    // 运动数据
    private double latitude = 0;
    private double longitude = 0;
    private long startTime = 0L;
    private long movingTime = 0;
    private float totalDistance = 0f;
    private float maxSpeed = 0f;
    private float mySpeed = 0f;
    private float totalClimb = 0f;
    private float totalDescent = 0f;
    private Location previousLocation;
    private double previousAltitude = 0f;

    // 回调接口
    public interface OnLocationUpdateListener {
        void onLocationUpdated(
            double lat,
            double lng,
            float speed,
            float distance
        );
        void onGPSStatusChanged(String status);
    }

    private OnLocationUpdateListener updateListener;

    // 单例模式
    public static GPSManager getInstance(Context context) {
        if (instance == null) {
            instance = new GPSManager(context.getApplicationContext());
        }
        return instance;
    }

    private GPSManager(Context context) {
        this.appContext = context;
        initLocationManager();
        initLocationListeners();
        initExecutors(); // 初始化线程池（仅创建一次，除非主动销毁）
    }

    // 初始化LocationManager
    private void initLocationManager() {
        if (locationManager == null) {
            locationManager = (LocationManager) appContext.getSystemService(
                Context.LOCATION_SERVICE
            );
        }
    }

    // 初始化线程池（关键修改：增加判断，避免重复创建/销毁）
    private void initExecutors() {
        // GPS线程池（高优先级）
        if (
            gpsExecutor == null ||
            gpsExecutor.isShutdown() ||
            gpsExecutor.isTerminated()
        ) {
            gpsExecutor = Executors.newSingleThreadExecutor(runnable -> {
                Thread thread = new Thread(runnable, "Global-GPS-Thread");
                thread.setPriority(Thread.MAX_PRIORITY);
                return thread;
            });
        }
        // 网络定位线程池
        if (
            networkExecutor == null ||
            networkExecutor.isShutdown() ||
            networkExecutor.isTerminated()
        ) {
            networkExecutor = Executors.newSingleThreadExecutor(runnable -> {
                Thread thread = new Thread(runnable, "Global-Network-Thread");
                thread.setPriority(Thread.NORM_PRIORITY);
                return thread;
            });
        }
    }

    // 初始化定位监听器
    private void initLocationListeners() {
        // GPS监听器
        gpsLocationListener = new LocationListenerCompat() {
            @Override
            public void onLocationChanged(Location location) {
                if (
                    LocationManager.GPS_PROVIDER.equals(location.getProvider())
                ) {
                    boolean stable = isGpsStable(location);
                    boolean unstable = isGpsUnstable(location);

                    if (stable && !isGpsReady) {
                        isGpsReady = true;
                        stopNetworkLocationUpdates();
                        Log.i(TAG, "GPS稳定，切回GPS定位");
                    } else if (unstable && isGpsReady) {
                        isGpsReady = false;
                        startNetworkLocationUpdates();
                        Log.i(TAG, "GPS不稳定，切换到网络定位");
                    }

                    latitude = location.getLatitude();
                    longitude = location.getLongitude();
                    updateTrackingData(location);

                    if (updateListener != null) {
                        updateListener.onLocationUpdated(
                            latitude,
                            longitude,
                            mySpeed,
                            totalDistance
                        );
                    }
                }
            }

            @Override
            public void onStatusChanged(
                String provider,
                int status,
                android.os.Bundle extras
            ) {
                String statusStr = switch (status) {
                    case LocationProvider.AVAILABLE -> "GPS: Available";
                    case LocationProvider.OUT_OF_SERVICE -> "GPS: Out of Service";
                    case LocationProvider.TEMPORARILY_UNAVAILABLE -> "GPS: Temporarily Unavailable";
                    default -> "GPS: Unknown";
                };
                Log.i(TAG, "GPS Status: " + statusStr);
                if (updateListener != null) {
                    updateListener.onGPSStatusChanged(statusStr);
                }
            }

            @Override
            public void onProviderEnabled(String provider) {
                Log.d(TAG, "Provider enabled: " + provider);
                if (
                    LocationManager.GPS_PROVIDER.equals(provider) &&
                    isGpsRunning
                ) {
                    try {
                        LocationManagerCompat.removeUpdates(
                            locationManager,
                            gpsLocationListener
                        );
                        LocationRequestCompat locationRequest =
                            new LocationRequestCompat.Builder(2000L)
                                .setMinUpdateDistanceMeters(1.0f)
                                .build();
                        LocationManagerCompat.requestLocationUpdates(
                            locationManager,
                            LocationManager.GPS_PROVIDER,
                            locationRequest,
                            gpsExecutor,
                            gpsLocationListener
                        );
                        previousLocation = null;
                        Log.i(TAG, "GPS重新启用，重新注册监听器");
                    } catch (SecurityException e) {
                        Log.e(TAG, "重新注册GPS监听器失败", e);
                    }
                }
            }

            @Override
            public void onProviderDisabled(String provider) {
                Log.d(TAG, "Provider disabled: " + provider);
            }
        };

        // 网络定位监听器
        networkLocationListener = new LocationListenerCompat() {
            @Override
            public void onLocationChanged(Location location) {
                if (!isGpsReady) {
                    latitude = location.getLatitude();
                    longitude = location.getLongitude();
                    updateTrackingData(location);
                    if (updateListener != null) {
                        updateListener.onLocationUpdated(
                            latitude,
                            longitude,
                            mySpeed,
                            totalDistance
                        );
                    }
                    Log.i(
                        TAG,
                        "网络定位更新：精度=" + location.getAccuracy() + "米"
                    );
                }
            }

            @Override
            public void onStatusChanged(
                String provider,
                int status,
                android.os.Bundle extras
            ) {
                Log.d(
                    TAG,
                    "网络定位状态变化：" + provider + "，状态=" + status
                );
            }

            @Override
            public void onProviderEnabled(String provider) {
                Log.d(TAG, "网络定位已启用：" + provider);
            }

            @Override
            public void onProviderDisabled(String provider) {
                Log.d(TAG, "网络定位已禁用：" + provider);
            }
        };
    }

    // 启动GPS（关键修改：启动前重新初始化线程池）
    public boolean startGPS(OnLocationUpdateListener listener) {
        if (isGpsRunning) {
            Log.w(TAG, "GPS已在运行");
            return true;
        }

        // 1. 检查权限
        String finePerm = android.Manifest.permission.ACCESS_FINE_LOCATION;
        String coarsePerm = android.Manifest.permission.ACCESS_COARSE_LOCATION;
        boolean hasFine =
            ContextCompat.checkSelfPermission(appContext, finePerm) ==
            android.content.pm.PackageManager.PERMISSION_GRANTED;
        boolean hasCoarse =
            ContextCompat.checkSelfPermission(appContext, coarsePerm) ==
            android.content.pm.PackageManager.PERMISSION_GRANTED;
        if (!hasFine && !hasCoarse) {
            Log.e(TAG, "缺少定位权限，启动GPS失败");
            return false;
        }

        // 2. 关键修复：启动前重新初始化线程池（避免stop后线程池为空）
        initExecutors();

        // 3. 重置运动数据
        resetTrackingData();

        // 4. 设置回调
        this.updateListener = listener;

        // 5. 检查定位服务是否开启
        boolean gpsEnabled = locationManager.isProviderEnabled(
            LocationManager.GPS_PROVIDER
        );
        boolean networkEnabled = locationManager.isProviderEnabled(
            LocationManager.NETWORK_PROVIDER
        );
        if (!gpsEnabled && !networkEnabled) {
            Log.e(TAG, "位置服务未开启，启动GPS失败");
            return false;
        }

        // 6. 启动GPS定位
        try {
            LocationRequestCompat gpsRequest =
                new LocationRequestCompat.Builder(2000L)
                    .setMinUpdateDistanceMeters(1.0f)
                    .build();
            LocationManagerCompat.requestLocationUpdates(
                locationManager,
                LocationManager.GPS_PROVIDER,
                gpsRequest,
                gpsExecutor,
                gpsLocationListener
            );
            Log.i(TAG, "GPS定位已启动");
        } catch (SecurityException e) {
            Log.e(TAG, "启动GPS定位失败（真实错误）", e); // 打印真实错误，便于排查
            return false;
        }

        // 7. 启动网络定位（若有粗定位权限）
        if (hasCoarse) {
            try {
                LocationRequestCompat networkRequest =
                    new LocationRequestCompat.Builder(2000L)
                        .setMinUpdateDistanceMeters(5.0f)
                        .build();
                LocationManagerCompat.requestLocationUpdates(
                    locationManager,
                    LocationManager.NETWORK_PROVIDER,
                    networkRequest,
                    networkExecutor,
                    networkLocationListener
                );
                Log.i(TAG, "网络定位已启动");
            } catch (SecurityException e) {
                Log.e(TAG, "启动网络定位失败", e);
            }
        }

        isGpsRunning = true;
        isGpsReady = false;
        return true;
    }

    // 停止GPS（关键修改：不销毁线程池，仅移除监听器）
    public void stopGPS() {
        if (!isGpsRunning) {
            return;
        }

        // 1. 移除监听器（核心停止逻辑）
        if (locationManager != null) {
            try {
                LocationManagerCompat.removeUpdates(
                    locationManager,
                    gpsLocationListener
                );
                LocationManagerCompat.removeUpdates(
                    locationManager,
                    networkLocationListener
                );
                Log.i(TAG, "定位监听器已移除");
            } catch (SecurityException e) {
                Log.e(TAG, "移除定位监听器失败", e);
            }
        }

        // 2. 关键修改：不再销毁线程池，仅重置状态（线程池复用）
        // 注释掉销毁线程池的代码，避免再次启动时线程池为空
        // if (gpsExecutor != null) { ... }
        // if (networkExecutor != null) { ... }

        // 3. 重置状态
        isGpsRunning = false;
        isGpsReady = false;
        updateListener = null;
        //resetTrackingData();
        Log.i(TAG, "GPS已停止（线程池保留，可复用）");
    }

    // 可选：App退出时销毁线程池（建议在Application的onTerminate中调用）
    public void destroy() {
        // 销毁GPS线程池
        if (gpsExecutor != null) {
            gpsExecutor.shutdownNow();
            try {
                if (!gpsExecutor.awaitTermination(1, TimeUnit.SECONDS)) {
                    Log.w(TAG, "GPS线程池未正常终止");
                }
            } catch (InterruptedException e) {
                gpsExecutor.shutdownNow();
                Thread.currentThread().interrupt();
            }
            gpsExecutor = null;
        }

        // 销毁网络定位线程池
        if (networkExecutor != null) {
            networkExecutor.shutdownNow();
            try {
                if (!networkExecutor.awaitTermination(1, TimeUnit.SECONDS)) {
                    Log.w(TAG, "网络定位线程池未正常终止");
                }
            } catch (InterruptedException e) {
                networkExecutor.shutdownNow();
                Thread.currentThread().interrupt();
            }
            networkExecutor = null;
        }

        Log.i(TAG, "GPSManager资源已销毁");
    }

    // 重置运动数据
    private void resetTrackingData() {
        latitude = 0;
        longitude = 0;

        startTime = System.currentTimeMillis();
        movingTime = 0;
        totalDistance = 0f;
        maxSpeed = 0f;
        mySpeed = 0f;
        totalClimb = 0f;
        totalDescent = 0f;
        previousLocation = null;
        previousAltitude = 0f;
        accuracyHistory.clear();
    }

    // 更新运动数据
    private void updateTrackingData(Location location) {
        if (previousLocation != null) {
            if (location.getSpeed() > 0) {
                long currentTime = System.currentTimeMillis();
                movingTime += currentTime - startTime;
                startTime = currentTime;

                totalDistance += previousLocation.distanceTo(location) / 1000;
                mySpeed = location.getSpeed() * 3.6f;
                if (mySpeed > maxSpeed) {
                    maxSpeed = mySpeed;
                }

                double currentAlt = location.getAltitude();
                if (
                    previousAltitude != 0 &&
                    currentAlt > -100 &&
                    currentAlt < 9000
                ) {
                    double diff = currentAlt - previousAltitude;
                    if (diff > 0) {
                        totalClimb += diff;
                    } else if (diff < 0) {
                        totalDescent += Math.abs(diff);
                    }
                }
                previousAltitude = currentAlt;
            } else {
                startTime = System.currentTimeMillis();
            }
        } else {
            previousAltitude = location.getAltitude();
        }
        previousLocation = location;
    }

    // 启动网络定位
    private void startNetworkLocationUpdates() {
        if (
            locationManager == null ||
            networkLocationListener == null ||
            !isGpsRunning
        ) {
            return;
        }

        boolean networkEnabled = locationManager.isProviderEnabled(
            LocationManager.NETWORK_PROVIDER
        );
        if (!networkEnabled) {
            Log.w(TAG, "网络定位未启用，无法启动");
            return;
        }

        String coarsePerm = android.Manifest.permission.ACCESS_COARSE_LOCATION;
        if (
            ContextCompat.checkSelfPermission(appContext, coarsePerm) !=
            android.content.pm.PackageManager.PERMISSION_GRANTED
        ) {
            Log.w(TAG, "缺少粗定位权限，无法启动网络定位");
            return;
        }

        try {
            LocationRequestCompat networkRequest =
                new LocationRequestCompat.Builder(2000L)
                    .setMinUpdateDistanceMeters(5.0f)
                    .build();
            LocationManagerCompat.requestLocationUpdates(
                locationManager,
                LocationManager.NETWORK_PROVIDER,
                networkRequest,
                networkExecutor,
                networkLocationListener
            );
            Log.i(TAG, "网络定位已重新启用");
        } catch (SecurityException e) {
            Log.e(TAG, "启动网络定位失败", e);
        }
    }

    // 停止网络定位
    private void stopNetworkLocationUpdates() {
        if (locationManager != null && networkLocationListener != null) {
            try {
                LocationManagerCompat.removeUpdates(
                    locationManager,
                    networkLocationListener
                );
                Log.i(TAG, "网络定位已停止");
            } catch (SecurityException e) {
                Log.e(TAG, "停止网络定位失败", e);
            }
        }
    }

    // 判断GPS是否稳定
    private boolean isGpsStable(Location location) {
        if (location.getLatitude() == 0 || location.getLongitude() == 0) {
            return false;
        }

        accuracyHistory.offer(location.getAccuracy());
        if (accuracyHistory.size() > ACCURACY_WINDOW_SIZE) {
            accuracyHistory.poll();
        }

        if (accuracyHistory.size() < ACCURACY_WINDOW_SIZE) {
            return false;
        }

        long goodCount = accuracyHistory
            .stream()
            .filter(acc -> acc <= GPS_GOOD_THRESHOLD)
            .count();
        return goodCount >= ACCURACY_WINDOW_SIZE * 0.8;
    }

    // 判断GPS是否不稳定
    private boolean isGpsUnstable(Location location) {
        if (location.getLatitude() == 0 || location.getLongitude() == 0) {
            return true;
        }

        if (accuracyHistory.size() < ACCURACY_WINDOW_SIZE) {
            return false;
        }

        long badCount = accuracyHistory
            .stream()
            .filter(acc -> acc > GPS_BAD_THRESHOLD)
            .count();
        return badCount >= ACCURACY_WINDOW_SIZE * 0.8;
    }

    // 获取GPS运行状态
    public boolean isGpsRunning() {
        return isGpsRunning;
    }

    // 获取运动数据
    public double getLatitude() {
        return latitude;
    }

    public double getLongitude() {
        return longitude;
    }

    public float getTotalDistance() {
        return totalDistance;
    }

    public float getMaxSpeed() {
        return maxSpeed;
    }

    public float getMySpeed() {
        return mySpeed;
    }

    public float getTotalClimb() {
        return totalClimb;
    }

    public float getTotalDescent() {
        return totalDescent;
    }

    public long getMovingTime() {
        return movingTime;
    }

    public double getPreviousAltitude() {
        return previousAltitude;
    }

    public boolean isGpsReady() {
        return isGpsReady;
    }

    // 设置定位更新回调（供外部设置/更新监听器）
    public void setUpdateListener(OnLocationUpdateListener listener) {
        this.updateListener = listener;
    }

    // 可选：获取当前回调（便于调试）
    public OnLocationUpdateListener getUpdateListener() {
        return this.updateListener;
    }
}
