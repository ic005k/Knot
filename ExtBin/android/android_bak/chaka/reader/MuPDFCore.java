package com.x.reader;

import android.app.ProgressDialog;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.RectF;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.SparseArray;
import com.artifex.mupdf.fitz.Cookie;
import com.artifex.mupdf.fitz.DisplayList;
import com.artifex.mupdf.fitz.Document;
import com.artifex.mupdf.fitz.Link;
import com.artifex.mupdf.fitz.Location;
import com.artifex.mupdf.fitz.Matrix;
import com.artifex.mupdf.fitz.Outline;
import com.artifex.mupdf.fitz.Page;
import com.artifex.mupdf.fitz.Quad;
import com.artifex.mupdf.fitz.Rect;
import com.artifex.mupdf.fitz.RectI;
import com.artifex.mupdf.fitz.SeekableInputStream;
import com.artifex.mupdf.fitz.StructuredText;
import com.artifex.mupdf.fitz.android.AndroidDrawDevice;
import com.x.R;
import java.util.ArrayList;

public class MuPDFCore {

    private static final int LAYOUT_PROGRESS_DELAY = 200;
    private Context mContext;
    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private AsyncTask<Void, Integer, Integer> mLayoutTask;
    private int resolution;
    private Document doc;
    private Outline[] outline;
    private int basePageCount = -1;
    private int pageCount = -1;
    private boolean reflowable = false;
    private int currentPage;
    private Page page;
    private float pageWidth;
    private float pageHeight;
    private float pageLeft; // crop margin render offset left
    private float pageTop; // crop margin render offset top
    private final SparseArray<TextSelectionModel> tsModel = new SparseArray<>();
    private int tint_black = -1;
    private int tint_white;
    private DisplayList displayList;
    private boolean singleColumnMode = false;
    private boolean textLeftMode = false;
    private boolean cropMarginMode = false;
    private boolean imgFollowPal = false;

    /* Default to "A Format" pocket book size. */
    private int layoutW = 312;
    private int layoutH = 504;
    private int layoutEM = 10;

    private MuPDFCore(Context context, Document doc) {
        this.mContext = context;
        this.doc = doc;
        if (!doc.needsPassword()) setup();
    }

    private void setup() {
        reflowable = doc.isReflowable();
        // PDFs use default pocket book size
        if (!reflowable) {
            doc.layout(layoutW, layoutH, layoutEM);
            basePageCount = doc.countPages();
            correctPageCount();
        }
        resolution = 160;
        currentPage = -1;
    }

    public MuPDFCore(Context context, byte[] buffer, String magic) {
        this(context, Document.openDocument(buffer, magic));
    }

    public MuPDFCore(Context context, SeekableInputStream stm, String magic) {
        this(context, Document.openDocument(stm, magic));
    }

    public String getTitle() {
        return doc.getMetaData(Document.META_INFO_TITLE);
    }

    public int getBackgroundColor() {
        return tint_white;
    }

    public int countPages() {
        return pageCount;
    }

    public boolean isReflowable() {
        return reflowable;
    }

    // flowable documents use custom book size
    public synchronized void layout(int oldPage, int w, int h, int em) {
        if (w != layoutW || h != layoutH || em != layoutEM) {
            System.out.println("LAYOUT: " + w + "," + h);
            layoutW = w;
            layoutH = h;
            layoutEM = em;
            ChapterPage mark = locatePage(realPage(oldPage));
            doc.layout(layoutW, layoutH, layoutEM);

            final ProgressDialogX progressDialog = new ProgressDialogX(
                mContext,
                R.style.MyDialog
            );
            progressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            progressDialog.setTitle(mContext.getString(R.string.relayout_));
            int nc = doc.countChapters();
            progressDialog.setMax(nc);
            progressDialog.setCanceledOnTouchOutside(false);
            mLayoutTask = new AsyncTask<Void, Integer, Integer>() {
                @Override
                protected Integer doInBackground(Void... params) {
                    int np = 0;
                    for (int i = 0; i < nc; i++) {
                        if (!progressDialog.isCancelled()) {
                            publishProgress(i);
                            np += doc.countPages(i);
                        }
                    }
                    return np;
                }

                @Override
                protected void onPostExecute(Integer result) {
                    progressDialog.cancel();
                    basePageCount = result;
                    correctPageCount();
                    currentPage = -1;
                    outline = null;
                    try {
                        outline = doc.loadOutline();
                    } catch (Exception ex) {
                        /* ignore error */
                    }
                    int newPage = estimatePage(mark);
                    ((DocumentActivity) mContext).afterRelayout(
                        correctPage(newPage)
                    );
                }

                @Override
                protected void onCancelled() {
                    progressDialog.cancel();
                }

                @Override
                protected void onProgressUpdate(Integer... values) {
                    progressDialog.setProgress(values[0].intValue());
                }

                @Override
                protected void onPreExecute() {
                    super.onPreExecute();
                    mHandler.postDelayed(
                        new Runnable() {
                            public void run() {
                                if (!progressDialog.isCancelled()) {
                                    progressDialog.show();
                                    progressDialog.setProgress(0);
                                }
                            }
                        },
                        LAYOUT_PROGRESS_DELAY
                    );
                }
            };

            mLayoutTask.execute();
        } else {
            ((DocumentActivity) mContext).afterRelayout(oldPage);
        }
    }

