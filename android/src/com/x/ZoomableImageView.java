package com.x;

import android.content.Context;
import android.graphics.Matrix;
import android.graphics.PointF;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.animation.DecelerateInterpolator;
import android.widget.ImageView;
import android.widget.OverScroller;

public class ZoomableImageView extends ImageView {

    // 缩放模式
    private static final int MODE_NONE = 0;
    private static final int MODE_DRAG = 1;
    private static final int MODE_ZOOM = 2;

    // ===== 核心缩放参数 =====
    private static final float MAX_SCALE_MULTIPLE = 4.0f; // 最大缩放倍数（基于初始适配值的4倍）
    private static final float SCALE_STEP = 1.0f; // 双击步长（每次×1倍，相对初始值）
    private static final int ANIMATION_DURATION = 200; // 缩放动画时长

    // 动画相关
    private final DecelerateInterpolator mInterpolator =
        new DecelerateInterpolator(1.5f);
    private final OverScroller mScroller;

    // 触摸相关
    private int mTouchMode = MODE_NONE;
    private final PointF mLastTouchPoint = new PointF();
    private final PointF mCurrentTouchPoint = new PointF();

    // 矩阵相关
    private final Matrix mImageMatrix = new Matrix();
    private final float[] mMatrixValues = new float[9];

    // 缩放检测器
    private final ScaleGestureDetector mScaleDetector;
    private final GestureDetector mGestureDetector;

    // 尺寸相关
    private int mViewWidth, mViewHeight;
    private int mImageWidth, mImageHeight;
    private float mInitScale; // 初始缩放值（适配屏幕宽度）
    private float mCurrentScale; // 当前缩放值

    public ZoomableImageView(Context context) {
        this(context, null);
    }

    public ZoomableImageView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ZoomableImageView(
        Context context,
        AttributeSet attrs,
        int defStyle
    ) {
        super(context, attrs, defStyle);
        mScroller = new OverScroller(getContext(), mInterpolator);
        init();

        mScaleDetector = new ScaleGestureDetector(context, new ScaleListener());
        mGestureDetector = new GestureDetector(context, new GestureListener());
    }

    private void init() {
        super.setScaleType(ScaleType.MATRIX);
        setOnTouchListener(new TouchListener());
    }

    /**
     * ===== 核心优化：初始缩放适配屏幕宽度 =====
     * 逻辑：
     * 1. 计算初始缩放值 = 屏幕宽度 / 图片原始宽度
     * 2. 图片宽度 < 屏幕宽度 → 放大到屏幕宽度
     * 3. 图片宽度 > 屏幕宽度 → 缩小到屏幕宽度
     * 4. 居中显示
     */
    private void initImagePosition() {
        Drawable drawable = getDrawable();
        if (drawable == null) return;

        // 获取图片原始尺寸
        mImageWidth = drawable.getIntrinsicWidth();
        mImageHeight = drawable.getIntrinsicHeight();

        if (mImageWidth <= 0 || mImageHeight <= 0) return;

        // 1. 计算初始缩放值（适配屏幕宽度）
        mInitScale = (float) mViewWidth / mImageWidth;
        // 确保缩放值有效（防止图片尺寸为0的异常）
        mInitScale = Math.max(mInitScale, 0.1f);

        // 2. 重置矩阵
        mImageMatrix.reset();

        // 3. 应用初始缩放（适配屏幕宽度）
        mImageMatrix.postScale(mInitScale, mInitScale);

        // 4. 计算居中偏移（让图片在屏幕中居中）
        float transX = (mViewWidth - mImageWidth * mInitScale) / 2f;
        float transY = (mViewHeight - mImageHeight * mInitScale) / 2f;
        mImageMatrix.postTranslate(transX, transY);

        // 5. 更新当前缩放值
        updateMatrix();
    }

    /**
     * 更新矩阵并刷新视图
     */
    private void updateMatrix() {
        setImageMatrix(mImageMatrix);
        mCurrentScale = getScaleFromMatrix(mImageMatrix);
    }

    /**
     * 从矩阵获取当前缩放值
     */
    private float getScaleFromMatrix(Matrix matrix) {
        matrix.getValues(mMatrixValues);
        return mMatrixValues[Matrix.MSCALE_X];
    }

