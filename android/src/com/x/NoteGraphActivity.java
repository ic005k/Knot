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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInstance = this;
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
        // ========== 修复：按钮水平居中 ==========
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
            String jsonStr = args.get(1);
            GraphData graphData = new Gson().fromJson(jsonStr, GraphData.class);
            mGraphView.setGraphData(graphData);
            // 节点选中回调
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

    // ========== 自定义绘图View，支持拖拽、点击选中节点 ==========
    public static class GraphView extends View {

        private GraphData mData;
        private OnNodeSelectListener mNodeSelectListener;
        // 画布偏移（拖拽）
        private float mOffsetX = 0f;
        private float mOffsetY = 0f;
        private float mLastTouchX;
        private float mLastTouchY;
        private boolean mIsDragging;

        // 保存节点 + 对应link方向
        private static class Node {

            float x;
            float y;
            String title;
            String filePath;
            String dir; // in / out / both
        }

        private final List<Node> mNodeList = new ArrayList<>();
        private Node mSelectedNode = null;
        private final float mNodeRadius = dpRaw(24);
        private final float arrowSize = dpRaw(8);
        // 标记：数据是否等待布局完成后重新排布
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

        public void setGraphData(GraphData data) {
            mData = data;
            mNeedLayoutNodes = true;
            mNodeList.clear();
            invalidate();
        }

        // 节点布局计算：在onLayout拿到真实宽高后执行
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
            centerNode.title = mData.current;
            centerNode.filePath = mData.current;
            centerNode.dir = "";
            mNodeList.add(centerNode);

            // 环绕排布关联节点，环形布局，radius放大
            if (mData.links != null) {
                int count = mData.links.size();
                float radius = 420; // 调大这个值，图谱整体放大，320→420
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

        // 绘制箭头工具函数
        private void drawArrow(
            android.graphics.Canvas canvas,
            android.graphics.Paint paint,
            float fromX,
            float fromY,
            float toX,
            float toY,
            boolean reverse
        ) {
            float dx = toX - fromX;
            float dy = toY - fromY;
            float len = (float) Math.hypot(dx, dy);
            if (len < 1) return;
            float nx = dx / len;
            float ny = dy / len;
            float tipX, tipY;
            if (reverse) {
                tipX = fromX + nx * mNodeRadius;
                tipY = fromY + ny * mNodeRadius;
            } else {
                tipX = toX - nx * mNodeRadius;
                tipY = toY - ny * mNodeRadius;
            }
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
            // 绘制连接线，区分 dir
            for (int i = 1; i < mNodeList.size(); i++) {
                Node nd = mNodeList.get(i);
                String dir = nd.dir;
                if ("out".equals(dir)) {
                    linePaint.setColor(0xFF4CAF50); //绿色：中心向外
                    canvas.drawLine(center.x, center.y, nd.x, nd.y, linePaint);
                    drawArrow(
                        canvas,
                        linePaint,
                        center.x,
                        center.y,
                        nd.x,
                        nd.y,
                        false
                    );
                } else if ("in".equals(dir)) {
                    linePaint.setColor(0xFFF44336); //红色：指向中心
                    canvas.drawLine(nd.x, nd.y, center.x, center.y, linePaint);
                    drawArrow(
                        canvas,
                        linePaint,
                        nd.x,
                        nd.y,
                        center.x,
                        center.y,
                        false
                    );
                } else if ("both".equals(dir)) {
                    linePaint.setColor(0xFFFFEB3B); //黄色双向
                    canvas.drawLine(center.x, center.y, nd.x, nd.y, linePaint);
                    drawArrow(
                        canvas,
                        linePaint,
                        center.x,
                        center.y,
                        nd.x,
                        nd.y,
                        false
                    );
                    drawArrow(
                        canvas,
                        linePaint,
                        nd.x,
                        nd.y,
                        center.x,
                        center.y,
                        false
                    );
                }
            }
            // 绘制所有节点
            for (Node nd : mNodeList) {
                if (nd == mSelectedNode) {
                    nodePaint.setColor(0xFF42A5F5);
                } else if (nd == center) {
                    nodePaint.setColor(0xFFFFC107);
                } else {
                    nodePaint.setColor(0xFF444444);
                }
                canvas.drawCircle(nd.x, nd.y, mNodeRadius, nodePaint);
                // 节点文字（截断）
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
                    // 点击检测节点
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