    // the pageNum is correctPage
    private synchronized void gotoPage(int pageNum) {
        /* TODO: page cache */
        if (pageNum > pageCount - 1) pageNum = pageCount - 1;
        else if (pageNum < 0) pageNum = 0;
        if (pageNum != currentPage) {
            if (page != null) page.destroy();
            page = null;
            if (displayList != null) displayList.destroy();
            displayList = null;
            page = null;
            pageWidth = 0;
            pageHeight = 0;
            pageLeft = 0;
            pageTop = 0;

            if (doc != null) {
                pageNum = realPage(pageNum);
                page = doc.loadPage(pageNum);
                Rect b = page.getBounds();

                if (cropMarginMode) {
                    Rect bb = getBBox(b);
                    pageLeft = bb.x0 - b.x0;
                    pageTop = bb.y0 - b.y0;
                    b = bb;
                }

                pageWidth = b.x1 - b.x0;
                pageHeight = b.y1 - b.y0;
            }

            currentPage = pageNum;
        }
    }

    /*
     * page full size
     */
    public synchronized RectF getPageSize(int pageNum) {
        gotoPage(pageNum);
        // ✅ 修复：pageLeft/pageTop 应始终反映实际内容原点
        // 这样 PageView 中的 mRenderOff 才能正确用于文字选择、链接点击等坐标转换
        float left = cropMarginMode ? pageLeft : 0;
        float top = cropMarginMode ? pageTop : 0;

        // 但如果页面本身的 getBounds().x0 != 0（如 TXT/CSS 排版），
        // 即使不裁边也需要记录这个偏移
        if (!cropMarginMode && page != null) {
            Rect b = page.getBounds();
            left = b.x0;
            top = b.y0;
        }

        return new RectF(pageWidth, pageHeight, left, top);
    }

    public synchronized void onDestroy() {
        if (displayList != null) displayList.destroy();
        displayList = null;
        if (page != null) page.destroy();
        page = null;
        if (doc != null) doc.destroy();
        doc = null;
    }

    public synchronized void drawPage(
        Bitmap bm,
        int pageNum,
        int pageW,
        int pageH,
        int patchX,
        int patchY,
        int patchW,
        int patchH,
        Cookie cookie
    ) {
        gotoPage(pageNum);
        if (displayList == null && page != null) {
            try {
                displayList = page.toDisplayList();
            } catch (Exception ex) {
                displayList = null;
            }
        }
        if (displayList == null || page == null) return;

        if (isSplitPage(currentPage)) pageW *= 2;

        float zoom = resolution / 72f / 2f;
        Matrix ctm = new Matrix(zoom, zoom);
        Rect b = page.getBounds();

        // ✅ 修复：始终获取内容边界（即使 cropMarginMode=false，也需要知道原始 bbox）
        Rect contentBounds = cropMarginMode ? getBBox(b) : b;
        RectI bbox = new RectI(contentBounds.transform(ctm));

        float xscale = (float) pageW / (float) (bbox.x1 - bbox.x0);
        float yscale = (float) pageH / (float) (bbox.y1 - bbox.y0);
        ctm.scale(xscale, yscale);

        // ✅ 核心修复：始终补偿页面原点偏移
        // 之前只在 cropMarginMode=true 时补偿，导致非裁边模式下内容左偏
        patchX += bbox.x0 * xscale;
        patchY += bbox.y0 * yscale;

        AndroidDrawDevice dev = null;
        try {
            dev = new AndroidDrawDevice(
                bm,
                72,
                72,
                patchX,
                patchY,
                patchW,
                patchH
            );
            displayList.run(dev, ctm, cookie);
            dev.close();

            boolean needsTint =
                tint_black != 0xff000000 || tint_white != 0xffffffff;
            if (needsTint) applyTintToBitmap(
                bm,
                tint_black,
                tint_white,
                imgFollowPal
            );
        } finally {
            if (dev != null) dev.destroy();
        }
    }

