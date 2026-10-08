package com.x.artifex.mupdf.viewer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.PointF;
import android.util.Log;
import com.artifex.mupdf.fitz.Cookie;
import com.artifex.mupdf.fitz.DisplayList;
import com.artifex.mupdf.fitz.Document;
import com.artifex.mupdf.fitz.Link;
import com.artifex.mupdf.fitz.Matrix;
import com.artifex.mupdf.fitz.Outline;
import com.artifex.mupdf.fitz.Page;
import com.artifex.mupdf.fitz.Quad;
import com.artifex.mupdf.fitz.Rect;
import com.artifex.mupdf.fitz.RectI;
import com.artifex.mupdf.fitz.SeekableInputStream;
import com.artifex.mupdf.fitz.android.AndroidDrawDevice;
import java.util.ArrayList;

public class MuPDFCore {

    private final String APP = "MuPDF";

    private boolean mInvertMode = false;

    private final int MAXIMUM_OUTLINE_ITEMS = 1000;
    private final int MAXIMUM_OUTLINE_DEPTH = 4;

    private int resolution;
    private Document doc;
    private Outline[] outline;
    private int pageCount = -1;
    private boolean reflowable = false;
    private int currentPage;
    private Page page;
    private float pageWidth;
    private float pageHeight;
    private DisplayList displayList;

    /* Default to "A Format" pocket book size. */
    private int layoutW = 312;
    private int layoutH = 504;
    private int layoutEM = 10;

    private boolean outlineTruncated;

    private MuPDFCore(Document doc) {
        this.doc = doc;
        doc.layout(layoutW, layoutH, layoutEM);
        pageCount = doc.countPages();
        reflowable = doc.isReflowable();
        resolution = 160;
        currentPage = -1;
    }

    public MuPDFCore(byte[] buffer, String magic) {
        this(Document.openDocument(buffer, magic));
    }

    public MuPDFCore(SeekableInputStream stm, String magic) {
        this(Document.openDocument(stm, magic));
    }

    public String getTitle() {
        return doc.getMetaData(Document.META_INFO_TITLE);
    }

    public int countPages() {
        return pageCount;
    }

    public boolean isReflowable() {
        return reflowable;
    }

    public synchronized int layout(int oldPage, int w, int h, int em) {
        if (w != layoutW || h != layoutH || em != layoutEM) {
            System.out.println("LAYOUT: " + w + "," + h);
            layoutW = w;
            layoutH = h;
            layoutEM = em;
            long mark = doc.makeBookmark(doc.locationFromPageNumber(oldPage));
            doc.layout(layoutW, layoutH, layoutEM);
            currentPage = -1;
            pageCount = doc.countPages();
            outline = null;
            try {
                outline = doc.loadOutline();
            } catch (Exception ex) {
                /* ignore error */
            }
            return doc.pageNumberFromLocation(doc.findBookmark(mark));
        }
        return oldPage;
    }

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
            currentPage = -1;

            if (doc != null) {
                page = doc.loadPage(pageNum);
                Rect b = page.getBounds();
                pageWidth = b.x1 - b.x0;
                pageHeight = b.y1 - b.y0;
            }