    /**
     * 判断是否处于缩放状态
     */
    public boolean isZoomed() {
        return Math.abs(mCurrentScale - mInitScale) > 0.01f;
    }

    /**
     * 限制缩放范围（基于初始缩放值的1-4倍）
     */
    private float clampScale(float scale) {
        float minScale = mInitScale;
        float maxScale = mInitScale * MAX_SCALE_MULTIPLE;
        return Math.max(minScale, Math.min(maxScale, scale));
    }

    /**
     * 限制拖动边界（图片不会完全移出屏幕）
     */
    private void checkBounds() {
        RectF rect = getMatrixRectF();
        float deltaX = 0,
            deltaY = 0;

        // X轴边界限制
        if (rect.width() < mViewWidth) {
            deltaX = (mViewWidth - rect.width()) / 2 - rect.left;
        } else if (rect.left > 0) {
            deltaX = -rect.left;
        } else if (rect.right < mViewWidth) {
            deltaX = mViewWidth - rect.right;
        }

        // Y轴边界限制
        if (rect.height() < mViewHeight) {
            deltaY = (mViewHeight - rect.height()) / 2 - rect.top;
        } else if (rect.top > 0) {
            deltaY = -rect.top;
        } else if (rect.bottom < mViewHeight) {
            deltaY = mViewHeight - rect.bottom;
        }

        // 应用边界修正
        if (deltaX != 0 || deltaY != 0) {
            mImageMatrix.postTranslate(deltaX, deltaY);
        }
    }

    /**
     * 获取矩阵对应的图片矩形区域
     */
    private RectF getMatrixRectF() {
        RectF rect = new RectF();
        Drawable d = getDrawable();
        if (d != null) {
            rect.set(0, 0, d.getIntrinsicWidth(), d.getIntrinsicHeight());
            mImageMatrix.mapRect(rect);
        }
        return rect;
    }

    /**
     * 平滑缩放至目标倍数（以点击点为中心）
     */
    private void zoomTo(float targetScale, float centerX, float centerY) {
        // 限制目标缩放值在合法范围
        targetScale = clampScale(targetScale);

        float currentScale = mCurrentScale;
        float scaleRatio = targetScale / currentScale;

        // 以点击点为中心缩放
        mImageMatrix.postScale(scaleRatio, scaleRatio, centerX, centerY);
        checkBounds();
        updateMatrix();

        // 平滑动画过渡
        startScaleAnimation(currentScale, targetScale, centerX, centerY);
    }

    /**
     * 缩放动画（保证过渡平滑）
     */
    private void startScaleAnimation(
        float startScale,
        float targetScale,
        float centerX,
        float centerY
    ) {
        final long startTime = System.currentTimeMillis();

        post(
            new Runnable() {
                @Override
                public void run() {
                    long currentTime = System.currentTimeMillis();
                    float progress =
                        (currentTime - startTime) / (float) ANIMATION_DURATION;
                    progress = Math.min(1.0f, progress);

                    // 减速插值，动画更自然
                    float scale =
                        startScale +
                        (targetScale - startScale) *
                        mInterpolator.getInterpolation(progress);
                    float scaleRatio = scale / getScaleFromMatrix(mImageMatrix);

                    mImageMatrix.postScale(
                        scaleRatio,
                        scaleRatio,
                        centerX,
                        centerY
                    );
                    checkBounds();
                    updateMatrix();

                    if (progress < 1.0f) {
                        post(this);
                    }
                }
            }
        );
    }

    /**
     * ===== 双击逻辑优化：基于初始缩放值逐级放大 =====
     * 逻辑：
     * 初始缩放（适配屏幕宽度）→ ×2 → ×3 → ×4 → 回到初始缩放
     */
    private void performDoubleTapZoom(float x, float y) {
        // 1. 计算当前相对初始值的倍数
        float currentMultiple = mCurrentScale / mInitScale;
        // 保留1位小数，避免精度问题
        currentMultiple = Math.round(currentMultiple * 10) / 10f;

        // 2. 计算下一个倍数
        float nextMultiple;
        if (currentMultiple >= MAX_SCALE_MULTIPLE - 0.01f) {
            // 达到最大值，回到初始缩放
            nextMultiple = 1.0f;
        } else {
            // 未到最大值，逐级+1
            nextMultiple = currentMultiple + SCALE_STEP;
            // 防止超过最大值
            nextMultiple = Math.min(nextMultiple, MAX_SCALE_MULTIPLE);
        }

        // 3. 计算目标缩放值（初始缩放 × 目标倍数）
        float targetScale = mInitScale * nextMultiple;

        // 4. 执行缩放（以双击点为中心）
        zoomTo(targetScale, x, y);
    }

