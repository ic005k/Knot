package com.x;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Color;
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
import android.widget.TextView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import org.osmdroid.api.IMapController;
import org.osmdroid.config.Configuration;
import org.osmdroid.events.MapListener;
import org.osmdroid.events.ScrollEvent;
import org.osmdroid.events.ZoomEvent;
import org.osmdroid.tileprovider.tilesource.ITileSource;
import org.osmdroid.tileprovider.tilesource.XYTileSource;
import org.osmdroid.util.GeoPoint;
import org.osmdroid.views.MapView;
import org.osmdroid.views.overlay.Marker;
import org.osmdroid.views.overlay.Polyline;

public class MapActivity extends Activity {

    // 直接定义两个Thunderforest图层的URL数组（核心简化点）
    private static final String[] TILE_URLS_CYCLE = {
        "https://a.tile.thunderforest.com/cycle/",
        "https://b.tile.thunderforest.com/cycle/",
        "https://c.tile.thunderforest.com/cycle/",
    };
    private static final String[] TILE_URLS_OUTDOORS = {
        "https://a.tile.thunderforest.com/outdoors/",
        "https://b.tile.thunderforest.com/outdoors/",
        "https://c.tile.thunderforest.com/outdoors/",
    };

    private static final String TAG = "OSM_Final";
    private static final String THUNDERFOREST_API_KEY =
        "5ad09b54d1e542909f4f20f3a01786ae"; // 确保此API密钥有效
    private static final double OSLO_LATITUDE = 59.9139;
    private static final double OSLO_LONGITUDE = 10.7522;
    private static final String USER_AGENT =
        "Knot/1.0 (Android; com.x; " + android.os.Build.VERSION.RELEASE + ")";
    private static final int REQUEST_PERMISSIONS_REQUEST_CODE = 1;

    private MapView osmMapView;
    private IMapController osmController;
    private Polyline osmPolyline;
    private List<GeoPoint> osmTrackPoints = new ArrayList<>();
    private TextView topDateLabel;
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

        if (MyActivity.isDark) {
            this.setStatusBarColor("#19232D"); // 深色
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            setContentView(R.layout.noteeditor_dark);
        } else {
            this.setStatusBarColor("#F3F3F3"); // 灰
            getWindow()
                .getDecorView()
                .setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            setContentView(R.layout.noteeditor);
        }

        // 初始化OSM配置 - 这必须在setContentView之前
        initOsmGlobalConfig();

        setContentView(R.layout.activity_map);

        // 检查并请求必要的权限
        // checkPermissions();

        initViews();
        initOsmMap();
        initCenterMarker();

