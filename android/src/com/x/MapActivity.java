package com.x;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.Activity;
import android.util.Log;
import android.Manifest;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.view.Window;
import org.osmdroid.views.overlay.Marker;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.osmdroid.config.Configuration;
import org.osmdroid.tileprovider.tilesource.ITileSource;
import org.osmdroid.tileprovider.tilesource.XYTileSource;
import org.osmdroid.util.GeoPoint;
import org.osmdroid.views.MapView;
import org.osmdroid.views.overlay.Polyline;
import org.osmdroid.api.IMapController;
import org.osmdroid.events.MapListener;
import org.osmdroid.events.ScrollEvent;
import org.osmdroid.events.ZoomEvent;

public class MapActivity extends Activity {
    private static final String TAG = "OSM_Final";
    private static final String THUNDERFOREST_API_KEY = "5ad09b54d1e542909f4f20f3a01786ae"; // 确保此API密钥有效
    private static final double OSLO_LATITUDE = 59.9139;
    private static final double OSLO_LONGITUDE = 10.7522;
    private static final String USER_AGENT = "Knot/1.0 (Android; com.x; " + android.os.Build.VERSION.RELEASE + ")";
    private static final int REQUEST_PERMISSIONS_REQUEST_CODE = 1;

    private MapView osmMapView;
    private IMapController osmController;
    private Polyline osmPolyline;
    private List<GeoPoint> osmTrackPoints = new ArrayList<>();
    private TextView topInfoLabel;
    private TextView bottomInfoLabel;
    private Button switchMapBtn;
    private boolean usingThunderforest = true; // 跟踪当前使用的瓦片源

    private Marker currentLocationMarker;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MyActivity.mapActivityInstance = this;

        // 去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        // 初始化OSM配置 - 这必须在setContentView之前
        initOsmGlobalConfig();

        setContentView(R.layout.activity_map);

        // 检查并请求必要的权限
        checkPermissions();

