package com.x.reader;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.PointF;
import android.os.AsyncTask;
import android.util.Log;
import android.util.SparseArray;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class PageAdapter extends BaseAdapter {

    private final String APP = "MuPDF";
    private final Context mContext;
    private final MuPDFCore mCore;
    private final SparseArray<PointF> mPageSizes = new SparseArray<PointF>();
    private Bitmap mSharedHqBm1;
    private Bitmap mSharedHqBm2;

    private boolean mFitWidth = false;

    // ✅ layout 版本号，每次 refresh() 递增
    // 用于让旧 AsyncTask 的 onPostExecute 自动失效
    private int mLayoutVersion = 0;

    public PageAdapter(Context c, MuPDFCore core) {
        mContext = c;
        mCore = core;
    }

    // ✅ setter
    public void setFitWidth(boolean fitWidth) {
        mFitWidth = fitWidth;
    }

    public int getCount() {
        try {
            return mCore.countPages();
        } catch (RuntimeException e) {
            return 0;
        }
    }

    public Object getItem(int position) {
        return null;
    }

    public long getItemId(int position) {
        return 0;
    }

    public synchronized void releaseBitmaps() {
        //  recycle and release the shared bitmap.
        if (mSharedHqBm1 != null) mSharedHqBm1.recycle();
        mSharedHqBm1 = null;
        if (mSharedHqBm2 != null) mSharedHqBm2.recycle();
        mSharedHqBm2 = null;
    }

    public void refresh() {
        mPageSizes.clear();
        mLayoutVersion++; // ✅ 使所有飞行中的旧 sizingTask 失效
    }

    public synchronized View getView(
        final int position,
        View convertView,
        ViewGroup parent
    ) {
        final PageView pageView;
        if (convertView == null) {
            if (
                mSharedHqBm1 == null ||
                mSharedHqBm1.getWidth() != parent.getWidth() ||
                mSharedHqBm1.getHeight() != parent.getHeight()
            ) {
                if (parent.getWidth() > 0 && parent.getHeight() > 0) {
                    mSharedHqBm1 = Bitmap.createBitmap(
                        parent.getWidth(),
                        parent.getHeight(),
                        Bitmap.Config.ARGB_8888
                    );
                    mSharedHqBm2 = Bitmap.createBitmap(
                        parent.getWidth(),
                        parent.getHeight(),
                        Bitmap.Config.ARGB_8888
                    );
                } else {
                    mSharedHqBm1 = null;
                    mSharedHqBm2 = null;
                }
            }

            pageView = new PageView(
                mContext,
                mCore,
                new Point(parent.getWidth(), parent.getHeight()),
                mSharedHqBm1,
                mSharedHqBm2
            );
        } else {
            pageView = (PageView) convertView;
        }

        // ✅ 每次 getView 都同步标记（处理复用场景）
        pageView.setFitWidth(mFitWidth);

        PointF pageSize = mPageSizes.get(position);
        if (pageSize != null) {
            // We already know the page size. Set it up
            // immediately
            pageView.setPage(position, pageSize);
        } else {
            // Page size as yet unknown. Blank it for now, and
            // start a background task to find the size
            pageView.blank(position);

            // ✅ 捕获当前版本号
            final int versionAtCreation = mLayoutVersion;

            AsyncTask<Void, Void, PointF> sizingTask = new AsyncTask<
                Void,
                Void,
                PointF
            >() {
                @Override
                protected PointF doInBackground(Void... arg0) {
                    try {
                        return mCore.getPageSize(position);
                    } catch (RuntimeException e) {
                        return null;
                    }
                }

                @Override
                protected void onPostExecute(PointF result) {
                    super.onPostExecute(result);

                    // ✅ 【关键】如果 layout 已刷新，丢弃这个过时结果
                    if (versionAtCreation != mLayoutVersion) return;

                    // We now know the page size
                    mPageSizes.put(position, result);
                    // Check that this view hasn't been reused for
                    // another page since we started
                    if (pageView.getPage() == position) pageView.setPage(
                        position,
                        result
                    );
                }
            };

            sizingTask.execute((Void) null);
        }
        return pageView;
    }
}
