package com.x;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout; // 导入FrameLayout类
import android.widget.TextView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import com.tencent.tencentmap.mapsdk.maps.CameraUpdate;
import com.tencent.tencentmap.mapsdk.maps.CameraUpdateFactory;
import com.tencent.tencentmap.mapsdk.maps.MapView;
import com.tencent.tencentmap.mapsdk.maps.TencentMap;
import com.tencent.tencentmap.mapsdk.maps.TencentMapOptions;
import com.tencent.tencentmap.mapsdk.maps.UiSettings;
import com.tencent.tencentmap.mapsdk.maps.model.BitmapDescriptor;
import com.tencent.tencentmap.mapsdk.maps.model.BitmapDescriptorFactory;
import com.tencent.tencentmap.mapsdk.maps.model.CameraPosition;
import com.tencent.tencentmap.mapsdk.maps.model.LatLng;
import com.tencent.tencentmap.mapsdk.maps.model.Marker;
import com.tencent.tencentmap.mapsdk.maps.model.MarkerOptions;
import com.tencent.tencentmap.mapsdk.maps.model.Polyline;
import com.tencent.tencentmap.mapsdk.maps.model.PolylineOptions;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

public class TencentMapActivity extends MapActivity {

    private static final String TAG = "TencentMap_Final";
    private static String TENCENT_MAP_KEY;
    private static final double OSLO_LATITUDE = 39.9042;
    private static final double OSLO_LONGITUDE = 116.4074;
    private static final int REQUEST_PERMISSIONS_REQUEST_CODE = 1;
    private static final int DEFAULT_ZOOM_LEVEL = 13;

    private MapView tencentMapView;
    private TencentMap tencentMap;
    private UiSettings mapUiSettings;
    private Polyline trackPolyline;
    private Marker currentLocationMarker;

    private List<LatLng> trackPoints = new ArrayList<>();
    private List<LatLng> globalTrackPoints = new ArrayList<>();

    public static TextView topDateLabel;
    private TextView topInfoLabel;
    public static TextView bottomInfoLabel;
    private Button switchMapBtn;
    private boolean usingStandardMap = true;

    private static final String KEY_PRIVACY_AGREED =
        "tencent_map_privacy_agreed";
    private boolean isInitialized = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyActivity.mapActivityInstance = this;

        Log.d(TAG, "开始初始化腾讯地图Activity");
        TENCENT_MAP_KEY = MyActivity.MY_TENCENT_MAP_KEY;
        Log.d(TAG, "当前使用的Key: " + TENCENT_MAP_KEY);

        setupTencentSDK();

        checkPermissions();

        if (!hasAgreedPrivacy()) {
            Log.d(TAG, "用户未同意隐私协议，显示弹窗");
            showPrivacyDialog();
            return;
        }