            currentPage = pageNum;
        }
    }

    public synchronized PointF getPageSize(int pageNum) {
        gotoPage(pageNum);
        return new PointF(pageWidth, pageHeight);
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

        if (displayList == null && page != null) try {
            displayList = page.toDisplayList();
        } catch (Exception ex) {
            displayList = null;
        }

        if (displayList == null || page == null) return;

        float zoom = resolution / 72;
        Matrix ctm = new Matrix(zoom, zoom);
        RectI bbox = new RectI(page.getBounds().transform(ctm));
        float xscale = (float) pageW / (float) (bbox.x1 - bbox.x0);
        float yscale = (float) pageH / (float) (bbox.y1 - bbox.y0);
        ctm.scale(xscale, yscale);

        AndroidDrawDevice dev = new AndroidDrawDevice(bm, patchX, patchY);
        try {
            displayList.run(dev, ctm, cookie);
            dev.close();
        } finally {
            dev.destroy();
        }

        // ✅ 渲染完成后，在后台线程直接做暖灰后处理
        if (mInvertMode) {
            applyWarmGray(bm);
        }
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

    public synchronized Link[] getPageLinks(int pageNum) {
        gotoPage(pageNum);
        return page != null ? page.getLinks() : null;
    }

    public synchronized int resolveLink(Link link) {
        return doc.pageNumberFromLocation(doc.resolveLink(link));
    }

    public synchronized Quad[][] searchPage(int pageNum, String text) {
        gotoPage(pageNum);
        return page.search(text);
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
        ArrayList<OutlineActivity.Item> result,
        Outline[] list,
        String indent,
        int depth
    ) {
        for (Outline node : list) {
            if (node.title != null) {
                int page = doc.pageNumberFromLocation(doc.resolveLink(node));
                if (result.size() >= MAXIMUM_OUTLINE_ITEMS) outlineTruncated =
                    true;
                else result.add(
                    new OutlineActivity.Item(indent + node.title, page)
                );
            }
            if (node.down != null) {
                if (
                    depth >= MAXIMUM_OUTLINE_DEPTH ||
                    result.size() >= MAXIMUM_OUTLINE_ITEMS
                ) outlineTruncated = true;
                else flattenOutlineNodes(
                    result,
                    node.down,
                    indent + "    ",
                    depth + 1
                );
            }
        }
    }

    public synchronized ArrayList<OutlineActivity.Item> getOutline() {
        outlineTruncated = false;
        ArrayList<OutlineActivity.Item> result =
            new ArrayList<OutlineActivity.Item>();
        flattenOutlineNodes(result, outline, "", 0);
        return result;
    }

    public synchronized boolean wasOutlineTruncated() {
        return outlineTruncated;
    }

    public synchronized boolean needsPassword() {
        return doc.needsPassword();
    }

    public synchronized boolean authenticatePassword(String password) {
        boolean authenticated = doc.authenticatePassword(password);
        pageCount = doc.countPages();
        reflowable = doc.isReflowable();
        return authenticated;
    }

    // ✅ 保留原有 setter（DocumentActivity 已调用此方法）
    public void setInvertMode(boolean invert) {
        mInvertMode = invert;
    }

    public boolean isInvertMode() {
        return mInvertMode;
    }

    // ✅ 暖灰算法：直接移植旧版 invertBitmap 的三区间映射
    // 放在 drawPage 末尾，渲染完 bitmap 后立即执行
    private void applyWarmGray(Bitmap bm) {
        if (bm == null || bm.isRecycled()) return;

        int w = bm.getWidth();
        int h = bm.getHeight();
        int[] pixels = new int[w * h];
        bm.getPixels(pixels, 0, w, 0, 0, w, h);

        // 与旧版完全一致的参数
        final int TARGET_R = 0xB8; // 184
        final int TARGET_G = 0xA8; // 168
        final int TARGET_B = 0x90; // 144
        final float TEXT_THRESHOLD = 95f / 255f;
        final float BG_THRESHOLD = 200f / 255f;

        for (int i = 0; i < pixels.length; i++) {
            int px = pixels[i];
            int a = (px >> 24) & 0xFF;
            int r = (px >> 16) & 0xFF;
            int g = (px >> 8) & 0xFF;
            int b = px & 0xFF;

            float lum = (0.299f * r + 0.587f * g + 0.114f * b) / 255f;

            int nr, ng, nb;
            if (lum >= BG_THRESHOLD) {
                // 背景 → 纯黑
                nr = ng = nb = 0;
            } else if (lum <= TEXT_THRESHOLD) {
                // 文字 → 暖灰
                float ratio = 1.0f - lum / TEXT_THRESHOLD;
                float scale = 0.7f + 0.3f * ratio;
                nr = (int) (TARGET_R * scale);
                ng = (int) (TARGET_G * scale);
                nb = (int) (TARGET_B * scale);
            } else {
                // 过渡区 → 平滑插值
                float edge =
                    (lum - TEXT_THRESHOLD) / (BG_THRESHOLD - TEXT_THRESHOLD);
                float scale = 0.7f * (1.0f - edge);
                nr = (int) (TARGET_R * scale);
                ng = (int) (TARGET_G * scale);
                nb = (int) (TARGET_B * scale);
            }

            pixels[i] = (a << 24) | (nr << 16) | (ng << 8) | nb;
        }

        bm.setPixels(pixels, 0, w, 0, 0, w, h);
    }
}
