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

    private static final int SCROLL_EPSILON = 2; // 容差像素
    protected int savedScrollX = 0; // 当前阅读水平位置，翻页时跨页恢复

    // TTS 高亮
    private Quad[] ttsQuads = null;
    private final Paint ttsPaint = new Paint();
    // ✅ 页面版本号，每次 setBitmap 递增
    private int pageVersion = 0;
    private int ttsHighlightVersion = -1; // 高亮对应的页面版本

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

    public void onLongPress(MotionEvent e) {
        showLinks = !showLinks;
        invalidate();
    }

    public boolean onSingleTapUp(MotionEvent e) {
        boolean foundLink = false;
        float x = e.getX();
        float y = e.getY();
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

    public synchronized boolean onFling(
        MotionEvent e1,
        MotionEvent e2,
        float dx,
        float dy
    ) {
        if (bitmap != null) {
            int maxX = bitmapW > canvasW ? bitmapW - canvasW : 0;
            int maxY = bitmapH > canvasH ? bitmapH - canvasH : 0;
            scroller.forceFinished(true);
            scroller.fling(
                scrollX,
                scrollY,
                (int) -dx,
                (int) -dy,
                0,
                maxX,
                0,
                maxY
            );
            invalidate();
        }
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

    /*public void goBackward() {
        scroller.forceFinished(true);
        if (scrollY <= 0) {
            if (scrollX <= 0) {
                if (actionListener != null) actionListener.goBackward();
                return;
            }
            scroller.startScroll(
                scrollX,
                scrollY,
                (-canvasW * 9) / 10,
                bitmapH - canvasH - scrollY,
                500
            );
        } else {
            scroller.startScroll(scrollX, scrollY, 0, (-canvasH * 9) / 10, 250);
        }
        invalidate();
    }*/

    /*public void goForward() {
        scroller.forceFinished(true);
        if (scrollY + canvasH >= bitmapH) {
            if (scrollX + canvasW >= bitmapW) {
                if (actionListener != null) actionListener.goForward();
                return;
            }
            scroller.startScroll(
                scrollX,
                scrollY,
                (canvasW * 9) / 10,
                -scrollY,
                500
            );
        } else {
            scroller.startScroll(scrollX, scrollY, 0, (canvasH * 9) / 10, 250);
        }
        invalidate();
    }*/

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

        /*if (bitmapW <= canvasW) {
            scrollX = 0;
            x = (canvasW - bitmapW) / 2;
        } else {
            if (scrollX < 0) scrollX = 0;
            if (scrollX > bitmapW - canvasW) scrollX = bitmapW - canvasW;
            x = -scrollX;
        }

        if (bitmapH <= canvasH) {
            scrollY = 0;
            y = (canvasH - bitmapH) / 2;
        } else {
            if (scrollY < 0) scrollY = 0;
            if (scrollY > bitmapH - canvasH) scrollY = bitmapH - canvasH;
            y = -scrollY;
        }*/
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
}