        Log.d(TAG, "用户已同意隐私协议，继续初始化");
        initializeMap();
    }

    private void setupTencentSDK() {
        try {
            Log.d(TAG, "开始设置腾讯地图SDK");
            // 仅保留隐私协议设置，移除SDK初始化逻辑
            boolean agreed = hasAgreedPrivacy();
            setTencentPrivacyStatus(agreed);
            Log.d(TAG, "隐私协议状态设置: " + agreed);
        } catch (Exception e) {
            Log.e(TAG, "SDK设置失败", e);
        }
    }

    private void setTencentPrivacyStatus(boolean agreed) {
        try {
            Class<?> initializerClass = Class.forName(
                "com.tencent.tencentmap.mapsdk.maps.TencentMapInitializer"
            );
            String[] methodNames = {
                "setAgreePrivacy",
                "setUserAgreePrivacy",
                "setPrivacyAgreed",
                "agreePrivacy",
            };
            for (String methodName : methodNames) {
                try {
                    Method method = initializerClass.getMethod(
                        methodName,
                        boolean.class
                    );
                    method.invoke(null, agreed);
                    Log.d(
                        TAG,
                        "隐私协议设置成功: " + methodName + " = " + agreed
                    );
                    break;
                } catch (NoSuchMethodException e) {
                    continue;
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "隐私协议设置失败", e);
            setPrivacyStatusSimple(agreed);
        }
    }

    private void setPrivacyStatusSimple(boolean agreed) {
        try {
            System.setProperty("tencent.map.agreed", String.valueOf(agreed));
            System.setProperty("privacy.agreed", String.valueOf(agreed));
            SharedPreferences sp = getSharedPreferences(
                "tencent_global_config",
                MODE_PRIVATE
            );
            sp.edit().putBoolean("privacy_agreed", agreed).apply();
            Log.d(TAG, "使用简单方式设置隐私状态: " + agreed);
        } catch (Exception e) {
            Log.e(TAG, "简单设置也失败", e);
        }
    }

    /**
     * 核心：通过TencentMapOptions设置Key（适配您的布局，移除map_container）
     */
    private void initializeMap() {
        if (isInitialized) {
            Log.w(TAG, "地图已经初始化，跳过重复初始化");
            return;
        }

        setContentView(R.layout.activity_tencent_map);
        initViews();
        // 修复：使用XML中的mapView，并通过TencentMapOptions设置Key
        initMapViewWithKey();
        initTencentMap();
        initCenterMarker();

        // 修复：修正ViewTreeObserver方法名（还原您原来的逻辑）
        if (tencentMapView != null) {
            ViewTreeObserver observer = tencentMapView.getViewTreeObserver();
            observer.addOnGlobalLayoutListener(
                new ViewTreeObserver.OnGlobalLayoutListener() {
                    @Override
                    public void onGlobalLayout() {
                        if (
                            Build.VERSION.SDK_INT >=
                            Build.VERSION_CODES.JELLY_BEAN
                        ) {
                            // 修复：正确方法名（带Global）
                            tencentMapView
                                .getViewTreeObserver()
                                .removeOnGlobalLayoutListener(this);
                        } else {
                            tencentMapView
                                .getViewTreeObserver()
                                .removeGlobalOnLayoutListener(this);
                        }

                        new Handler(Looper.getMainLooper()).postDelayed(
                            () -> {
                                if (
                                    tencentMapView != null &&
                                    !isFinishing() &&
                                    !isDestroyed()
                                ) {
                                    convertOsmTrackPoints();
                                    drawAllTrackPoints();
                                    if (topDateLabel != null) {
                                        topDateLabel.setText(
                                            MyActivity.lblDate
                                        );
                                    }
                                    if (bottomInfoLabel != null) {
                                        bottomInfoLabel.setText(
                                            MyActivity.lblInfo
                                        );
                                    }
                                }
                            },
                            100
                        );
                    }
                }
            );
        }

        isInitialized = true;
        Log.d(TAG, "地图界面初始化完成");
    }

    private void initMapViewWithKey() {
        try {
            // 1. 创建带Key的配置
            TencentMapOptions options = new TencentMapOptions();
            options.setMapKey(TENCENT_MAP_KEY); // 动态传入Key
            Log.d(TAG, "通过代码创建MapView，已设置Key");

            // 2. 代码创建MapView（替代XML的findViewById）
            tencentMapView = new MapView(this, options);

            // 3. 将MapView添加到布局容器（需在XML中添加一个容器，如FrameLayout）
            FrameLayout mapContainer = findViewById(R.id.map_container);
            if (mapContainer != null) {
                mapContainer.addView(tencentMapView);
            } else {
                Log.e(TAG, "未找到地图容器");
            }
        } catch (Exception e) {
            Log.e(TAG, "初始化mapView失败", e);
        }
    }

    /**
     * 创建TencentMapOptions并设置Key
     */
    private TencentMapOptions createMapOptionsWithKey() {
        TencentMapOptions options = new TencentMapOptions();
        // 直接使用官方API设置Key，移除反射备用逻辑
        options.setMapKey(TENCENT_MAP_KEY);
        Log.d(TAG, "通过TencentMapOptions.setMapKey设置Key成功");
        return options;
    }

    private void initViews() {
        try {
            topDateLabel = findViewById(R.id.topDateLabel);
            topInfoLabel = findViewById(R.id.topInfoLabel);
            bottomInfoLabel = findViewById(R.id.bottomInfoLabel);
            switchMapBtn = findViewById(R.id.switchMapBtn);
            // 保留原有逻辑，不改动

            if (topInfoLabel != null) {
                topInfoLabel.setText("加载中...腾讯地图");
            }
            if (bottomInfoLabel != null) {
                bottomInfoLabel.setText("距离: 0.0km | 速度: 0.0km/h");
            }
            if (switchMapBtn != null) {
                switchMapBtn.setOnClickListener(v -> switchMapType());
            }

            Log.d(TAG, "视图初始化完成");
        } catch (Exception e) {
            Log.e(TAG, "视图初始化失败", e);
        }
    }

    // 以下方法与您原有实现完全一致，未做修改
    private void initTencentMap() {
        try {
            if (tencentMapView == null) {
                Log.e(TAG, "MapView为null，无法初始化地图");
                return;
            }

            tencentMap = tencentMapView.getMap();
            if (tencentMap == null) {
                Log.e(TAG, "地图初始化失败");
                if (topInfoLabel != null) {
                    topInfoLabel.setText("地图初始化失败");
                }
                return;
            }

            mapUiSettings = tencentMap.getUiSettings();
            mapUiSettings.setZoomControlsEnabled(true);
            mapUiSettings.setCompassEnabled(true);
            tencentMap.setTrafficEnabled(false);

            LatLng osloCenter = new LatLng(OSLO_LATITUDE, OSLO_LONGITUDE);
            CameraUpdate cameraUpdate = CameraUpdateFactory.newCameraPosition(
                new CameraPosition(osloCenter, DEFAULT_ZOOM_LEVEL, 0, 0)
            );
            tencentMap.moveCamera(cameraUpdate);

            initTrackPolyline();
            initMapListeners();

            if (topInfoLabel != null) {
                topInfoLabel.setText("地图加载完成 - 标准地图");
            }

            Log.d(TAG, "腾讯地图初始化成功");
        } catch (Exception e) {
            Log.e(TAG, "地图初始化异常", e);
            if (topInfoLabel != null) {
                topInfoLabel.setText("地图加载失败");
            }
        }
    }

    private void initMapListeners() {
        if (tencentMap == null) return;

        tencentMap.setOnCameraChangeListener(
            new TencentMap.OnCameraChangeListener() {
                @Override
                public void onCameraChange(CameraPosition cameraPosition) {
                    updateCameraInfo(cameraPosition);
                    if (
                        currentLocationMarker != null &&
                        currentLocationMarker.isVisible()
                    ) {
                        currentLocationMarker.setPosition(
                            cameraPosition.target
                        );
                    }
                }

                @Override
                public void onCameraChangeFinished(
                    CameraPosition cameraPosition
                ) {
                    updateCameraInfo(cameraPosition);
                    if (
                        currentLocationMarker != null &&
                        currentLocationMarker.isVisible()
                    ) {
                        currentLocationMarker.setPosition(
                            cameraPosition.target
                        );
                    }
                }
            }
        );
    }

    private void initTrackPolyline() {
        if (tencentMap == null) return;

        PolylineOptions polylineOptions = new PolylineOptions()
            .color(0xFFFF0000)
            .width(5f);

        trackPolyline = tencentMap.addPolyline(polylineOptions);
    }

    private void initCenterMarker() {
        try {
            LatLng initialPos = new LatLng(OSLO_LATITUDE, OSLO_LONGITUDE);

            Drawable markerDrawable = ContextCompat.getDrawable(
                this,
                R.drawable.marker_center
            );
            if (markerDrawable == null) {
                Log.e(
                    TAG,
                    "XML Shape资源加载失败！请检查R.drawable.marker_center是否存在"
                );
                return;
            }

            Bitmap bitmap = drawableToBitmap(markerDrawable);
            if (bitmap == null) {
                Log.e(TAG, "Drawable转Bitmap失败");
                return;
            }

            BitmapDescriptor markerIcon = BitmapDescriptorFactory.fromBitmap(
                bitmap
            );

            MarkerOptions markerOptions = new MarkerOptions()
                .position(initialPos)
                .icon(markerIcon)
                .anchor(0.5f, 0.5f)
                .visible(true)
                .zIndex(1000);

            currentLocationMarker = tencentMap.addMarker(markerOptions);
            tencentMapView.invalidate();
            Log.d(TAG, "XML Shape图标初始化成功");
        } catch (Exception e) {
            Log.e(TAG, "标记初始化失败", e);
        }
    }

    private Bitmap drawableToBitmap(Drawable drawable) {
        if (drawable instanceof BitmapDrawable) {
            return ((BitmapDrawable) drawable).getBitmap();
        }

        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();
        if (width <= 0) width = dpToPx(30);
        if (height <= 0) height = dpToPx(30);

        Bitmap bitmap = Bitmap.createBitmap(
            width,
            height,
            Bitmap.Config.ARGB_8888
        );
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);
        return bitmap;
    }

    private int dpToPx(int dp) {
        return (int) (dp * getResources().getDisplayMetrics().density + 0.5f);
    }

    private void showPrivacyDialog() {
        new AlertDialog.Builder(this)
            .setTitle("隐私协议同意")
            .setMessage("为了提供地图服务，需要您同意腾讯地图隐私协议")
            .setPositiveButton("同意并继续", (dialog, which) -> {
                Log.d(TAG, "用户同意隐私协议");
                savePrivacyAgreed();
                setTencentPrivacyStatus(true);
                dialog.dismiss();
                initializeMap();
            })
            .setNegativeButton("拒绝", (dialog, which) -> {
                Log.d(TAG, "用户拒绝隐私协议");
                dialog.dismiss();
                finish();
            })
            .setCancelable(false)
            .show();
    }

    private boolean hasAgreedPrivacy() {
        SharedPreferences sp = getSharedPreferences(
            "app_settings",
            MODE_PRIVATE
        );
        return sp.getBoolean(KEY_PRIVACY_AGREED, false);
    }

    private void savePrivacyAgreed() {
        SharedPreferences sp = getSharedPreferences(
            "app_settings",
            MODE_PRIVATE
        );
        sp.edit().putBoolean(KEY_PRIVACY_AGREED, true).apply();
        Log.d(TAG, "隐私协议同意状态已保存");
    }

    private void switchMapType() {
        if (tencentMap == null) return;
        usingStandardMap = !usingStandardMap;
        tencentMap.setMapType(
            usingStandardMap
                ? TencentMap.MAP_TYPE_NORMAL
                : TencentMap.MAP_TYPE_SATELLITE
        );
        if (topInfoLabel != null) {
            topInfoLabel.setText(
                usingStandardMap ? "已切换到标准地图" : "已切换到卫星地图"
            );
        }
    }

    private void convertOsmTrackPoints() {
        globalTrackPoints.clear();
        if (
            MyActivity.osmTrackPoints == null ||
            MyActivity.osmTrackPoints.isEmpty()
        ) return;

        for (Object obj : MyActivity.osmTrackPoints) {
            if (obj instanceof org.osmdroid.util.GeoPoint) {
                org.osmdroid.util.GeoPoint osmPoint =
                    (org.osmdroid.util.GeoPoint) obj;
                // 关键修改：GPS坐标（WGS84）转GCJ02
                LatLng gcjPoint = wgs84ToGcj02(
                    osmPoint.getLatitude(),
                    osmPoint.getLongitude()
                );
                globalTrackPoints.add(gcjPoint);
            }
        }
    }

    private void updateCameraInfo(CameraPosition cameraPosition) {
        if (cameraPosition == null || topInfoLabel == null) return;
        LatLng target = cameraPosition.target;
        topInfoLabel.setText(
            String.format(
                "Zoom: %d | Lat: %.4f | Lng: %.4f",
                (int) cameraPosition.zoom,
                target.latitude,
                target.longitude
            )
        );
    }

    private void checkPermissions() {
        List<String> permissionsNeeded = new ArrayList<>();
        String[] permissions = {
            Manifest.permission.INTERNET,
            Manifest.permission.ACCESS_NETWORK_STATE,
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION,
        };
        for (String permission : permissions) {
            if (
                ContextCompat.checkSelfPermission(this, permission) !=
                PackageManager.PERMISSION_GRANTED
            ) {
                permissionsNeeded.add(permission);
            }
        }
        if (!permissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(
                this,
                permissionsNeeded.toArray(new String[0]),
                REQUEST_PERMISSIONS_REQUEST_CODE
            );
        }
    }

    @Override
    public void appendTrackPoint(double latitude, double longitude) {
        if (
            latitude < -90.0 ||
            latitude > 90.0 ||
            longitude < -180.0 ||
            longitude > 180.0
        ) {
            Log.w(TAG, "非法经纬度，跳过");
            return;
        }
        if (tencentMap == null || trackPolyline == null) return;

        runOnUiThread(() -> {
            try {
                // 关键修改：GPS坐标（WGS84）转GCJ02
                LatLng newPoint = wgs84ToGcj02(latitude, longitude);
                trackPoints.add(newPoint);
                trackPolyline.setPoints(trackPoints);

                CameraUpdate cameraUpdate = CameraUpdateFactory.newLatLng(
                    newPoint
                );
                tencentMap.animateCamera(cameraUpdate);

                if (currentLocationMarker != null) {
                    currentLocationMarker.setPosition(newPoint);
                    currentLocationMarker.setVisible(true);
                }
            } catch (Exception e) {
                Log.e(TAG, "添加轨迹点异常", e);
            }
        });
    }

    @Override
    public void clearTrack() {
        if (tencentMap == null || trackPolyline == null) return;
        runOnUiThread(() -> {
            trackPoints.clear();
            globalTrackPoints.clear();
            trackPolyline.setPoints(trackPoints);
            if (currentLocationMarker != null) currentLocationMarker.setVisible(
                false
            );
        });
    }

    private void drawAllTrackPoints() {
        if (
            globalTrackPoints.isEmpty() ||
            tencentMap == null ||
            trackPolyline == null
        ) return;
        trackPoints.clear();
        trackPoints.addAll(globalTrackPoints);
        trackPolyline.setPoints(trackPoints);

        if (!trackPoints.isEmpty()) {
            LatLng lastPoint = trackPoints.get(trackPoints.size() - 1);
            CameraUpdate cameraUpdate = CameraUpdateFactory.newLatLngZoom(
                lastPoint,
                DEFAULT_ZOOM_LEVEL
            );
            tencentMap.moveCamera(cameraUpdate);
            if (currentLocationMarker != null) {
                currentLocationMarker.setPosition(lastPoint);
                currentLocationMarker.setVisible(true);
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (tencentMapView != null) tencentMapView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (tencentMapView != null) tencentMapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (tencentMapView != null) {
            tencentMapView.onDestroy();
            tencentMapView = null;
        }
        trackPoints.clear();
        globalTrackPoints.clear();
        tencentMap = null;
        mapUiSettings = null;
        trackPolyline = null;
        currentLocationMarker = null;
        synchronized (MyActivity.class) {
            if (MyActivity.mapActivityInstance == this) {
                MyActivity.mapActivityInstance = null;
            }
        }
        Log.d(TAG, "腾讯地图Activity已销毁");
    }

    @Override
    public void onRequestPermissionsResult(
        int requestCode,
        String[] permissions,
        int[] grantResults
    ) {
        super.onRequestPermissionsResult(
            requestCode,
            permissions,
            grantResults
        );
        if (requestCode == REQUEST_PERMISSIONS_REQUEST_CODE) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            if (allGranted) {
                Log.d(TAG, "所有权限已授予");
            } else {
                Log.w(TAG, "部分权限未授予");
                if (topInfoLabel != null) {
                    topInfoLabel.setText("部分权限未授予，地图功能可能受限");
                }
            }
        }
    }

    /**
     * WGS84（GPS标准坐标）转GCJ02（腾讯地图/高德地图适配坐标）
     * @param wgsLat GPS纬度
     * @param wgsLng GPS经度
     * @return 转换后的GCJ02坐标（LatLng）
     */
    private LatLng wgs84ToGcj02(double wgsLat, double wgsLng) {
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
     * 判断坐标是否在中国境外（境外坐标不转换，直接返回原坐标）
     */
    private boolean outOfChina(double lat, double lng) {
        return !(lng > 73.66 && lng < 135.05 && lat > 3.86 && lat < 53.55);
    }

    /**
     * 纬度转换辅助计算
     */
    private double transformLat(double x, double y) {
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
    private double transformLng(double x, double y) {
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
