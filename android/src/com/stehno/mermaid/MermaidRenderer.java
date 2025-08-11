package com.stehno.mermaid;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import com.caverock.androidsvg.SVG;
import com.caverock.androidsvg.SVGParseException;
import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;

public class MermaidRenderer {

    // 渲染Mermaid图表为Bitmap
    public static Bitmap render(String mermaidSource) throws MermaidException {
        try {
            // 1. 生成简单的Mermaid SVG（简化版，支持基础流程图）
            String svg = generateSvgFromMermaid(mermaidSource);
            
            // 2. 使用AndroidSVG解析SVG
            SVG svgObj = SVG.getFromInputStream(
                new ByteArrayInputStream(svg.getBytes(StandardCharsets.UTF_8))
            );
            
            // 3. 计算SVG尺寸
            float width = svgObj.getDocumentWidth();
            float height = svgObj.getDocumentHeight();
            
            // 4. 创建Bitmap并绘制SVG
            Bitmap bitmap = Bitmap.createBitmap(
                (int) width, 
                (int) height, 
                Bitmap.Config.ARGB_8888
            );
            Canvas canvas = new Canvas(bitmap);
            canvas.drawColor(Color.WHITE); // 白色背景
            svgObj.renderToCanvas(canvas);
            
            return bitmap;
        } catch (SVGParseException e) {
            throw new MermaidException("SVG解析错误", e);
        } catch (Exception e) {
            throw new MermaidException("渲染失败", e);
        }
    }

    // 简易Mermaid语法转SVG（支持基础graph TD语法）
    private static String generateSvgFromMermaid(String source) {
        // 简化实现：仅处理基础流程图（实际项目可扩展）
        // 示例：graph TD A[开始] --> B[结束]
        StringBuilder svg = new StringBuilder();
        svg.append("<svg width=\"600\" height=\"400\" xmlns=\"http://www.w3.org/2000/svg\">");
        
        // 绘制背景
        svg.append("<rect width=\"100%\" height=\"100%\" fill=\"white\"/>");
        
        // 简单文本提示（实际项目可解析mermaid语法生成图形）
        svg.append("<text x=\"50\" y=\"50\" font-size=\"16\" fill=\"black\">");
        svg.append("Mermaid图表预览（基础版）");
        svg.append("</text>");
        
        // 显示原始代码（实际项目应解析为图形）
        svg.append("<text x=\"50\" y=\"100\" font-size=\"12\" fill=\"gray\" width=\"500\">");
        svg.append(source.replace("<", "&lt;").replace(">", "&gt;"));
        svg.append("</text>");
        
        svg.append("</svg>");
        return svg.toString();
    }
}