    @Override
    protected void onLayout(
        boolean changed,
        int left,
        int top,
        int right,
        int bottom
    ) {
        super.onLayout(changed, left, top, right, bottom);
        if (changed) {
            mViewWidth = getWidth() - getPaddingLeft() - getPaddingRight();
            mViewHeight = getHeight() - getPaddingTop() - getPaddingBottom();
            initImagePosition();
        }
    }

    // ====================== 手势监听 ======================
    private class TouchListener implements OnTouchListener {

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            // 优先处理双击
            if (mGestureDetector.onTouchEvent(event)) return true;

            // 处理双指缩放
            mScaleDetector.onTouchEvent(event);

            mCurrentTouchPoint.set(event.getX(), event.getY());

            switch (event.getAction() & MotionEvent.ACTION_MASK) {
                case MotionEvent.ACTION_DOWN:
                    mScroller.forceFinished(true);
                    mTouchMode = MODE_DRAG;
                    mLastTouchPoint.set(mCurrentTouchPoint);
                    break;
                case MotionEvent.ACTION_POINTER_DOWN:
                    mTouchMode = MODE_ZOOM;
                    mLastTouchPoint.set(mCurrentTouchPoint);
                    break;
                case MotionEvent.ACTION_MOVE:
                    // 仅在缩放后允许拖动
                    if (
                        mTouchMode == MODE_DRAG &&
                        !isEqual(mCurrentScale, mInitScale)
                    ) {
                        float dx = mCurrentTouchPoint.x - mLastTouchPoint.x;
                        float dy = mCurrentTouchPoint.y - mLastTouchPoint.y;

                        mImageMatrix.postTranslate(dx, dy);
                        checkBounds();
                        updateMatrix();

                        mLastTouchPoint.set(mCurrentTouchPoint);
                    }
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_POINTER_UP:
                    mTouchMode = MODE_NONE;
                    break;
            }

            return true;
        }
    }

    /**
     * 双指缩放手势监听（无级缩放）
     */
    private class ScaleListener
        extends ScaleGestureDetector.SimpleOnScaleGestureListener
    {

        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            mTouchMode = MODE_ZOOM;
            return true;
        }

        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            float scaleFactor = detector.getScaleFactor();
            float currentScale = getScaleFromMatrix(mImageMatrix);
            float newScale = currentScale * scaleFactor;

            // 限制缩放范围（基于初始值的1-4倍）
            newScale = clampScale(newScale);
            scaleFactor = newScale / currentScale;

            // 以双指中点为中心缩放
            mImageMatrix.postScale(
                scaleFactor,
                scaleFactor,
                detector.getFocusX(),
                detector.getFocusY()
            );
            checkBounds();
            updateMatrix();

            return true;
        }
    }

    /**
     * 双击手势监听
     */
    private class GestureListener
        extends GestureDetector.SimpleOnGestureListener
    {

        @Override
        public boolean onDoubleTap(MotionEvent e) {
            // 触发双击缩放逻辑
            performDoubleTapZoom(e.getX(), e.getY());
            return true;
        }
    }

    /**
     * 重置缩放（回到初始适配屏幕宽度的状态）
     */
    public void resetZoom() {
        zoomTo(mInitScale, mViewWidth / 2, mViewHeight / 2);
    }

    @Override
    public void setImageMatrix(Matrix matrix) {
        super.setImageMatrix(matrix);
        mImageMatrix.set(matrix);
    }

    /**
     * 获取当前缩放值（对外接口）
     */
    public float getCurrentScale() {
        return mCurrentScale;
    }

    /**
     * 浮点值相等判断（解决精度问题）
     */
    private boolean isEqual(float a, float b) {
        return Math.abs(a - b) < 0.01f;
    }
}