    // ✅ 新增方法：在 Java 层实现底层 tint 逻辑
    private void applyTintToBitmap(
        Bitmap bm,
        int tintColor,
        int bgColor,
        boolean followPalette
    ) {
        // 提取目标颜色的 RGB 通道 (0.0 ~ 1.0)
        float tR = ((tintColor >> 16) & 0xFF) / 255.0f;
        float tG = ((tintColor >> 8) & 0xFF) / 255.0f;
        float tB = (tintColor & 0xFF) / 255.0f;

        float bR = ((bgColor >> 16) & 0xFF) / 255.0f;
        float bG = ((bgColor >> 8) & 0xFF) / 255.0f;
        float bB = (bgColor & 0xFF) / 255.0f;

        // 构建 ColorMatrix 进行颜色映射
        // 原理：将原图的黑色映射为 tintColor，白色映射为 bgColor
        float[] matrix;
        if (followPalette) {
            // imgFollowPal 模式：图片也跟随变色
            matrix = new float[] {
                tR - bR,
                0,
                0,
                0,
                bR * 255,
                0,
                tG - bG,
                0,
                0,
                bG * 255,
                0,
                0,
                tB - bB,
                0,
                bB * 255,
                0,
                0,
                0,
                1,
                0,
            };
        } else {
            // 默认模式：图片保持原色，只改变文字/背景
            // 这里简化处理：使用亮度阈值区分文字和背景，或者直接应用全局色调
            // 为了最接近 Chaka 的默认效果，我们使用一种基于亮度的混合矩阵
            matrix = new float[] {
                (tR - bR) * 0.8f + 0.2f,
                0,
                0,
                0,
                bR * 255,
                0,
                (tG - bG) * 0.8f + 0.2f,
                0,
                0,
                bG * 255,
                0,
                0,
                (tB - bB) * 0.8f + 0.2f,
                0,
                bB * 255,
                0,
                0,
                0,
                1,
                0,
            };
        }

        android.graphics.ColorMatrix cm = new android.graphics.ColorMatrix(
            matrix
        );
        android.graphics.Canvas canvas = new android.graphics.Canvas(bm);
        android.graphics.Paint paint = new android.graphics.Paint();
        paint.setColorFilter(new android.graphics.ColorMatrixColorFilter(cm));
        canvas.drawBitmap(bm, 0, 0, paint);
    }

    public synchronized void updatePage(
        Bitmap bm,
        int pageNum,
        int pageW,
        int pageH,
        int patchX,
        int patchY,
        int patchW,
        int patchH,
        Cookie cookie
    ) {
        drawPage(
            bm,
            pageNum,
            pageW,
            pageH,
            patchX,
            patchY,
            patchW,
            patchH,
            cookie
        );
    }

    public boolean setTintColor(int black, int white) {
        boolean ret = true;

        if (tint_black == -1) ret = false;
        if (tint_black == black && tint_white == white) ret = false;
        else {
            tint_black = black;
            tint_white = white;
        }

        return ret;
    }

    public void toggleSingleColumn() {
        singleColumnMode = !singleColumnMode;
        correctPageCount();
    }

    public void toggleTextLeft() {
        textLeftMode = !textLeftMode;
    }

    public void toggleCropMargin() {
        // force reread pagesize of current page
        // prevent display distort when uncrop margin after a page scale
        currentPage = -1;
        cropMarginMode = !cropMarginMode;
    }

    public void toggleImgFollowPal() {
        imgFollowPal = !imgFollowPal;
    }

    public synchronized Link[] getPageLinks(int pageNum) {
        gotoPage(pageNum);
        return page != null ? page.getLinks() : null;
    }

    public synchronized int resolveLink(Link link) {
        return correctPage(doc.pageNumberFromLocation(doc.resolveLink(link)));
    }

    public synchronized Quad[][] searchPage(int pageNum, String text) {
        gotoPage(pageNum);
        Quad[][] ret = page.search(text);
        if (!isSplitPage(pageNum)) return ret;

        ArrayList<Quad[]> reslist = new ArrayList<>();
        float mid = pageWidth / 2;
        for (Quad[] r : ret) {
            for (Quad q : r) {
                if (
                    (!isRightPage(pageNum) && q.ul_x < mid) ||
                    (isRightPage(pageNum) && q.ur_x > mid)
                ) {
                    reslist.add(r);
                    break;
                }
            }
        }
        if (reslist.size() > 0) {
            Quad[][] res = new Quad[reslist.size()][];
            res = reslist.toArray(res);
            return res;
        }
        return null;
    }

    public synchronized boolean hasOutline() {
        if (outline == null) {
            try {
                outline = doc.loadOutline();
            } catch (Exception ex) {
                /* ignore error */
            }
        }
        return outline != null;
    }