        // 监听地图布局完成并延迟绘制轨迹
        if (osmMapView != null) {
            ViewTreeObserver observer = osmMapView.getViewTreeObserver();
            observer.addOnGlobalLayoutListener(
                new ViewTreeObserver.OnGlobalLayoutListener() {
                    @Override
                    public void onGlobalLayout() {
                        if (
                            Build.VERSION.SDK_INT >=
                            Build.VERSION_CODES.JELLY_BEAN
                        ) {
                            osmMapView
                                .getViewTreeObserver()
                                .removeOnGlobalLayoutListener(this);
                        } else {
                            // 兼容旧版本
                            osmMapView
                                .getViewTreeObserver()
                                .removeGlobalOnLayoutListener(this);
                        }

                        // 动态计算延迟时间
                        int delay = MyActivity.osmTrackPoints.size() > 100
                            ? 50
                            : 20;
                        delay = 0; //目前取消延迟
                        new Handler(Looper.getMainLooper()).postDelayed(
                            () -> {
                                if (
                                    osmMapView != null &&
                                    !isFinishing() &&
                                    !isDestroyed()
                                ) {
                                    drawAllTrackPoints();
                                    topDateLabel.setText(MyActivity.lblDate);
                                    bottomInfoLabel.setText(MyActivity.lblInfo);
                                }
                            },
                            delay
                        );
                    }
                }
            );
        }
    }

    private void initOsmGlobalConfig() {
        try {
            // 正确设置全局配置
            Configuration.getInstance().load(
                this,
                getPreferences(MODE_PRIVATE)
            );
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
        topDateLabel = findViewById(R.id.topDateLabel);
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
            osmMapView.setMapListener(
                new MapListener() {
                    @Override
                    public boolean onScroll(ScrollEvent event) {
                        // 获取当前地图中心坐标
                        org.osmdroid.api.IGeoPoint center =
                            osmMapView.getMapCenter();
                        // 获取当前缩放级别
                        double zoomLevel = osmMapView.getZoomLevel();

                        // 同时显示缩放级、纬度、经度
                        topInfoLabel.setText(
                            String.format(
                                "Zoom: %d | Lat: %.4f | Lng: %.4f",
                                (int) zoomLevel,
                                center.getLatitude(),
                                center.getLongitude()
                            )
                        );

                        // 更新当前位置标记
                        if (
                            currentLocationMarker != null &&
                            currentLocationMarker.isEnabled()
                        ) {
                            currentLocationMarker.setPosition(
                                new GeoPoint(
                                    center.getLatitude(),
                                    center.getLongitude()
                                )
                            );
                        }
                        return false;
                    }

                    @Override
                    public boolean onZoom(ZoomEvent event) {
                        // 获取当前地图中心坐标
                        org.osmdroid.api.IGeoPoint center =
                            osmMapView.getMapCenter();
                        // 获取当前缩放级别（从事件中获取最新缩放级）
                        double zoomLevel = event.getZoomLevel();

                        // 同时显示缩放级、纬度、经度
                        topInfoLabel.setText(
                            String.format(
                                "Zoom: %d | Lat: %.4f | Lng: %.4f",
                                (int) zoomLevel,
                                center.getLatitude(),
                                center.getLongitude()
                            )
                        );
                        return false;
                    }
                }
            );

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

    private void setupThunderforestTiles_Low() {
        try {
            ITileSource thunderforestTile = new XYTileSource(
                "Thunderforest_Cycle",
                1,
                18,
                256,
                ".png",
                new String[] {
                    "https://a.tile.thunderforest.com/cycle/",
                    "https://b.tile.thunderforest.com/cycle/",
                    "https://c.tile.thunderforest.com/cycle/",
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

    private void setupThunderforestTiles() {
        try {
            // 关键修改1：后缀改为 "@2x.png"（因为要拼接成 {y}@2x.png）
            ITileSource thunderforestTile = new XYTileSource(
                "Thunderforest_Cycle_HD",
                1,
                18,
                512, // 瓦片尺寸512不变（高清核心）
                "@2x.png", // 后缀改为 "@2x.png"（原是 ".png"）
                TILE_URLS_CYCLE
            ) {
                @Override
                public String getTileURLString(long pMapTileIndex) {
                    String url = super.getTileURLString(pMapTileIndex);
                    // 关键修改2：移除原有的 ".png@2x" 替换逻辑，因为后缀已改为 "@2x.png"
                    // 此时super生成的URL已为：https://xxx/cycle/z/x/y@2x.png
                    // 直接附加API密钥即可
                    return url + "?apikey=" + THUNDERFOREST_API_KEY;
                }
            };

            osmMapView.setTileSource(thunderforestTile);
            topInfoLabel.setText("地图加载完成 - Thunderforest（高清）");
            usingThunderforest = true;
        } catch (Exception e) {
            Log.e(TAG, "Thunderforest高清瓦片加载失败", e);
            topInfoLabel.setText("高清瓦片加载失败，切换到普通瓦片");
            setupOpenStreetMapTiles();
        }
    }

    private void setupOpenStreetMapTiles() {
        try {
            ITileSource osmTileSource = new XYTileSource(
                "OpenStreetMap",
                1,
                18,
                256,
                ".png",
                new String[] {
                    "https://a.tile.openstreetmap.org/",
                    "https://b.tile.openstreetmap.org/",
                    "https://c.tile.openstreetmap.org/",
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

        final String[] permissions = new String[] {
            Manifest.permission.INTERNET,
            Manifest.permission.ACCESS_NETWORK_STATE,
            // Android 10+ 不需要WRITE_EXTERNAL_STORAGE权限
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
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
        currentLocationMarker.setIcon(
            ContextCompat.getDrawable(this, R.drawable.marker_center)
        );
        // 锚点设置为图标中心，确保圆点中心与经纬度完全对齐
        currentLocationMarker.setAnchor(
            Marker.ANCHOR_CENTER,
            Marker.ANCHOR_CENTER
        );
        // 初始隐藏，同时设置初始位置（避免位置为空）
        currentLocationMarker.setPosition(
            new GeoPoint(OSLO_LATITUDE, OSLO_LONGITUDE)
        );
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
    /*public void appendTrackPoint(double latitude, double longitude) {
        // 1. 基础合法性校验
        if (
            latitude < -90.0 ||
            latitude > 90.0 ||
            longitude < -180.0 ||
            longitude > 180.0
        ) {
            Log.w(
                TAG,
                "非法经纬度，跳过该轨迹点 | 纬度：" +
                    latitude +
                    "，经度：" +
                    longitude
            );
            return;
        }

        // 增加 currentLocationMarker 的空值判断（关键）
        // if (osmController == null || osmMapView == null || osmPolyline == null ||
        // currentLocationMarker == null) {
        // Log.e(TAG, "地图对象未初始化，无法追加轨迹点");
        // return;
        // }

        runOnUiThread(() -> {
            try {
                GeoPoint newPoint = new GeoPoint(latitude, longitude);
                osmTrackPoints.add(newPoint);
                osmPolyline.setPoints(osmTrackPoints);

                // 移动地图中心并更新标识
                osmController.setCenter(newPoint);
                currentLocationMarker.setPosition(newPoint);
                currentLocationMarker.setEnabled(true); // 显示标识
                // Log.d(TAG, "标识已更新到新轨迹点 | 可见性：" + currentLocationMarker.isEnabled());

                // 强制刷新地图，确保标识立即显示
                osmMapView.invalidate();
                // Log.d(TAG, "轨迹点追加成功 | 总点数：" + osmTrackPoints.size());
            } catch (Exception e) {
                Log.e(TAG, "追加轨迹点异常", e);
            }
        });
    }*/

    public void appendTrackPoint(double latitude, double longitude) {
        // 新增：1. 校验Activity状态（销毁则直接返回）
        if (isFinishing() || isDestroyed()) {
            Log.w(TAG, "Activity已销毁，跳过轨迹点添加");
            return;
        }

        // 2. 基础合法性校验（保留原有）
        if (
            latitude < -90.0 ||
            latitude > 90.0 ||
            longitude < -180.0 ||
            longitude > 180.0
        ) {
            Log.w(
                TAG,
                "非法经纬度，跳过该轨迹点 | 纬度：" +
                    latitude +
                    "，经度：" +
                    longitude
            );
            return;
        }

        // 新增：3. 核心对象空判（补充完整）
        if (
            osmController == null ||
            osmMapView == null ||
            osmPolyline == null ||
            currentLocationMarker == null
        ) {
            Log.e(TAG, "地图核心对象未初始化，无法追加轨迹点");
            return;
        }

        // 新增：4. 校验静态引用是否为当前有效实例
        if (MyActivity.mapActivityInstance != this) {
            Log.w(TAG, "当前实例已失效，跳过轨迹点添加");
            return;
        }

        runOnUiThread(() -> {
            // 新增：5. UI线程内再次校验（避免线程切换中Activity销毁）
            if (isFinishing() || isDestroyed() || osmMapView == null) {
                return;
            }
            try {
                GeoPoint newPoint = new GeoPoint(latitude, longitude);
                osmTrackPoints.add(newPoint);
                osmPolyline.setPoints(osmTrackPoints);

                osmController.setCenter(newPoint);
                currentLocationMarker.setPosition(newPoint);
                currentLocationMarker.setEnabled(true);
                osmMapView.invalidate();
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
    public void onBackPressed() {
        // 1. 优先清除静态引用（核心提前操作）
        synchronized (MyActivity.class) {
            if (MyActivity.mapActivityInstance == this) {
                MyActivity.mapActivityInstance = null;
                Log.d(TAG, "回退键触发，提前清除静态引用");
            }
        }

        // 2. 执行正常的回退逻辑（关闭当前Activity）
        super.onBackPressed(); // 或 finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy(); // 优先调用父类销毁方法

        // 1. 清理地图叠加层（避免视图层级引用泄漏）
        if (osmMapView != null) {
            // 移除所有叠加层（包括轨迹线、标记点）
            osmMapView.getOverlays().clear();
            // 清理瓦片缓存（释放磁盘/内存缓存，不影响渲染上下文）
            if (osmMapView.getTileProvider() != null) {
                osmMapView.getTileProvider().clearTileCache();
            }
            // 释放绘制缓存（纯Android视图资源，与Qt渲染无关）
            osmMapView.destroyDrawingCache();
            // 正常置空，无需延迟（已确认与地图渲染无关）
            osmMapView = null;
        }

        // 2. 清理轨迹数据（避免集合引用导致的内存泄漏）
        if (osmTrackPoints != null) {
            synchronized (osmTrackPoints) {
                // 确保线程安全
                osmTrackPoints.clear();
            }
            osmTrackPoints = null; // 彻底释放引用
        }

        // 3. 置空其他辅助对象（解除对象间引用）
        currentLocationMarker = null;
        osmController = null;
        osmPolyline = null;

        // 4. 解除静态引用（核心！避免Activity实例泄漏）
        synchronized (MyActivity.class) {
            if (MyActivity.mapActivityInstance == this) {
                MyActivity.mapActivityInstance = null;
            }
        }

        Log.d(TAG, "正常释放资源完成（与地图渲染无关场景）");
    }

    private void drawAllTrackPoints() {
        // 1. 校验集合非空
        if (MyActivity.osmTrackPoints.isEmpty()) {
            Log.d(TAG, "osmTrackPoints 集合为空，无需遍历");
            return;
        }

        // 2. 校验 MyActivity 实例有效（避免空指针）
        if (MyActivity.mapActivityInstance == null) {
            Log.e(TAG, "MyActivity 中 MapActivity 实例为空，无法重放轨迹");
            return;
        }

        Log.d(
            TAG,
            "开始遍历轨迹集合，总点数：" + MyActivity.osmTrackPoints.size()
        );

        // 3. 遍历集合（CopyOnWriteArrayList 遍历线程安全）
        for (GeoPoint point : MyActivity.osmTrackPoints) {
            if (point == null) {
                Log.w(TAG, "跳过空的轨迹点");
                continue;
            }

            // 提取经纬度
            double latitude = point.getLatitude();
            double longitude = point.getLongitude();

            appendTrackPoint(latitude, longitude);
        }

        Log.d(TAG, "轨迹集合遍历完成");
    }

    private void setStatusBarColor(String color) {
        // 需要安卓版本大于5.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().addFlags(
                WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS
            );
            getWindow().setStatusBarColor(Color.parseColor(color));
        }
    }

    // 新增：设置顶部日期文本的公开接口（父类统一定义）
    public void setTopDate(String text) {
        if (topDateLabel != null) {
            runOnUiThread(() -> topDateLabel.setText(text)); // 确保UI线程操作
        }
    }

    // 新增：设置底部信息文本的公开接口（父类统一定义）
    public void setBottomInfo(String text) {
        if (bottomInfoLabel != null) {
            runOnUiThread(() -> bottomInfoLabel.setText(text)); // 确保UI线程操作
        }
    }
}
