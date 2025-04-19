package com.x;

import android.content.Context;
import android.graphics.Matrix;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.ViewTreeObserver;
import android.widget.ImageView;

public class ZoomableImageView extends ImageView {

    private Matrix matrix = new Matrix();
    private float scaleFactor = 1.0f;
    private ScaleGestureDetector scaleGestureDetector;
    private GestureDetector gestureDetector;
    private PointF lastPoint = new PointF();
    private Matrix savedMatrix = new Matrix();
    private boolean isFirstCentered = false;
    private float initialScale = 1.0f;

    private static final float MIN_SCALE = 0.1f;
    private static final float MAX_SCALE = 10.0f;

    private boolean isScaling = false;

    public ZoomableImageView(Context context) {
        super(context);
        init(context);
    }

    public ZoomableImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public ZoomableImageView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        setScaleType(ScaleType.MATRIX);
        scaleGestureDetector = new ScaleGestureDetector(context, new ScaleListener());
        gestureDetector = new GestureDetector(context, new GestureListener());

        getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                getViewTreeObserver().removeOnGlobalLayoutListener(this);
                centerAndScaleImage();
            }
        });
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        scaleGestureDetector.onTouchEvent(event);
        gestureDetector.onTouchEvent(event);

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                lastPoint.set(event.getX(), event.getY());
                break;
            case MotionEvent.ACTION_MOVE:
                if (event.getPointerCount() == 1 && !isScaling) {
                    float dx = event.getX() - lastPoint.x;
                    float dy = event.getY() - lastPoint.y;
                    matrix.postTranslate(dx, dy);
                    fixTranslation(); // 移动后调整位置
                    setImageMatrix(matrix);
                    lastPoint.set(event.getX(), event.getY());
                }
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                isScaling = false;
                break;
        }
        return true;
    }

    private void centerAndScaleImage() {
        if (getDrawable() == null) return;

        int viewWidth = getWidth();
        int viewHeight = getHeight();
        int drawableWidth = getDrawable().getIntrinsicWidth();
        int drawableHeight = getDrawable().getIntrinsicHeight();

        float scale = (float) (viewWidth * 0.75) / drawableWidth;
        initialScale = scale;

        matrix.setScale(scale, scale);
        float translateX = (viewWidth - drawableWidth * scale) / 2;
        float translateY = (viewHeight - drawableHeight * scale) / 2;
        matrix.postTranslate(translateX, translateY);

        setImageMatrix(matrix);
        savedMatrix.set(matrix);
        scaleFactor = initialScale;
        isFirstCentered = true;
    }

    private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
        @Override
        public boolean onScaleBegin(ScaleGestureDetector detector) {
            isScaling = true;
            return true;
        }

        @Override
        public boolean onScale(ScaleGestureDetector detector) {
            float scale = detector.getScaleFactor();
            float newScale = scaleFactor * scale;

            if (newScale > MAX_SCALE) {
                scale = MAX_SCALE / scaleFactor;
                scaleFactor = MAX_SCALE;
            } else if (newScale < MIN_SCALE) {
                scale = MIN_SCALE / scaleFactor;
                scaleFactor = MIN_SCALE;
            } else {
                scaleFactor = newScale;
            }

            matrix.postScale(scale, scale, detector.getFocusX(), detector.getFocusY());
            fixTranslation(); // 缩放时调整位置
            setImageMatrix(matrix);
            return true;
        }

        @Override
        public void onScaleEnd(ScaleGestureDetector detector) {
            isScaling = false;
            fixTranslation(); // 缩放结束后调整位置
        }
    }

    private class GestureListener extends GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onDoubleTap(MotionEvent e) {
            matrix.set(savedMatrix);
            scaleFactor = initialScale;
            setImageMatrix(matrix);
            return true;
        }
    }

    private void fixTranslation() {
        if (getDrawable() == null) return;

        RectF rect = new RectF(0, 0, getDrawable().getIntrinsicWidth(), getDrawable().getIntrinsicHeight());
        matrix.mapRect(rect);

        float deltaX = 0;
        float deltaY = 0;

        int viewWidth = getWidth();
        int viewHeight = getHeight();

        // 水平调整
        if (rect.width() <= viewWidth) {
            deltaX = (viewWidth - rect.width()) / 2 - rect.left;
        } else {
            if (rect.left > 0) deltaX = -rect.left;
            if (rect.right < viewWidth) deltaX = viewWidth - rect.right;
        }

        // 垂直调整
        if (rect.height() <= viewHeight) {
            deltaY = (viewHeight - rect.height()) / 2 - rect.top;
        } else {
            if (rect.top > 0) deltaY = -rect.top;
            if (rect.bottom < viewHeight) deltaY = viewHeight - rect.bottom;
        }

        // 应用修正
        if (deltaX != 0 || deltaY != 0) {
            matrix.postTranslate(deltaX, deltaY);
            setImageMatrix(matrix);
        }
    }
}