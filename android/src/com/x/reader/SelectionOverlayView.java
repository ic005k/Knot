package com.x.reader;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class SelectionOverlayView extends View {

    private static final String TAG = "SelectionOverlay";

    public interface OnTextSelectedListener {
        void onTextSelected(
            String text,
            float docLeft,
            float docTop,
            float docRight,
            float docBottom,
            int pageNum
        );
        void onSelectionCancelled();
    }

    // Handle type constants
    private static final int NONE = 0;
    private static final int CORNER_TL = 1;
    private static final int CORNER_TR = 2;
    private static final int CORNER_BL = 3;
    private static final int CORNER_BR = 4;
    private static final int EDGE_TOP = 5;
    private static final int EDGE_BOTTOM = 6;
    private static final int EDGE_LEFT = 7;
    private static final int EDGE_RIGHT = 8;
    private static final int MOVE = 9;
    private static final int CONFIRM_BTN = 10; // ✅ 新增：确认按钮

    private final RectF mSelectionRect = new RectF();
    private final RectF mConfirmBtnRect = new RectF(); // ✅ 确认按钮区域
    private int mActiveHandle = NONE;
    private float mLastX, mLastY;
    private boolean mIsActive;

    private float mHandleRadius;
    private float mEdgeTouchWidth;
    private float mMinRectSize;
    private float mConfirmBtnRadius; // ✅ 确认按钮半径

    private final Paint mFillPaint = new Paint();
    private final Paint mBorderPaint = new Paint();
    private final Paint mHandleFillPaint = new Paint();
    private final Paint mHandleStrokePaint = new Paint();
    private final Paint mDragHandlePaint = new Paint();
    private final Paint mDimPaint = new Paint();
    private final Paint mConfirmBtnBgPaint = new Paint(); // ✅ 确认按钮背景
    private final Paint mConfirmBtnIconPaint = new Paint(); // ✅ 确认按钮图标

    private OnTextSelectedListener mListener;
    private PageView mPageView;
    private boolean mInvertMode = false;

    public SelectionOverlayView(Context context) {
        super(context);
        init(context);
    }

    private void init(Context context) {
        float density = context.getResources().getDisplayMetrics().density;
        mHandleRadius = 10 * density;
        mEdgeTouchWidth = 30 * density;
        mMinRectSize = 30 * density;
        mConfirmBtnRadius = 18 * density; // ✅ 确认按钮大小：18dp 半径

        updatePaintColors();
    }

    private void updatePaintColors() {
        if (mInvertMode) {
            mFillPaint.setColor(0x20FFFFFF);
            mBorderPaint.setColor(0xAA90CAF9);
            mHandleFillPaint.setColor(0xFF90CAF9);
            mHandleStrokePaint.setColor(0xFF000000);
            mDragHandlePaint.setColor(0xCC64B5F6);
            mDimPaint.setColor(0x40000000);
            mConfirmBtnBgPaint.setColor(0xFF4CAF50);
            mConfirmBtnIconPaint.setColor(0xFFFFFFFF);
        } else {
            mFillPaint.setColor(0x301565C0);
            mBorderPaint.setColor(0xCC1565C0);
            mHandleFillPaint.setColor(0xFF1565C0);
            mHandleStrokePaint.setColor(0xFFFFFFFF);
            mDragHandlePaint.setColor(0xCC1976D2);
            mDimPaint.setColor(0x20000000);
            mConfirmBtnBgPaint.setColor(0xFF2E7D32);
            mConfirmBtnIconPaint.setColor(0xFFFFFFFF);
        }

        mFillPaint.setStyle(Paint.Style.FILL);
        mFillPaint.setAntiAlias(true);

        mBorderPaint.setStyle(Paint.Style.STROKE);
        mBorderPaint.setStrokeWidth(2.5f);
        mBorderPaint.setAntiAlias(true);

        mHandleFillPaint.setStyle(Paint.Style.FILL);
        mHandleFillPaint.setAntiAlias(true);

        mHandleStrokePaint.setStyle(Paint.Style.STROKE);
        mHandleStrokePaint.setStrokeWidth(2f);
        mHandleStrokePaint.setAntiAlias(true);

        mDragHandlePaint.setStyle(Paint.Style.FILL);
        mDragHandlePaint.setAntiAlias(true);

        mDimPaint.setStyle(Paint.Style.FILL);

        mConfirmBtnBgPaint.setStyle(Paint.Style.FILL);
        mConfirmBtnBgPaint.setAntiAlias(true);

        mConfirmBtnIconPaint.setStyle(Paint.Style.STROKE);
        mConfirmBtnIconPaint.setStrokeWidth(3f);
        mConfirmBtnIconPaint.setStrokeCap(Paint.Cap.ROUND);
        mConfirmBtnIconPaint.setStrokeJoin(Paint.Join.ROUND);
        mConfirmBtnIconPaint.setAntiAlias(true);
    }

    public void initRect(float centerX, float centerY) {
        float w = getWidth();
        float h = getHeight();
        if (w <= 0 || h <= 0) return;

        // ✅ 【修改】缩小默认选框：宽 40%、高 15%（原为 60%×25%）
        // 更适合精确选择一段文字或几个句子
        float initW = w * 0.40f;
        float initH = h * 0.15f;
        float left = Math.max(0, centerX - initW / 2);
        float top = Math.max(0, centerY - initH / 2);
        float right = Math.min(w, left + initW);
        float bottom = Math.min(h, top + initH);

        mSelectionRect.set(left, top, right, bottom);
        updateConfirmBtnPosition();
        mIsActive = true;
        invalidate();
    }

    /**
     * ✅ 更新确认按钮位置：始终在选框右下角外侧
     */
    private void updateConfirmBtnPosition() {
        float btnR = mConfirmBtnRadius;
        float gap = btnR * 0.5f; // 与选框的间距
        float cx = mSelectionRect.right + gap + btnR;
        float cy = mSelectionRect.bottom + gap + btnR;

        // ✅ 边界约束：按钮不能超出 View
        float viewW = getWidth();
        float viewH = getHeight();
        if (cx + btnR > viewW) cx = viewW - btnR;
        if (cy + btnR > viewH) cy = viewH - btnR;
        // 如果右下角放不下，尝试放到右上角外侧
        if (cy + btnR > viewH && mSelectionRect.top - gap - btnR * 2 >= 0) {
            cy = mSelectionRect.top - gap - btnR;
        }

        mConfirmBtnRect.set(cx - btnR, cy - btnR, cx + btnR, cy + btnR);
    }

    public void setListener(OnTextSelectedListener listener) {
        mListener = listener;
    }

    public void setPageView(PageView pageView) {
        mPageView = pageView;
    }

    public void setInvertMode(boolean invert) {
        mInvertMode = invert;
        updatePaintColors();
        invalidate();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                // ✅ 优先检测确认按钮
                if (isInsideConfirmBtn(x, y)) {
                    mActiveHandle = CONFIRM_BTN;
                    mLastX = x;
                    mLastY = y;
                    return true;
                }

                mActiveHandle = detectHandle(x, y);
                if (mActiveHandle == NONE) {
                    if (mSelectionRect.contains(x, y)) {
                        mActiveHandle = MOVE;
                    } else {
                        // ✅ 点击矩形外部且不是确认按钮 → 取消选择
                        cancelSelection();
                        return true;
                    }
                }
                mLastX = x;
                mLastY = y;
                return true;
            case MotionEvent.ACTION_MOVE:
                if (mActiveHandle == CONFIRM_BTN) {
                    // ✅ 确认按钮不支持拖拽，仅响应点击
                    return true;
                }
                if (mActiveHandle != NONE) {
                    float dx = x - mLastX;
                    float dy = y - mLastY;
                    moveHandle(dx, dy);
                    mLastX = x;
                    mLastY = y;
                    invalidate();
                }
                return true;
            case MotionEvent.ACTION_UP:
                if (mActiveHandle == CONFIRM_BTN) {
                    // ✅ 【核心修改】只有点击确认按钮才提取文字并回调
                    mActiveHandle = NONE;
                    extractAndCallback();
                    return true;
                }
                // ✅ 【核心修改】普通拖拽松手不再触发菜单，仅重置状态
                if (mActiveHandle != NONE && mIsActive) {
                    mActiveHandle = NONE;
                    // 更新确认按钮位置（因为选框可能已移动/缩放）
                    updateConfirmBtnPosition();
                    invalidate();
                }
                return true;
            case MotionEvent.ACTION_CANCEL:
                cancelSelection();
                return true;
        }
        return true;
    }

    private boolean isInsideConfirmBtn(float x, float y) {
        float dx = x - mConfirmBtnRect.centerX();
        float dy = y - mConfirmBtnRect.centerY();
        // ✅ 扩大触摸区域 1.5 倍，方便点击
        float touchRadius = mConfirmBtnRadius * 1.5f;
        return dx * dx + dy * dy <= touchRadius * touchRadius;
    }

    /**
     * ✅ 【修复崩溃】使用 post() 延迟取消，避免在触摸事件分发中修改 View 树
     */
    private void cancelSelection() {
        mActiveHandle = NONE;
        mIsActive = false;
        invalidate();
        post(() -> {
            if (mListener != null) {
                mListener.onSelectionCancelled();
            }
        });
    }

    private int detectHandle(float x, float y) {
        float hr = mHandleRadius * 2.5f;

        // 1. 优先检测四角
        if (
            dist(x, y, mSelectionRect.left, mSelectionRect.top) < hr
        ) return CORNER_TL;
        if (
            dist(x, y, mSelectionRect.right, mSelectionRect.top) < hr
        ) return CORNER_TR;
        if (
            dist(x, y, mSelectionRect.left, mSelectionRect.bottom) < hr
        ) return CORNER_BL;
        if (
            dist(x, y, mSelectionRect.right, mSelectionRect.bottom) < hr
        ) return CORNER_BR;

        // 2. 检测四边
        float et = mEdgeTouchWidth / 2;
        if (y >= mSelectionRect.top - et && y <= mSelectionRect.top + et) {
            if (
                x >= mSelectionRect.left + hr && x <= mSelectionRect.right - hr
            ) return EDGE_TOP;
        }
        if (
            y >= mSelectionRect.bottom - et && y <= mSelectionRect.bottom + et
        ) {
            if (
                x >= mSelectionRect.left + hr && x <= mSelectionRect.right - hr
            ) return EDGE_BOTTOM;
        }
        if (x >= mSelectionRect.left - et && x <= mSelectionRect.left + et) {
            if (
                y >= mSelectionRect.top + hr && y <= mSelectionRect.bottom - hr
            ) return EDGE_LEFT;
        }
        if (x >= mSelectionRect.right - et && x <= mSelectionRect.right + et) {
            if (
                y >= mSelectionRect.top + hr && y <= mSelectionRect.bottom - hr
            ) return EDGE_RIGHT;
        }

        return NONE;
    }

    private float dist(float x1, float y1, float x2, float y2) {
        float dx = x1 - x2;
        float dy = y1 - y2;
        return (float) Math.sqrt(dx * dx + dy * dy);
    }

    private void moveHandle(float dx, float dy) {
        float w = getWidth();
        float h = getHeight();
        RectF r = mSelectionRect;

        switch (mActiveHandle) {
            case CORNER_TL:
                r.left = clamp(r.left + dx, 0, r.right - mMinRectSize);
                r.top = clamp(r.top + dy, 0, r.bottom - mMinRectSize);
                break;
            case CORNER_TR:
                r.right = clamp(r.right + dx, r.left + mMinRectSize, w);
                r.top = clamp(r.top + dy, 0, r.bottom - mMinRectSize);
                break;
            case CORNER_BL:
                r.left = clamp(r.left + dx, 0, r.right - mMinRectSize);
                r.bottom = clamp(r.bottom + dy, r.top + mMinRectSize, h);
                break;
            case CORNER_BR:
                r.right = clamp(r.right + dx, r.left + mMinRectSize, w);
                r.bottom = clamp(r.bottom + dy, r.top + mMinRectSize, h);
                break;
            case EDGE_TOP:
                r.top = clamp(r.top + dy, 0, r.bottom - mMinRectSize);
                break;
            case EDGE_BOTTOM:
                r.bottom = clamp(r.bottom + dy, r.top + mMinRectSize, h);
                break;
            case EDGE_LEFT:
                r.left = clamp(r.left + dx, 0, r.right - mMinRectSize);
                break;
            case EDGE_RIGHT:
                r.right = clamp(r.right + dx, r.left + mMinRectSize, w);
                break;
            case MOVE:
                float rw = r.width();
                float rh = r.height();
                float newLeft = r.left + dx;
                float newTop = r.top + dy;
                if (newLeft < 0) newLeft = 0;
                if (newTop < 0) newTop = 0;
                if (newLeft + rw > w) newLeft = w - rw;
                if (newTop + rh > h) newTop = h - rh;
                r.left = newLeft;
                r.top = newTop;
                r.right = newLeft + rw;
                r.bottom = newTop + rh;
                break;
        }
    }

    private float clamp(float val, float min, float max) {
        return Math.max(min, Math.min(val, max));
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (!mIsActive) return;
        super.onDraw(canvas);

        float w = getWidth();
        float h = getHeight();
        RectF r = mSelectionRect;

        // 1. 矩形外部半透明遮罩
        canvas.drawRect(0, 0, w, r.top, mDimPaint);
        canvas.drawRect(0, r.bottom, w, h, mDimPaint);
        canvas.drawRect(0, r.top, r.left, r.bottom, mDimPaint);
        canvas.drawRect(r.right, r.top, w, r.bottom, mDimPaint);

        // 2. 选区填充 + 边框
        canvas.drawRect(r, mFillPaint);
        canvas.drawRect(r, mBorderPaint);

        // 3. 四角圆形手柄
        float hr = mHandleRadius;
        canvas.drawCircle(r.left, r.top, hr, mHandleFillPaint);
        canvas.drawCircle(r.left, r.top, hr, mHandleStrokePaint);
        canvas.drawCircle(r.right, r.top, hr, mHandleFillPaint);
        canvas.drawCircle(r.right, r.top, hr, mHandleStrokePaint);
        canvas.drawCircle(r.left, r.bottom, hr, mHandleFillPaint);
        canvas.drawCircle(r.left, r.bottom, hr, mHandleStrokePaint);
        canvas.drawCircle(r.right, r.bottom, hr, mHandleFillPaint);
        canvas.drawCircle(r.right, r.bottom, hr, mHandleStrokePaint);

        // 4. 四边中点方形手柄
        float ehr = hr * 0.6f;
        float midX = (r.left + r.right) / 2;
        float midY = (r.top + r.bottom) / 2;
        drawSquareHandle(canvas, midX, r.top, ehr);
        drawSquareHandle(canvas, midX, r.bottom, ehr);
        drawSquareHandle(canvas, r.left, midY, ehr);
        drawSquareHandle(canvas, r.right, midY, ehr);

        // 5. 中心拖拽指示器（小菱形）
        float ds = hr * 0.7f;
        Path diamond = new Path();
        diamond.moveTo(midX, midY - ds);
        diamond.lineTo(midX + ds, midY);
        diamond.lineTo(midX, midY + ds);
        diamond.lineTo(midX - ds, midY);
        diamond.close();
        canvas.drawPath(diamond, mDragHandlePaint);

        // 6. ✅ 确认按钮（绿色圆形 + ✓ 图标）
        float btnCx = mConfirmBtnRect.centerX();
        float btnCy = mConfirmBtnRect.centerY();
        float btnR = mConfirmBtnRadius;
        canvas.drawCircle(btnCx, btnCy, btnR, mConfirmBtnBgPaint);

        // 绘制 ✓ 勾号
        float checkScale = btnR * 0.45f;
        Path checkMark = new Path();
        checkMark.moveTo(btnCx - checkScale, btnCy);
        checkMark.lineTo(btnCx - checkScale * 0.2f, btnCy + checkScale * 0.8f);
        checkMark.lineTo(btnCx + checkScale, btnCy - checkScale * 0.6f);
        canvas.drawPath(checkMark, mConfirmBtnIconPaint);
    }

    private void drawSquareHandle(
        Canvas canvas,
        float cx,
        float cy,
        float halfSize
    ) {
        canvas.drawRect(
            cx - halfSize,
            cy - halfSize,
            cx + halfSize,
            cy + halfSize,
            mHandleFillPaint
        );
        canvas.drawRect(
            cx - halfSize,
            cy - halfSize,
            cx + halfSize,
            cy + halfSize,
            mHandleStrokePaint
        );
    }

    /**
     * ✅ 【修复崩溃】使用 post() 延迟回调，避免在触摸事件中同步移除 View
     */
    private void extractAndCallback() {
        if (mPageView == null || mListener == null) {
            mIsActive = false;
            return;
        }

        float scale = mPageView.getSourceScale();
        float viewScale =
            (scale * (float) getWidth()) / (float) mPageView.getPageSize().x;

        if (viewScale <= 0) {
            mIsActive = false;
            return;
        }

        float docLeft = mSelectionRect.left / viewScale;
        float docTop = mSelectionRect.top / viewScale;
        float docRight = mSelectionRect.right / viewScale;
        float docBottom = mSelectionRect.bottom / viewScale;

        String text = mPageView.mCore.getTextInRect(
            mPageView.getPage(),
            docLeft,
            docTop,
            docRight,
            docBottom
        );

        // ✅ 先标记为非活跃，防止重复触发
        mIsActive = false;
        invalidate();

        // ✅ 延迟到下一帧再回调，确保触摸事件链完整结束
        final String finalText = text;
        final float fDocLeft = docLeft;
        final float fDocTop = docTop;
        final float fDocRight = docRight;
        final float fDocBottom = docBottom;
        final int fPageNum = mPageView.getPage();

        post(() -> {
            if (finalText != null && !finalText.trim().isEmpty()) {
                mListener.onTextSelected(
                    finalText.trim(),
                    fDocLeft,
                    fDocTop,
                    fDocRight,
                    fDocBottom,
                    fPageNum
                );
            } else {
                mListener.onSelectionCancelled();
            }
        });
    }
}