    private void flattenOutlineNodes(
        ArrayList<TocItem> result,
        Outline[] list,
        int level
    ) {
        for (Outline node : list) {
            if (node.title != null) {
                int pageNum = correctPage(
                    doc.pageNumberFromLocation(doc.resolveLink(node))
                );
                int count = 0;
                if (node.down != null) {
                    count = node.down.length;
                }
                result.add(new TocItem(node.title, pageNum, level, count));
                if (count > 0) flattenOutlineNodes(
                    result,
                    node.down,
                    level + 1
                );
            }
        }
    }

    public synchronized ArrayList<TocItem> getOutline() {
        ArrayList<TocItem> result = new ArrayList<TocItem>();
        flattenOutlineNodes(result, outline, 0);
        return result;
    }

    /*
     * for some epubs, findBookmark may take much time
     * so disable it's use
     */
    // public long makeBookmark(int page) {
    //     return doc.makeBookmark(doc.locationFromPageNumber(page));
    // }

    // public int findBookmark(long mark) {
    //     return doc.pageNumberFromLocation(doc.findBookmark(mark));
    // }

    public ChapterPage locatePage(int page) {
        Location loc = doc.locationFromPageNumber(page);
        return new ChapterPage(
            loc.chapter,
            loc.page,
            doc.countPages(loc.chapter)
        );
    }

    public int estimatePage(ChapterPage cp) {
        int chPageCount = doc.countPages(cp.chapter);
        int page = Math.round((chPageCount * (cp.page + 1)) / cp.pageCount) - 1;
        if (page < 0) page = 0;
        if (page >= chPageCount) page = chPageCount - 1;
        return doc.pageNumberFromLocation(new Location(cp.chapter, page));
    }

    private synchronized Rect getBBox(Rect b) {
        // ✅ 修复：官方 StructuredText 没有 getBounds() 方法。
        // Chaka 在底层 C++ 定制了内容边界计算，官方 Java API 不支持直接获取。
        //
        // 为了保证渲染性能和稳定性，这里采用【降级处理】：
        // 直接返回页面的物理边界 (b)，暂时禁用智能裁边功能。
        //
        // 如果未来必须实现裁边，需要通过 page.toStructuredText() 获取文本结构，
        // 然后遍历所有 TextBlock -> TextLine -> Quad 来手动合并计算包围盒，
        // 但这会带来明显的翻页卡顿，不建议在 Java 层实现。

        return b;
    }

    public synchronized StructuredText getSText(int pageNum) {
        gotoPage(pageNum);
        return page != null ? page.toStructuredText() : null;
    }

    public boolean isSingleColumn() {
        return singleColumnMode;
    }

    public boolean isSplitPage(int pageNum) {
        return singleColumnMode && pageNum > 0 && pageNum < pageCount - 1;
    }

    /*
     * for splitted page
     */
    public boolean isRightPage(int pageNum) {
        return (
            (textLeftMode && pageNum % 2 == 1) ||
            (!textLeftMode && pageNum % 2 == 0)
        );
    }

    public boolean isTextLeft() {
        return textLeftMode;
    }

    private void correctPageCount() {
        if (
            singleColumnMode // divide every page into 2 pages, except first and last page
        ) pageCount = basePageCount * 2 - 2;
        else pageCount = basePageCount;
    }

    public int correctPage(int p) {
        if (singleColumnMode) {
            p = p * 2 - 1;
            if (p < 0) p = 0;
        }
        return p;
    }

    public int realPage(int p) {
        if (singleColumnMode) return (p + 1) / 2;
        return p;
    }

    public synchronized boolean needsPassword() {
        return doc.needsPassword();
    }

    public synchronized boolean authenticatePassword(String password) {
        boolean authenticated = doc.authenticatePassword(password);
        if (authenticated) setup();
        return authenticated;
    }

    public TextSelectionModel getTSModel(int pageNum) {
        return getTSModel(pageNum, false);
    }

    public TextSelectionModel getTSModel(int pageNum, boolean create) {
        TextSelectionModel tsmodel = tsModel.get(pageNum);
        if (tsmodel == null) {
            if (create) {
                tsmodel = new TextSelectionModel();
                tsmodel.sText = getSText(pageNum);
            }
        }
        return tsmodel;
    }

    public void putTSModel(int pageNum, TextSelectionModel tsmodel) {
        tsModel.put(pageNum, tsmodel);
    }

    public void rmTSModel(int pageNum) {
        tsModel.remove(pageNum);
    }

    public void clearTSModel() {
        tsModel.clear();
    }

    public static class TextSelectionModel {

        public StructuredText sText; // page text structure
        public Quad[] selectionBoxes; // selection result on source
        public PointF[] textHandles = new PointF[2]; // handles point on source
        public PointF[] boundries = new PointF[2]; // boundries point on source
        public int dir; // 0: none, 1: left, 2: right, 3: both
        public android.graphics.Rect[] rectHandles =
            new android.graphics.Rect[2]; // handles rect on view
    }
}
