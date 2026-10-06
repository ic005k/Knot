package com.x.artifex.mupdf.mini;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.widget.Scroller;
import com.artifex.mupdf.fitz.*;
import com.x.R;

public class PageView
    extends View
    implements
        GestureDetector.OnGestureListener,
        ScaleGestureDetector.OnScaleGestureListener
{

    private final String APP = "MuPDF";

    // ✅ 水平手势翻页阈值（仅 Reflowable 生效）
    private static final float H_FLING_VELOCITY_DP = 150f; // 速度门槛减半
    private static final float H_FLING_DIRECTION_RATIO = 1.2f; // 方向容忍度放宽
    private static final float H_FLING_DISTANCE_DP = 80f; // 最小水平滑动距离

    // ✅ 翻页页码提示 UI
    private boolean showPageTurnHint = false;
    private String pageTurnHintText = "";
    private final Paint hintBgPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint hintTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private static final float HINT_TEXT_SIZE_SP = 16f;
    private static final int HINT_BG_COLOR = 0xB3000000;
    private static final int HINT_TEXT_COLOR = 0xFFFFFFFF;
    private final Runnable hideHintRunnable = () -> {
        if (showPageTurnHint) {
            showPageTurnHint = false;
            invalidate();
        }
    };

    private static final int SCROLL_EPSILON = 2; // 容差像素
    protected int savedScrollX = 0; // 当前阅读水平位置，翻页时跨页恢复

    // TTS 高亮
    private Quad[] ttsQuads = null;
    private final Paint ttsPaint = new Paint();
    // ✅ 页面版本号，每次 setBitmap 递增
    private int pageVersion = 0;
    private int ttsHighlightVersion = -1; // 高亮对应的页面版本

    // ✅ 一次性回调：下一次 setBitmap 完成后触发
    private Runnable mOnNextBitmapReady = null;

    /**
     * 注册一次性回调：下一次 setBitmap 完成（bitmap 已渲染上屏）后触发
     * 用于确保 relayout 完成后再执行笔记定位搜索
     */
    public void setOnNextBitmapReady(Runnable callback) {
        mOnNextBitmapReady = callback;
    }

    // ✅ 边缘防误触安全区（像素），此区域内长按不触发文本选择
    // 24dp ≈ 48px @2x，36dp ≈ 72px @2x
    private int edgeSafeZonePx;

    // 文字选择相关
    private Quad[] selectQuads;
    private boolean isSelectingText;
    private float selectStartX, selectStartY;
    private float selectEndX, selectEndY;

    // 初始化
    {
        ttsPaint.setColor(0x66FF9800); // 半透明橙色
        ttsPaint.setStyle(Paint.Style.FILL);
        ttsPaint.setAntiAlias(true);
    }

    protected DocumentActivity actionListener;

    protected float pageScale, viewScale, minScale, maxScale;
    protected Bitmap bitmap;
    protected int bitmapW, bitmapH;
    protected int canvasW, canvasH;
    protected int scrollX, scrollY;
    protected Rect[] linkBounds;
    protected String[] linkURIs;
    protected Quad[][] hits;
    protected boolean showLinks;

    protected GestureDetector detector;
    protected ScaleGestureDetector scaleDetector;
    protected Scroller scroller;
    protected boolean error;
    protected Paint errorPaint;
    protected Path errorPath;
    protected Paint linkPaint;
    protected Paint hitPaint;

    public PageView(Context ctx, AttributeSet atts) {
        super(ctx, atts);

        // 24dp 作为默认值，横屏下足够覆盖手掌边缘
        float density = ctx.getResources().getDisplayMetrics().density;
        edgeSafeZonePx = (int) (24 * density + 0.5f);

        scroller = new Scroller(ctx);
        detector = new GestureDetector(ctx, this);
        scaleDetector = new ScaleGestureDetector(ctx, this);

        pageScale = 1;
        viewScale = 1;
        minScale = 1;
        maxScale = 8;

        linkPaint = new Paint();
        linkPaint.setARGB(32, 0, 0, 255);

        hitPaint = new Paint();
        hitPaint.setARGB(32, 255, 0, 0);
        hitPaint.setStyle(Paint.Style.FILL);

        errorPaint = new Paint();
        errorPaint.setARGB(255, 255, 80, 80);
        errorPaint.setStrokeWidth(5);
        errorPaint.setStyle(Paint.Style.STROKE);

        errorPath = new Path();
        errorPath.moveTo(-100, -100);
        errorPath.lineTo(100, 100);
        errorPath.moveTo(100, -100);
        errorPath.lineTo(-100, 100);

        // 翻页指示器
        hintBgPaint.setColor(HINT_BG_COLOR);
        hintBgPaint.setStyle(Paint.Style.FILL);

        hintTextPaint.setColor(HINT_TEXT_COLOR);
        hintTextPaint.setTextSize(HINT_TEXT_SIZE_SP * density);
        hintTextPaint.setTextAlign(Paint.Align.CENTER);
    }

    public void setActionListener(DocumentActivity l) {
        actionListener = l;
    }

    public synchronized void setError() {
        if (bitmap != null) bitmap.recycle();
        error = true;
        linkBounds = new Rect[0];
        linkURIs = new String[0];
        hits = null;
        bitmap = null;
        scroller.forceFinished(true);
        invalidate();
    }

    public synchronized void setBitmap(
        Bitmap b,
        float zoom,
        boolean wentBack,
        boolean toggledUI,
        boolean scrollToFirstSearchHit,
        Rect[] lbs,
        String[] lus,
        Quad[][] hs
    ) {
        if (bitmap != null) bitmap.recycle();
        error = false;

        // ✅ 递增版本号，使旧高亮自动失效
        pageVersion++;
        ttsQuads = null;
        ttsHighlightVersion = -1;

        linkBounds = lbs;
        linkURIs = lus;
        hits = hs;
        bitmap = b;
        bitmapW = (int) ((bitmap.getWidth() * viewScale) / zoom);
        bitmapH = (int) ((bitmap.getHeight() * viewScale) / zoom);
        scroller.forceFinished(true);
        if (scrollToFirstSearchHit && hits != null) {
            float top = bitmapH;
            float left = bitmapW;

            for (Quad[] hit : hits) {
                for (Quad q : hit) {
                    if (q.ul_x * viewScale < left) left = q.ul_x * viewScale;
                    if (q.ll_x * viewScale < left) left = q.ll_x * viewScale;
                    if (q.lr_x * viewScale < left) left = q.lr_x * viewScale;
                    if (q.ur_x * viewScale < left) left = q.ur_x * viewScale;

                    if (q.ul_y * viewScale < top) top = q.ul_y * viewScale;
                    if (q.ll_y * viewScale < top) top = q.ll_y * viewScale;
                    if (q.lr_y * viewScale < top) top = q.lr_y * viewScale;
                    if (q.ur_y * viewScale < top) top = q.ur_y * viewScale;
                }
            }

            scrollX = (int) (left + 0.5f) - canvasW / 2;
            scrollY = (int) (top + 0.5f) - canvasH / 2;

            if (scrollX < 0) scrollX = 0;
            if (scrollY < 0) scrollY = 0;
            if (scrollX > bitmapW - canvasW) scrollX = bitmapW - canvasW;

            //if (scrollY > bitmapW - canvasW) scrollY = bitmapW - canvasW;
            if (scrollY > bitmapH - canvasH) scrollY = bitmapH - canvasH; // ✅

            savedScrollX = scrollX; // ✅ 搜索位置也成为新的记忆基准
        } else if (!toggledUI && pageScale == zoom) {
            // scrollX = wentBack ? bitmapW - canvasW : 0;
            // scrollY = wentBack ? bitmapH - canvasH : 0;
            // ✅ 正常翻页：无条件恢复 savedScrollX，钳位到新页面合法范围
            scrollY = wentBack ? Math.max(0, bitmapH - canvasH) : 0;
            int maxScrollX = Math.max(0, bitmapW - canvasW);
            scrollX = Math.max(0, Math.min(savedScrollX, maxScrollX));
        }
        pageScale = zoom;
        invalidate();

        // ✅ 在 setBitmap 最后、invalidate 之后触发回调
        // 此时新字号的 bitmap 已经设置完毕，pageVersion 已递增
        if (mOnNextBitmapReady != null) {
            Runnable cb = mOnNextBitmapReady;
            mOnNextBitmapReady = null; // 一次性消费
            cb.run();
        }
    }

    public void resetHits() {
        hits = null;
        invalidate();
    }

    public void onSizeChanged(int w, int h, int ow, int oh) {
        canvasW = w;
        canvasH = h;
        if (actionListener != null) actionListener.onPageViewSizeChanged(w, h);
    }

    public boolean onTouchEvent(MotionEvent event) {
        detector.onTouchEvent(event);
        scaleDetector.onTouchEvent(event);

        return true;
    }

    public boolean onDown(MotionEvent e) {
        scroller.forceFinished(true);
        return true;
    }

    public void onShowPress(MotionEvent e) {}

    /*public void onLongPress(MotionEvent e) {
        showLinks = !showLinks;
        invalidate();
    }*/

    @Override
    public void onLongPress(MotionEvent e) {
        if (bitmap == null) return;

        float touchX = e.getX();
        float touchY = e.getY();

        // ✅ 排除屏幕边缘区域，防止手掌误触触发读书笔记弹窗
        if (
            touchX < edgeSafeZonePx ||
            touchX > canvasW - edgeSafeZonePx ||
            touchY < edgeSafeZonePx ||
            touchY > canvasH - edgeSafeZonePx
        ) {
            Log.d(APP, "Long press ignored: edge safe zone");
            return;
        }

        // ✅ 不再自己做坐标转换，把原始数据和必要参数都传出去
        if (actionListener != null) {
            actionListener.onLongPressSelectText(
                touchX,
                touchY,
                scrollX,
                scrollY,
                bitmapW,
                bitmapH,
                canvasW,
                canvasH,
                viewScale,
                pageScale,
                this
            );
        }
    }

    public boolean onSingleTapUp(MotionEvent e) {
        // ✅ 单击时清除文字选区
        if (isSelectingText) {
            clearSelection();
        }

        float x = e.getX();
        float y = e.getY();

        // ✅ 新增：边缘防误触检测（与 onLongPress 保持一致）
        if (
            x < edgeSafeZonePx ||
            x > canvasW - edgeSafeZonePx ||
            y < edgeSafeZonePx ||
            y > canvasH - edgeSafeZonePx
        ) {
            Log.d(APP, "Single tap ignored: edge safe zone");
            return true; // 消费事件，但不执行后续翻页/UI切换
        }

        boolean foundLink = false;
        if (showLinks && linkBounds != null) {
            float dx = bitmapW <= canvasW ? (bitmapW - canvasW) / 2 : scrollX;
            float dy = bitmapH <= canvasH ? (bitmapH - canvasH) / 2 : scrollY;
            float mx = (x + dx) / viewScale;
            float my = (y + dy) / viewScale;
            for (int i = 0; i < linkBounds.length; i++) {
                Rect b = linkBounds[i];
                if (mx >= b.x0 && mx <= b.x1 && my >= b.y0 && my <= b.y1) {
                    if (
                        Link.isExternal(linkURIs[i]) && actionListener != null
                    ) actionListener.gotoURI(linkURIs[i]);
                    else if (actionListener != null) actionListener.gotoPage(
                        linkURIs[i]
                    );
                    foundLink = true;
                    break;
                }
            }
        }
        if (!foundLink) {
            float a = canvasW / 3;
            float b = a * 2;
            if (x <= a) goBackward();
            if (x >= b) goForward();
            if (
                x > a && x < b && actionListener != null
            ) actionListener.toggleUI();
        }
        invalidate();
        return true;
    }

    public synchronized boolean onScroll(
        MotionEvent e1,
        MotionEvent e2,
        float dx,
        float dy
    ) {
        if (bitmap != null) {
            scrollX += (int) dx;
            scrollY += (int) dy;
            scroller.forceFinished(true);
            invalidate();
        }
        return true;
    }

    @Override
    public synchronized boolean onFling(
        MotionEvent e1,
        MotionEvent e2,
        float velocityX,
        float velocityY
    ) {
        if (bitmap == null || e1 == null || e2 == null) return false;

        // ✅ 仅在未缩放时允许水平手势翻页
        // 放大后水平滑动应走下方的 scroller.fling 平移逻辑
        boolean isZoomed = viewScale > minScale + 0.01f;

        // ✅ 边缘防误触：fling 起点在屏幕左/右边缘安全区内时，
        // 视为系统手势残留，不触发翻页
        boolean startInEdgeZone =
            e1.getX() < edgeSafeZonePx || e1.getX() > canvasW - edgeSafeZonePx;

        if (
            !isZoomed &&
            !startInEdgeZone &&
            actionListener != null &&
            actionListener.isReflowable
        ) {
            float absVx = Math.abs(velocityX);
            float absVy = Math.abs(velocityY);
            float density = getResources().getDisplayMetrics().density;
            float deltaX = e2.getX() - e1.getX();
            float absDeltaX = Math.abs(deltaX);

            boolean fastEnough =
                absVx > H_FLING_VELOCITY_DP * density &&
                absVx > absVy * H_FLING_DIRECTION_RATIO;
            boolean farEnough =
                absDeltaX > H_FLING_DISTANCE_DP * density &&
                absDeltaX >
                    Math.abs(e2.getY() - e1.getY()) * H_FLING_DIRECTION_RATIO;

            if (fastEnough || farEnough) {
                scroller.forceFinished(true);

                if (actionListener != null) {
                    // ✅ （转为 1-based 显示值）
                    int targetPage =
                        deltaX < 0
                            ? actionListener.currentPage + 2 // (current+1) 是下一页的0-based索引，再+1用于显示
                            : actionListener.currentPage; // (current-1)+1 = current，刚好抵消
                    targetPage = Math.max(
                        1,
                        Math.min(targetPage, actionListener.pageCount)
                    );

                    pageTurnHintText = String.valueOf(targetPage);
                    showPageTurnHint = true;

                    // ✅  600ms 后自动消失
                    removeCallbacks(hideHintRunnable);
                    postDelayed(hideHintRunnable, 600);

                    invalidate();

                    // ✅ 不再延迟翻页，立即执行
                    if (deltaX < 0) actionListener.goForward();
                    else actionListener.goBackward();
                }
                return true;
            }
        }

        // ===== 原有垂直滚动逻辑不变 =====
        int maxX = bitmapW > canvasW ? bitmapW - canvasW : 0;
        int maxY = bitmapH > canvasH ? bitmapH - canvasH : 0;
        scroller.forceFinished(true);
        scroller.fling(
            scrollX,
            scrollY,
            (int) -velocityX,
            (int) -velocityY,
            0,
            maxX,
            0,
            maxY
        );
        invalidate();
        return true;
    }

    public boolean onScaleBegin(ScaleGestureDetector det) {
        return true;
    }

    public synchronized boolean onScale(ScaleGestureDetector det) {
        if (bitmap != null) {
            float focusX = det.getFocusX();
            float focusY = det.getFocusY();
            float scaleFactor = det.getScaleFactor();
            float pageFocusX = (focusX + scrollX) / viewScale;
            float pageFocusY = (focusY + scrollY) / viewScale;
            viewScale *= scaleFactor;
            if (viewScale < minScale) viewScale = minScale;
            if (viewScale > maxScale) viewScale = maxScale;
            bitmapW = (int) ((bitmap.getWidth() * viewScale) / pageScale);
            bitmapH = (int) ((bitmap.getHeight() * viewScale) / pageScale);
            scrollX = (int) (pageFocusX * viewScale - focusX);
            scrollY = (int) (pageFocusY * viewScale - focusY);
            scroller.forceFinished(true);
            invalidate();
        }
        return true;
    }

    public void onScaleEnd(ScaleGestureDetector det) {
        if (actionListener != null) actionListener.onPageViewZoomChanged(
            viewScale
        );
    }

    public void goBackward() {
        scroller.forceFinished(true);

        // ✅ 仅钳位 Y，X 由 onDraw 自行管理，翻页逻辑不碰它
        int maxScrollY = Math.max(0, bitmapH - canvasH);
        scrollY = Math.max(0, Math.min(scrollY, maxScrollY));

        if (scrollY <= 0) {
            savedScrollX = scrollX; // ✅ 无条件快照
            if (actionListener != null) actionListener.goBackward();
            return;
        }

        int dy = (-canvasH * 9) / 10;
        scroller.startScroll(scrollX, scrollY, 0, dy, 250);
        invalidate();
    }

    public void goForward() {
        scroller.forceFinished(true);

        // ✅ 仅钳位 Y，X 由 onDraw 自行管理，翻页逻辑不碰它
        int maxScrollY = Math.max(0, bitmapH - canvasH);
        scrollY = Math.max(0, Math.min(scrollY, maxScrollY));

        if (scrollY + canvasH >= bitmapH) {
            savedScrollX = scrollX; // ✅ 无条件快照
            if (actionListener != null) actionListener.goForward();
            return;
        }

        int dy = (canvasH * 9) / 10;
        scroller.startScroll(scrollX, scrollY, 0, dy, 250);
        invalidate();
    }

    private android.graphics.Rect dst = new android.graphics.Rect();
    private Path path = new Path();

    public synchronized void onDraw(Canvas canvas) {
        int x, y;

        if (bitmap == null) {
            if (error) {
                canvas.translate(canvasW / 2, canvasH / 2);
                canvas.drawPath(errorPath, errorPaint);
            }
            return;
        }

        if (scroller.computeScrollOffset()) {
            scrollX = scroller.getCurrX();
            scrollY = scroller.getCurrY();
            invalidate(); /* keep animating */
        }

        // 钳位逻辑
        if (bitmapW <= canvasW) {
            scrollX = 0;
            x = (canvasW - bitmapW) / 2;
        } else {
            // ✅ 双向钳位，消除浮点截断残留
            if (scrollX < 0) scrollX = 0;
            int maxScrollX = bitmapW - canvasW;
            if (scrollX > maxScrollX) scrollX = maxScrollX;
            x = -scrollX;
        }

        if (bitmapH <= canvasH) {
            scrollY = 0;
            y = (canvasH - bitmapH) / 2;
        } else {
            if (scrollY < 0) scrollY = 0;
            int maxScrollY = bitmapH - canvasH;
            if (scrollY > maxScrollY) scrollY = maxScrollY;
            y = -scrollY;
        }

        dst.set(x, y, x + bitmapW, y + bitmapH);
        canvas.drawBitmap(bitmap, null, dst, null);

        if (showLinks && linkBounds != null) {
            for (Rect b : linkBounds) {
                canvas.drawRect(
                    x + b.x0 * viewScale,
                    y + b.y0 * viewScale,
                    x + b.x1 * viewScale,
                    y + b.y1 * viewScale,
                    linkPaint
                );
            }
        }

        if (hits != null && hits.length > 0) {
            for (Quad[] h : hits)
                for (Quad q : h) {
                    path.rewind();
                    path.moveTo(x + q.ul_x * viewScale, y + q.ul_y * viewScale);
                    path.lineTo(x + q.ll_x * viewScale, y + q.ll_y * viewScale);
                    path.lineTo(x + q.lr_x * viewScale, y + q.lr_y * viewScale);
                    path.lineTo(x + q.ur_x * viewScale, y + q.ur_y * viewScale);
                    path.close();
                    canvas.drawPath(path, hitPaint);
                }
        }

        // ✅ TTS 高亮绘制 —— 修正缩放比
        if (ttsQuads != null && ttsQuads.length > 0) {
            // ttsQuads 是在 pageScale 下用基础ctm变换的
            // 当前实际缩放是 viewScale，需要补一个比值
            float scaleRatio = viewScale / pageScale;

            for (Quad q : ttsQuads) {
                path.rewind();
                path.moveTo(x + q.ul_x * scaleRatio, y + q.ul_y * scaleRatio);
                path.lineTo(x + q.ll_x * scaleRatio, y + q.ll_y * scaleRatio);
                path.lineTo(x + q.lr_x * scaleRatio, y + q.lr_y * scaleRatio);
                path.lineTo(x + q.ur_x * scaleRatio, y + q.ur_y * scaleRatio);
                path.close();
                canvas.drawPath(path, ttsPaint);
            }
        }

        // 绘制文字选中选区
        if (isSelectingText && selectQuads != null) {
            for (Quad q : selectQuads) {
                path.rewind();
                path.moveTo(x + q.ul_x * viewScale, y + q.ul_y * viewScale);
                path.lineTo(x + q.ll_x * viewScale, y + q.ll_y * viewScale);
                path.lineTo(x + q.lr_x * viewScale, y + q.lr_y * viewScale);
                path.lineTo(x + q.ur_x * viewScale, y + q.ur_y * viewScale);
                path.close();
                canvas.drawPath(path, selectionPaint); // ✅ 使用独立的蓝色选区 Paint
            }
        }

        // ✅ 翻页页码提示气泡
        if (showPageTurnHint && !pageTurnHintText.isEmpty()) {
            float textY = canvasH * 0.35f; // 垂直偏上位置，不遮挡正文
            float paddingX = 24f * getResources().getDisplayMetrics().density;
            float paddingY = 12f * getResources().getDisplayMetrics().density;
            float cornerRadius =
                8f * getResources().getDisplayMetrics().density;

            // 测量文字尺寸
            float textWidth = hintTextPaint.measureText(pageTurnHintText);
            float textHeight = hintTextPaint.getTextSize();

            float left = (canvasW - textWidth) / 2f - paddingX;
            float top = textY - textHeight / 2f - paddingY;
            float right = (canvasW + textWidth) / 2f + paddingX;
            float bottom = textY + textHeight / 2f + paddingY;

            // 绘制圆角背景
            canvas.drawRoundRect(
                left,
                top,
                right,
                bottom,
                cornerRadius,
                cornerRadius,
                hintBgPaint
            );
            // 绘制页码文字（baseline 对齐）
            Paint.FontMetrics fm = hintTextPaint.getFontMetrics();
            float baseline = textY - (fm.ascent + fm.descent) / 2f;
            canvas.drawText(
                pageTurnHintText,
                canvasW / 2f,
                baseline,
                hintTextPaint
            );
        }
    }

    public void clearTtsHighlight() {
        this.ttsQuads = null;
        invalidate();
    }

    /**
     * 设置 TTS 高亮并自动滚动到高亮位置
     * ✅ version 参数，防止跨页高亮残留
     */

    public void setTtsHighlight(Quad[] quads, int version) {
        if (version != pageVersion) {
            Log.i(
                APP,
                "TTS highlight ignored: version mismatch (" +
                    version +
                    " vs " +
                    pageVersion +
                    ")"
            );
            return;
        }

        this.ttsQuads = quads;
        this.ttsHighlightVersion = version;

        // ✅ 仅垂直滚动到高亮位置，水平位置完全不动
        if (quads != null && quads.length > 0 && bitmapW > 0 && bitmapH > 0) {
            float scaleRatio = viewScale / pageScale;

            float centerY = 0;
            for (Quad q : quads) {
                centerY += (q.ul_y + q.lr_y) / 2f;
            }
            centerY /= quads.length;

            float viewCenterY = centerY * scaleRatio;
            int targetScrollY = (int) (viewCenterY - canvasH / 2f);

            int maxScrollY = Math.max(0, bitmapH - canvasH);
            targetScrollY = Math.max(0, Math.min(targetScrollY, maxScrollY));

            // ✅ 仅当垂直偏移超过阈值时才启动滚动动画
            if (Math.abs(targetScrollY - scrollY) > 10) {
                scroller.forceFinished(true);
                scroller.startScroll(
                    scrollX, // ← X 起点保持当前值
                    scrollY,
                    0, // ← dx = 0，不产生任何水平位移
                    targetScrollY - scrollY,
                    300
                );
            }
        }

        invalidate();
    }

    public int getPageVersion() {
        return pageVersion;
    }

    /**
     * 外部调用方在触发非 goForward/goBackward 的翻页前，
     * 主动快照当前水平位置，确保 setBitmap 能正确恢复。
     */
    public void saveCurrentScrollX() {
        savedScrollX = scrollX;
    }

    // ✅ 文字选区专用 Paint
    private final Paint selectionPaint = new Paint();

    {
        selectionPaint.setColor(0x662196F3); // 半透明蓝色
        selectionPaint.setStyle(Paint.Style.FILL);
        selectionPaint.setAntiAlias(true);
    }

    /** 设置文字选区高亮 */
    public void setSelection(Quad[] quads) {
        this.selectQuads = quads;
        this.isSelectingText = quads != null && quads.length > 0;
        invalidate();
    }

    /** 清除文字选区 */
    public void clearSelection() {
        this.selectQuads = null;
        this.isSelectingText = false;
        invalidate();
    }
}