        initViews();
        initOsmMap();
        initCenterMarker();
    }

    private void initOsmGlobalConfig() {
        try {
            // 正确设置全局配置
            Configuration.getInstance().load(this, getPreferences(MODE_PRIVATE));
            Configuration.getInstance().setUserAgentValue(USER_AGENT);

            File basePath = getExternalFilesDir(null);
            if (basePath != null) {
                Configuration.getInstance().setOsmdroidBasePath(basePath);

                File tileCache = new File(basePath, "tiles");
                if (!tileCache.exists()) {
                    tileCache.mkdirs();
                }
                Configuration.getInstance().setOsmdroidTileCache(tileCache);
            }

            Configuration.getInstance().setDebugMode(true);
            Log.d(TAG, "OSM配置初始化完成");
        } catch (Exception e) {
            Log.e(TAG, "OSM配置初始化失败", e);
        }
    }

    private void initViews() {
        topInfoLabel = findViewById(R.id.topInfoLabel);
        bottomInfoLabel = findViewById(R.id.bottomInfoLabel);
        switchMapBtn = findViewById(R.id.switchMapBtn);
        osmMapView = findViewById(R.id.osmMapView); // 提前获取MapView引用

        topInfoLabel.setText("加载中...Thunderforest");
        bottomInfoLabel.setText("距离: 0.0km | 速度: 0.0km/h");

        switchMapBtn.setOnClickListener(v -> {
            switchMapSource();
        });
    }

    private void switchMapSource() {
        clearOsmCache();
        usingThunderforest = !usingThunderforest;

        if (usingThunderforest) {
            setupThunderforestTiles();
            topInfoLabel.setText("切换到Thunderforest，加载中...");
        } else {
            setupOpenStreetMapTiles();
            topInfoLabel.setText("切换到OpenStreetMap，加载中...");
        }

        osmMapView.invalidate();
    }

    private void initOsmMap() {
        try {
            osmMapView.setMultiTouchControls(true);
            osmMapView.setTilesScaledToDpi(true);

            // 初始化地图控制器和默认中心
            osmController = osmMapView.getController();
            GeoPoint osloCenter = new GeoPoint(OSLO_LATITUDE, OSLO_LONGITUDE);
            osmController.setCenter(osloCenter);
            osmController.setZoom(13);

            // 初始化轨迹线
            osmPolyline = new Polyline();
            osmPolyline.setColor(0xFFFF0000); // 红色轨迹
            osmPolyline.setWidth(5f);
            osmMapView.getOverlays().add(osmPolyline);

            // 地图监听（保留原有逻辑）
            osmMapView.setMapListener(new MapListener() {
                @Override
                public boolean onScroll(ScrollEvent event) {
                    org.osmdroid.api.IGeoPoint center = osmMapView.getMapCenter();
                    topInfoLabel.setText(String.format(
                            "纬度:%.4f 经度:%.4f",
                            center.getLatitude(), center.getLongitude()));
                    // 用isEnabled()判断是否可见
                    if (currentLocationMarker != null && currentLocationMarker.isEnabled()) {
                        currentLocationMarker.setPosition(new GeoPoint(center.getLatitude(), center.getLongitude()));
                    }
                    return false;
                }

                @Override
                public boolean onZoom(ZoomEvent event) {
                    topInfoLabel.setText(String.format("缩放级:%d", (int) event.getZoomLevel()));
                    return false;
                }
            });

            setupThunderforestTiles();

            // 地图初始化完成后，刷新一次覆盖层
            osmMapView.invalidate();
            Log.d(TAG, "地图初始化完成，覆盖层已刷新");

        } catch (Exception e) {
            Log.e(TAG, "地图初始化失败", e);
            topInfoLabel.setText("地图初始化失败，尝试备用方案");
            setupOpenStreetMapTiles();
        }
    }

    private void setupThunderforestTiles() {
        try {
            ITileSource thunderforestTile = new XYTileSource(
                    "Thunderforest_Cycle",
                    1, 18, 256, ".png",
                    new String[] {
                            "https://a.tile.thunderforest.com/cycle/",
                            "https://b.tile.thunderforest.com/cycle/",
                            "https://c.tile.thunderforest.com/cycle/"
                    }) {
                @Override
                public String getTileURLString(long pMapTileIndex) {
                    String url = super.getTileURLString(pMapTileIndex);
                    // 附加API密钥
                    return url + "?apikey=" + THUNDERFOREST_API_KEY;
                }
            };

            osmMapView.setTileSource(thunderforestTile);
            topInfoLabel.setText("地图加载完成 - Thunderforest");
            usingThunderforest = true;

        } catch (Exception e) {
            Log.e(TAG, "Thunderforest TileSource失败", e);
            topInfoLabel.setText("Thunderforest加载失败，切换到OpenStreetMap");
            setupOpenStreetMapTiles();
        }
    }

    private void setupOpenStreetMapTiles() {
        try {
            ITileSource osmTileSource = new XYTileSource(
                    "OpenStreetMap",
                    1, 18, 256, ".png",
                    new String[] {
                            "https://a.tile.openstreetmap.org/",
                            "https://b.tile.openstreetmap.org/",
                            "https://c.tile.openstreetmap.org/"
                    });

            osmMapView.setTileSource(osmTileSource);
            topInfoLabel.setText("使用OpenStreetMap瓦片");
            usingThunderforest = false;

        } catch (Exception ex) {
            Log.e(TAG, "OpenStreetMap也失败", ex);
            topInfoLabel.setText("地图加载失败，请检查网络连接");
        }
    }

    // 权限检查与请求
    private void checkPermissions() {
        List<String> permissionsNeeded = new ArrayList<>();

        final String[] permissions = new String[] {
                Manifest.permission.INTERNET,
                Manifest.permission.ACCESS_NETWORK_STATE,
                // Android 10+ 不需要WRITE_EXTERNAL_STORAGE权限
                Manifest.permission.WRITE_EXTERNAL_STORAGE
        };

        for (String permission : permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                permissionsNeeded.add(permission);
            }
        }

        if (!permissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(
                    this,
                    permissionsNeeded.toArray(new String[0]),
                    REQUEST_PERMISSIONS_REQUEST_CODE);
        }
    }

    private void clearOsmCache() {
        try {
            // 清除内存缓存
            if (osmMapView != null && osmMapView.getTileProvider() != null) {
                osmMapView.getTileProvider().clearTileCache();
            }

            // 清除磁盘缓存
            File cacheDir = Configuration.getInstance().getOsmdroidTileCache();
            if (cacheDir != null && cacheDir.exists()) {
                deleteDir(cacheDir);
                cacheDir.mkdirs();
            }

            Log.d(TAG, "已清除瓦片缓存");

        } catch (Exception e) {
            Log.e(TAG, "清除缓存失败", e);
        }
    }

    private boolean deleteDir(File dir) {
        if (dir == null)
            return false;
        if (dir.isDirectory()) {
            File[] files = dir.listFiles();
            if (files != null) {
                for (File file : files) {
                    if (!deleteDir(file))
                        return false;
                }
            }
        }
        return dir.delete();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
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
                if (osmMapView != null) {
                    osmMapView.invalidate();
                }
            } else {
                Log.w(TAG, "部分权限未授予，可能影响地图功能");
                topInfoLabel.setText("部分权限未授予，地图可能无法正常显示");
            }
        }
    }

    private void initCenterMarker() {
        if (osmMapView == null) {
            Log.e(TAG, "地图未初始化，无法创建中心点标识");
            return;
        }

        currentLocationMarker = new Marker(osmMapView);
        // 引用自定义红色圆点图标（关键修改）
        currentLocationMarker.setIcon(ContextCompat.getDrawable(this, R.drawable.marker_center));
        // 锚点设置为图标中心，确保圆点中心与经纬度完全对齐
        currentLocationMarker.setAnchor(Marker.ANCHOR_CENTER, Marker.ANCHOR_CENTER);
        // 初始隐藏，同时设置初始位置（避免位置为空）
        currentLocationMarker.setPosition(new GeoPoint(OSLO_LATITUDE, OSLO_LONGITUDE));
        currentLocationMarker.setEnabled(false);

        // 关键：调整覆盖层顺序，将标识放在最上层（避免被轨迹线遮挡）
        osmMapView.getOverlays().add(currentLocationMarker);
        // 移除后重新添加，确保在所有覆盖层最上方
        osmMapView.getOverlays().remove(currentLocationMarker);
        osmMapView.getOverlays().add(currentLocationMarker);

        Log.d(TAG, "中心点标识初始化完成，初始位置：奥斯陆");
    }

    /**
     * 追加轨迹点的接口方法（每次追加都将地图中心定位到当前点）
     * 
     * @param latitude  纬度（合法范围：-90.0 ~ 90.0）
     * @param longitude 经度（合法范围：-180.0 ~ 180.0）
     */
    public void appendTrackPoint(double latitude, double longitude) {
        // 1. 基础合法性校验
        if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
            Log.w(TAG, "非法经纬度，跳过该轨迹点 | 纬度：" + latitude + "，经度：" + longitude);
            return;
        }
        // 增加 currentLocationMarker 的空值判断（关键）
        if (osmController == null || osmMapView == null || osmPolyline == null || currentLocationMarker == null) {
            Log.e(TAG, "地图对象未初始化，无法追加轨迹点");
            return;
        }

        runOnUiThread(() -> {
            try {
                GeoPoint newPoint = new GeoPoint(latitude, longitude);
                osmTrackPoints.add(newPoint);
                osmPolyline.setPoints(osmTrackPoints);

                // 移动地图中心并更新标识
                osmController.setCenter(newPoint);
                currentLocationMarker.setPosition(newPoint);
                currentLocationMarker.setEnabled(true); // 显示标识
                Log.d(TAG, "标识已更新到新轨迹点 | 可见性：" + currentLocationMarker.isEnabled());

                // 强制刷新地图，确保标识立即显示
                osmMapView.invalidate();
                Log.d(TAG, "轨迹点追加成功 | 总点数：" + osmTrackPoints.size());
            } catch (Exception e) {
                Log.e(TAG, "追加轨迹点异常", e);
            }
        });
    }

    /**
     * 清除所有轨迹数据（辅助方法）
     */
    public void clearTrack() {
        // 先判断核心对象是否为空
        if (osmMapView == null || osmPolyline == null) {
            Log.e(TAG, "地图对象未初始化或已销毁，无法清除轨迹");
            return;
        }

        runOnUiThread(() -> {
            osmTrackPoints.clear();
            osmPolyline.setPoints(osmTrackPoints);
            currentLocationMarker.setEnabled(false);
            osmMapView.invalidate();
            Log.d(TAG, "轨迹数据已清空");
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (osmMapView != null) {
            osmMapView.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (osmMapView != null) {
            osmMapView.onPause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (osmMapView != null) {
            osmMapView.onDetach();
            if (osmMapView.getTileProvider() != null) {
                osmMapView.getTileProvider().clearTileCache();
            }
        }

        if (currentLocationMarker != null) {
            osmMapView.getOverlays().remove(currentLocationMarker);
            currentLocationMarker = null;
        }

        // 释放地图资源（避免内存泄漏）
        if (osmMapView != null) {
            osmMapView.onDetach();
            osmMapView.getTileProvider().clearTileCache();
            osmMapView = null; // 置空，避免后续误访问
        }
        osmController = null; // 置空核心对象
        osmPolyline = null;
        osmTrackPoints.clear();

        if (MyActivity.mapActivityInstance == this) {
            MyActivity.mapActivityInstance = null;
        }
    }
}