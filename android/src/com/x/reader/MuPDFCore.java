package com.x.reader;

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

    // ✅ 固定排版参数（与 DocumentActivity 中的常量保持一致）
    private static final float FIXED_LETTER_SPACING = 0.08f;
    private static final float FIXED_LINE_HEIGHT = 1.65f;

    private MuPDFCore(Document doc) {
        this.doc = doc;

        // ✅ 必须先获取 reflowable 状态，applyFixedSpacing() 内部依赖此变量
        reflowable = doc.isReflowable();

        // ✅ 首次 layout 前注入 CSS
        applyFixedSpacing();

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

            // ✅ 新增：每次重新排版前注入 CSS（确保全局 CSS 状态最新）
            applyFixedSpacing();

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
    private void applyWarmGray(Bitmap bm) {
        if (bm == null || bm.isRecycled()) return;

        int w = bm.getWidth();
        int h = bm.getHeight();
        int[] pixels = new int[w * h];
        bm.getPixels(pixels, 0, w, 0, 0, w, h);

        // ✅ 降低全黑环境下的文字亮度
        // 旧值: R=0xB8(184) G=0xA8(168) B=0x90(144) → 感知亮度 ~170
        // 新值: R=0x80(128) G=0x74(116) B=0x64(100) → 感知亮度 ~118
        // 对比度 (118+5)/(0+5) ≈ 24.6:1，仍远高于 WCAG AA 4.5:1 要求
        // 但绝对亮度降低 35%，全黑环境下不再刺眼
        final int TARGET_R = 0x80; // 128 (was 184)
        final int TARGET_G = 0x74; // 116 (was 168)
        final int TARGET_B = 0x64; // 100 (was 144)

        // ✅ 放宽文字判定阈值，让更多"深灰"像素也走暖灰映射
        // 避免阈值边缘出现突兀的亮度跳变
        final float TEXT_THRESHOLD = 110f / 255f; // was 95/255
        final float BG_THRESHOLD = 190f / 255f; // was 200/255

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
                // 文字 → 暖灰（降低后的目标色）
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

    /**
     * 提取指定页面的纯文本（用于 TTS）
     */
    public synchronized String getPageText(int pageNum) {
        gotoPage(pageNum);
        if (page == null) return "";

        try {
            // 使用 StructuredText 提取纯文本
            com.artifex.mupdf.fitz.StructuredText stext = page.toStructuredText(
                "preserve-whitespace"
            );
            String text = "";

            // 尝试调用 asText() (MuPDF 1.24+)
            try {
                java.lang.reflect.Method m = stext
                    .getClass()
                    .getMethod("asText");
                text = (String) m.invoke(stext);
            } catch (Exception ignored) {}

            // 降级尝试 toPlainText()
            if (text == null || text.isEmpty()) {
                try {
                    java.lang.reflect.Method m = stext
                        .getClass()
                        .getMethod("toPlainText");
                    text = (String) m.invoke(stext);
                } catch (Exception ignored) {}
            }

            stext.destroy();
            return text != null ? text.trim() : "";
        } catch (Exception e) {
            Log.e(APP, "getPageText failed: " + e.getMessage());
            return "";
        }
    }

    /**
     * 注入固定的字间距、行间距和段落缩进 CSS。
     * Context.setUserCSS 是进程级全局静态方法，对所有后续 doc.layout() 生效。
     * 仅在 reflowable 文档（如 EPUB、转换后的 TXT）上执行。
     */
    private void applyFixedSpacing() {
        if (!reflowable) return;

        try {
            String css = String.format(
                // 1. 全局盒模型重置：消除所有元素的默认外边距和内边距
                "* { " +
                    "  margin-left: 0 !important; " +
                    "  margin-right: 0 !important; " +
                    "  padding-left: 0 !important; " +
                    "  padding-right: 0 !important; " +
                    "} " +
                    // 2. body 强制全宽 + 消除 UA 默认 1em margin
                    "body { " +
                    "  margin: 0 !important; " +
                    "  padding: 0 !important; " +
                    "  width: 100%% !important; " +
                    "  max-width: none !important; " +
                    "  box-sizing: border-box !important; " +
                    "} " +
                    // 3. 块级元素强制撑满宽度
                    "p, div, section, article, blockquote, pre, ul, ol, li, table, h1, h2, h3, h4, h5, h6 { " +
                    "  width: 100%% !important; " +
                    "  max-width: none !important; " +
                    "  box-sizing: border-box !important; " +
                    "} " +
                    // 4. 段落首行缩进2字符
                    "p { " +
                    "  text-indent: 2em !important; " +
                    "} " +
                    // 5. 字间距和行高
                    "body, p, div, span, li { " +
                    "  letter-spacing: %.3fem !important; " +
                    "  line-height: %.3f !important; " +
                    "} " +
                    "h1, h2, h3, h4, h5, h6 { " +
                    "  line-height: 1.3 !important; " +
                    "} ",
                FIXED_LETTER_SPACING,
                FIXED_LINE_HEIGHT
            );

            // 使用反射调用，兼容不同 MuPDF 构建版本
            java.lang.reflect.Method method =
                com.artifex.mupdf.fitz.Context.class.getMethod(
                    "setUserCSS",
                    String.class
                );
            method.invoke(null, css);

            Log.i(
                APP,
                "Fixed spacing CSS applied (letter-spacing=" +
                    FIXED_LETTER_SPACING +
                    "em, line-height=" +
                    FIXED_LINE_HEIGHT +
                    ")"
            );
        } catch (NoSuchMethodException e) {
            Log.w(APP, "Context.setUserCSS not available in this MuPDF build");
        } catch (Exception e) {
            Log.w(APP, "Context.setUserCSS failed: " + e.getMessage());
        }
    }

    /**
     * 提取指定页面的纯文本（用于 TTS 和 笔记全文提取）
     * 已有的 getPageText 方法可直接复用，无需修改。
     */

    /**
     * 【笔记专用】获取长按位置附近的文字片段（用于高亮定位）
     * @param pageNum 页码
     * @param docX 文档坐标系 X
     * @param docY 文档坐标系 Y
     * @return 附近的文字片段，可能为 null
     */
    public synchronized String getNearText(
        int pageNum,
        float docX,
        float docY
    ) {
        gotoPage(pageNum);
        if (page == null) return null;
        try {
            com.artifex.mupdf.fitz.StructuredText stext = page.toStructuredText(
                "preserve-whitespace"
            );
            float r = 5f;
            com.artifex.mupdf.fitz.Point p1 = new com.artifex.mupdf.fitz.Point(
                docX - r,
                docY - r
            );
            com.artifex.mupdf.fitz.Point p2 = new com.artifex.mupdf.fitz.Point(
                docX + r,
                docY + r
            );
            Quad[] hlQuads = stext.highlight(p1, p2);

            String nearText = null;
            if (hlQuads != null && hlQuads.length > 0) {
                Quad nearest = hlQuads[0];
                float minDist = Float.MAX_VALUE;
                for (Quad q : hlQuads) {
                    float cx = (q.ul_x + q.lr_x) / 2f;
                    float cy = (q.ul_y + q.lr_y) / 2f;
                    float dist =
                        (cx - docX) * (cx - docX) + (cy - docY) * (cy - docY);
                    if (dist < minDist) {
                        minDist = dist;
                        nearest = q;
                    }
                }
                float lineTop = nearest.ul_y - 2f;
                float lineBottom = nearest.lr_y + 2f;
                float lineLeft = Math.max(0, nearest.ul_x - 200f);
                float lineRight = nearest.lr_x + 200f;
                com.artifex.mupdf.fitz.Point qa =
                    new com.artifex.mupdf.fitz.Point(lineLeft, lineTop);
                com.artifex.mupdf.fitz.Point qb =
                    new com.artifex.mupdf.fitz.Point(lineRight, lineBottom);
                nearText = stext.copy(qa, qb);
            }
            stext.destroy();
            return nearText;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * 【笔记专用】生成当前页的 MuPDF 书签（Hex 字符串）
     */
    public synchronized String makeCurrentBookmark(int pageNum) {
        try {
            long mark = doc.makeBookmark(doc.locationFromPageNumber(pageNum));
            return Long.toHexString(mark);
        } catch (Exception e) {
            return "";
        }
    }

    /**
     * 【笔记专用】检查指定页面是否包含关键词
     */
    public synchronized boolean checkPageForKeyword(
        int pageNum,
        String keyword
    ) {
        gotoPage(pageNum);
        if (page == null) return false;
        try {
            Quad[][] hits = page.search(keyword);
            return hits != null && hits.length > 0;
        } catch (Exception e) {
            return false;
        }
    }

    public boolean isFixedLayout() {
        return !reflowable;
    }
}
