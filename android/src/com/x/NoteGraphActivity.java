package com.x;

import android.os.Bundle;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import androidx.appcompat.app.AppCompatActivity;
import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;
import java.util.ArrayList;
import java.util.List;

public class NoteGraphActivity extends AppCompatActivity {

    public static NoteGraphActivity mInstance = null;
    private GraphView mGraphView;
    private String mSelectedFilePath = null;
    private boolean isDark;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
        isDark = ImmersiveUtil.applyRealImmersive(this);
        // 外层垂直布局：绘图区 + 底部按钮栏
        LinearLayout rootLayout = new LinearLayout(this);
        rootLayout.setOrientation(LinearLayout.VERTICAL);
        rootLayout.setLayoutParams(
            new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT
            )
        );
        // 1. 绘图区域（占满剩余空间）
        mGraphView = new GraphView(this);
        LinearLayout.LayoutParams graphLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            0,
            1.0f
        );
        mGraphView.setLayoutParams(graphLp);
        rootLayout.addView(mGraphView);
        // 2. 底部按钮栏
        LinearLayout bottomBar = new LinearLayout(this);
        bottomBar.setOrientation(LinearLayout.HORIZONTAL);
        bottomBar.setPadding(dp(16), dp(8), dp(16), dp(8));
        bottomBar.setGravity(Gravity.CENTER_HORIZONTAL);
        bottomBar.setWeightSum(2);
        Button btnView = new Button(this);
        Button btnEdit = new Button(this);
        boolean isZh = MyActivity.zh_cn;
        btnView.setText(isZh ? "查看" : "View");
        btnEdit.setText(isZh ? "编辑" : "Edit");
        LinearLayout.LayoutParams btnLp = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT,
            1
        );
        btnView.setLayoutParams(btnLp);
        btnEdit.setLayoutParams(btnLp);
        btnView.setOnClickListener(v -> {
            if (
                mSelectedFilePath == null || mSelectedFilePath.isEmpty()
            ) return;
            MyActivity.mInstance.PublicJavaCallCpp(
                "note_view_graph|==|" + mSelectedFilePath
            );
        });
        btnEdit.setOnClickListener(v -> {
            if (
                mSelectedFilePath == null || mSelectedFilePath.isEmpty()
            ) return;
            MyActivity.mInstance.PublicJavaCallCpp(
                "note_edit_graph|==|" + mSelectedFilePath
            );
        });
        bottomBar.addView(btnView);
        bottomBar.addView(btnEdit);
        rootLayout.addView(bottomBar);
        setContentView(rootLayout);
        // 读取传入数据：ArrayList，0=中心标题，1=图谱json字符串
        ArrayList<String> args = getIntent().getStringArrayListExtra(
            "graph_data"
        );
        if (args != null && args.size() >= 2) {
            String centerTitle = args.get(0);
            String jsonStr = args.get(1);
            GraphData graphData = new Gson().fromJson(jsonStr, GraphData.class);
            mGraphView.setGraphData(graphData, centerTitle);
            mGraphView.setOnNodeSelectListener(filePath -> {
                mSelectedFilePath = filePath;
            });
        }
    }

    private int dp(int dpVal) {
        float density = getResources().getDisplayMetrics().density;
        return (int) (dpVal * density + 0.5f);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mInstance = null;
    }

    // ========== 数据模型 ==========
    public static class GraphData {

        @SerializedName("current")
        public String current;

        @SerializedName("links")
        public List<GraphLink> links;
    }

    public static class GraphLink {

        @SerializedName("dir")
        public String dir;

        @SerializedName("file")
        public String file;

        @SerializedName("title")
        public String title;
    }

    // ========== 自定义绘图View ==========
    public static class GraphView extends View {

        private GraphData mData;
        private String mCenterTitle;
        private OnNodeSelectListener mNodeSelectListener;
        private float mOffsetX = 0f;
        private float mOffsetY = 0f;
        private float mLastTouchX;
        private float mLastTouchY;
        private boolean mIsDragging;

        private static class Node {

            float x;
            float y;
            String title;
            String filePath;
            String dir;
        }

        private final List<Node> mNodeList = new ArrayList<>();
        private Node mSelectedNode = null;
        private final float mNodeRadius = dpRaw(24);
        private final float arrowSize = dpRaw(8);
        private boolean mNeedLayoutNodes = false;

        public interface OnNodeSelectListener {
            void onSelect(String filePath);
        }

        public void setOnNodeSelectListener(OnNodeSelectListener l) {
            mNodeSelectListener = l;
        }

        public GraphView(android.content.Context ctx) {
            super(ctx);
            setBackgroundColor(0xFF1E1E1E);
        }

        public void setGraphData(GraphData data, String centerTitle) {
            mData = data;
            mCenterTitle = centerTitle;
            mNeedLayoutNodes = true;
            mNodeList.clear();
            invalidate();
        }

        private void layoutNodes() {
            if (mData == null) return;
            mNodeList.clear();
            int w = getWidth();
            int h = getHeight();
            float centerX = w / 2f;
            float centerY = h / 2f;
            // 中心节点
            Node centerNode = new Node();
            centerNode.x = centerX;
            centerNode.y = centerY;
            centerNode.title = mCenterTitle;
            centerNode.filePath = mData.current;
            centerNode.dir = "";
            mNodeList.add(centerNode);

            if (mData.links != null) {
                int count = mData.links.size();
                // 动态半径，节点越多圆环越大
                float radius = 80 + count * 45f;
                for (int i = 0; i < count; i++) {
                    GraphLink link = mData.links.get(i);
                    Node nd = new Node();
                    double rad = (2 * Math.PI * i) / count;
                    nd.x = centerX + (float) (radius * Math.cos(rad));
                    nd.y = centerY + (float) (radius * Math.sin(rad));
                    nd.title = link.title;
                    nd.filePath = link.file;
                    nd.dir = link.dir;
                    mNodeList.add(nd);
                }
            }
            mNeedLayoutNodes = false;
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
            if (mNeedLayoutNodes) {
                layoutNodes();
                invalidate();
            }
        }

        /**
         * 沿着二次贝塞尔曲线终点切线绘制箭头
         * p0起点，p1控制点，p2终点
         */
        private void drawArrowOnCurveEnd(
            android.graphics.Canvas canvas,
            android.graphics.Paint paint,
            float p0x,
            float p0y,
            float p1x,
            float p1y,
            float p2x,
            float p2y
        ) {
            // 取t=0.98，获取曲线末端切线向量
            float t = 0.98f;
            float dx = 2 * (1 - t) * (p1x - p0x) + 2 * t * (p2x - p1x);
            float dy = 2 * (1 - t) * (p1y - p0y) + 2 * t * (p2y - p1y);
            float len = (float) Math.hypot(dx, dy);
            if (len < 1e-6f) return;
            float nx = dx / len;
            float ny = dy / len;

            // 箭头顶点：往曲线起点方向回退，停靠在目标节点圆周上
            float tipX = p2x - nx * mNodeRadius;
            float tipY = p2y - ny * mNodeRadius;

            // 箭头两翼，垂直于切线
            float ax = -ny * arrowSize;
            float ay = nx * arrowSize;
            canvas.drawLine(
                tipX,
                tipY,
                tipX - nx * arrowSize + ax,
                tipY - ny * arrowSize + ay,
                paint
            );
            canvas.drawLine(
                tipX,
                tipY,
                tipX - nx * arrowSize - ax,
                tipY - ny * arrowSize - ay,
                paint
            );
        }

        @Override
        protected void onDraw(android.graphics.Canvas canvas) {
            super.onDraw(canvas);
            if (mData == null || mNodeList.isEmpty()) return;
            canvas.save();
            canvas.translate(mOffsetX, mOffsetY);
            android.graphics.Paint linePaint = new android.graphics.Paint();
            linePaint.setStrokeWidth(2);
            linePaint.setStyle(android.graphics.Paint.Style.STROKE);
            android.graphics.Paint nodePaint = new android.graphics.Paint();
            android.graphics.Paint textPaint = new android.graphics.Paint();
            textPaint.setColor(0xFFFFFFFF);
            textPaint.setTextSize(36);
            textPaint.setTextAlign(android.graphics.Paint.Align.CENTER);
            Node center = mNodeList.get(0);
            android.graphics.Path path = new android.graphics.Path();

            for (int i = 1; i < mNodeList.size(); i++) {
                Node nd = mNodeList.get(i);
                String dir = nd.dir;
                // 贝塞尔控制点向外偏移
                float dx = nd.x - center.x;
                float dy = nd.y - center.y;
                float len = (float) Math.hypot(dx, dy);
                float ctrlX, ctrlY;
                if (len > 0) {
                    float nx = -dy / len;
                    float ny = dx / len;
                    float curveOffset = 55f;
                    ctrlX = (center.x + nd.x) / 2f + nx * curveOffset;
                    ctrlY = (center.y + nd.y) / 2f + ny * curveOffset;
                } else {
                    ctrlX = (center.x + nd.x) / 2f;
                    ctrlY = (center.y + nd.y) / 2f;
                }

                if ("out".equals(dir)) {
                    linePaint.setColor(0xFF4CAF50);
                    path.reset();
                    path.moveTo(center.x, center.y);
                    path.quadTo(ctrlX, ctrlY, nd.x, nd.y);
                    canvas.drawPath(path, linePaint);
                    // 起点center，控制点ctrlX/ctrlY，终点nd
                    drawArrowOnCurveEnd(
                        canvas,
                        linePaint,
                        center.x,
                        center.y,
                        ctrlX,
                        ctrlY,
                        nd.x,
                        nd.y
                    );
                } else if ("in".equals(dir)) {
                    linePaint.setColor(0xFFF44336);
                    path.reset();
                    path.moveTo(nd.x, nd.y);
                    path.quadTo(ctrlX, ctrlY, center.x, center.y);
                    canvas.drawPath(path, linePaint);
                    // 起点nd，控制点ctrlX/ctrlY，终点center
                    drawArrowOnCurveEnd(
                        canvas,
                        linePaint,
                        nd.x,
                        nd.y,
                        ctrlX,
                        ctrlY,
                        center.x,
                        center.y
                    );
                } else if ("both".equals(dir)) {
                    linePaint.setColor(0xFFFFEB3B);
                    path.reset();
                    path.moveTo(center.x, center.y);
                    path.quadTo(ctrlX, ctrlY, nd.x, nd.y);
                    canvas.drawPath(path, linePaint);
                    drawArrowOnCurveEnd(
                        canvas,
                        linePaint,
                        center.x,
                        center.y,
                        ctrlX,
                        ctrlY,
                        nd.x,
                        nd.y
                    );
                    drawArrowOnCurveEnd(
                        canvas,
                        linePaint,
                        nd.x,
                        nd.y,
                        ctrlX,
                        ctrlY,
                        center.x,
                        center.y
                    );
                }
            }
            // 绘制节点
            for (Node nd : mNodeList) {
                if (nd == mSelectedNode) {
                    nodePaint.setColor(0xFF42A5F5);
                } else if (nd == center) {
                    nodePaint.setColor(0xFFFFC107);
                } else {
                    nodePaint.setColor(0xFF444444);
                }
                canvas.drawCircle(nd.x, nd.y, mNodeRadius, nodePaint);
                String displayText = nd.title;
                if (displayText.length() > 10) displayText =
                    displayText.substring(0, 9) + "…";
                canvas.drawText(displayText, nd.x, nd.y + 12, textPaint);
            }
            canvas.restore();
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            float x = event.getX();
            float y = event.getY();
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mLastTouchX = x;
                    mLastTouchY = y;
                    mIsDragging = false;
                    mSelectedNode = null;
                    for (Node nd : mNodeList) {
                        float nx = nd.x + mOffsetX;
                        float ny = nd.y + mOffsetY;
                        float dist = (float) Math.hypot(x - nx, y - ny);
                        if (dist < mNodeRadius) {
                            mSelectedNode = nd;
                            if (mNodeSelectListener != null) {
                                mNodeSelectListener.onSelect(nd.filePath);
                            }
                            break;
                        }
                    }
                    invalidate();
                    break;
                case MotionEvent.ACTION_MOVE:
                    float dx = x - mLastTouchX;
                    float dy = y - mLastTouchY;
                    if (Math.abs(dx) > 8 || Math.abs(dy) > 8) {
                        mIsDragging = true;
                    }
                    if (mIsDragging) {
                        mOffsetX += dx;
                        mOffsetY += dy;
                        invalidate();
                    }
                    mLastTouchX = x;
                    mLastTouchY = y;
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    break;
            }
            return true;
        }

        private static float dpRaw(int dpVal) {
            return (
                dpVal *
                android.content.res.Resources.getSystem()
                    .getDisplayMetrics()
                    .density
            );
        }
    }
}
