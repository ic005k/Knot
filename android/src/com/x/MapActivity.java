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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // 去除title(App Name)
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        // 初始化OSM配置 - 这必须在setContentView之前
        initOsmGlobalConfig();
        
        setContentView(R.layout.activity_map);
        
        // 检查并请求必要的权限
        checkPermissions();
        
        initViews();
        initOsmMap();
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
            // 基础地图设置
            osmMapView.setMultiTouchControls(true);
            osmMapView.setTilesScaledToDpi(true);

            // 定位到奥斯陆
            osmController = osmMapView.getController();
            GeoPoint osloCenter = new GeoPoint(OSLO_LATITUDE, OSLO_LONGITUDE);
            osmController.setCenter(osloCenter);
            osmController.setZoom(13);

            // 初始化轨迹线
            osmPolyline = new Polyline();
            osmPolyline.setColor(0xFFFF0000); // 红色轨迹
            osmPolyline.setWidth(5f);        // 轨迹线宽度
            osmPolyline.setPoints(osmTrackPoints); // 设置点列表
            osmMapView.getOverlays().add(osmPolyline);

            // 地图状态监听
            osmMapView.setMapListener(new MapListener() {
                @Override
                public boolean onScroll(ScrollEvent event) {
                    org.osmdroid.api.IGeoPoint center = osmMapView.getMapCenter();
                    topInfoLabel.setText(String.format(
                        "纬度:%.4f 经度:%.4f",
                        center.getLatitude(), center.getLongitude()
                    ));
                    return false;
                }

                @Override
                public boolean onZoom(ZoomEvent event) {
                    topInfoLabel.setText(String.format("缩放级:%d", (int) event.getZoomLevel()));
                    return false;
                }
            });
            
            // 尝试加载Thunderforest瓦片
            setupThunderforestTiles();
            
        } catch (Exception e) {
            Log.e(TAG, "地图初始化失败", e);
            topInfoLabel.setText("地图初始化失败，尝试备用方案");
            
            // 备用方案：使用OpenStreetMap
            setupOpenStreetMapTiles();
        }
    }

    private void setupThunderforestTiles() {
        try {
            ITileSource thunderforestTile = new XYTileSource(
                "Thunderforest_Cycle",
                1, 18, 256, ".png",
                new String[]{
                    "https://a.tile.thunderforest.com/cycle/",
                    "https://b.tile.thunderforest.com/cycle/", 
                    "https://c.tile.thunderforest.com/cycle/"
                }
            ) {
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
                new String[]{
                    "https://a.tile.openstreetmap.org/",
                    "https://b.tile.openstreetmap.org/",
                    "https://c.tile.openstreetmap.org/"
                }
            );
            
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

        final String[] permissions = new String[]{
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
                    REQUEST_PERMISSIONS_REQUEST_CODE
            );
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
        if (dir == null) return false;
        if (dir.isDirectory()) {
            File[] files = dir.listFiles();
            if (files != null) {
                for (File file : files) {
                    if (!deleteDir(file)) return false;
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

    /**
     * 【核心修改】追加轨迹点的接口方法
     * 直接接收纬度和经度参数，内部自动转换为GeoPoint并更新轨迹
     * @param latitude 纬度（合法范围：-90.0 ~ 90.0）
     * @param longitude 经度（合法范围：-180.0 ~ 180.0）
     */
    public void appendTrackPoint(double latitude, double longitude) {
        // 1. 校验经纬度合法性，过滤无效数据
        if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
            Log.w(TAG, "非法经纬度，跳过该轨迹点 | 纬度：" + latitude + "，经度：" + longitude);
            return;
        }

        // 2. 地图操作需在UI线程执行，避免线程安全问题
        runOnUiThread(() -> {
            try {
                // 3. 经纬度转OSM所需的GeoPoint对象
                GeoPoint newPoint = new GeoPoint(latitude, longitude);
                // 4. 追加到轨迹点列表
                osmTrackPoints.add(newPoint);
                // 5. 更新轨迹线的点集，刷新显示
                osmPolyline.setPoints(osmTrackPoints);
                // 6. 第一个点时自动定位到该位置
                if (osmTrackPoints.size() == 1) {
                    osmController.setCenter(newPoint);
                    Log.d(TAG, "首次轨迹点定位 | 纬度：" + latitude + "，经度：" + longitude);
                }
                // 7. 刷新地图，确保轨迹实时生效
                osmMapView.invalidate();
                // 8. 日志记录轨迹点数量，便于调试
                Log.d(TAG, "轨迹点追加成功 | 当前总点数：" + osmTrackPoints.size());
            } catch (Exception e) {
                Log.e(TAG, "追加轨迹点异常", e);
            }
        });
    }

    /**
     * 清除所有轨迹数据（辅助方法）
     */
    public void clearTrack() {
        runOnUiThread(() -> {
            osmTrackPoints.clear();
            osmPolyline.setPoints(osmTrackPoints);
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
    }
}