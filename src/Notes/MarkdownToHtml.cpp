#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm.h>

#include <QDebug>
#include <QRegularExpression>
#include <QString>

// 预处理：给公式包裹唯一标签，避免被Markdown解析器干扰
static QString preprocessMath(const QString &md) {
  QString processed = md;

  // 行内公式：\( ... \) → <z_m_i>\( ... \)</z_m_i>
  processed.replace(
      QRegularExpression(R"(\\\(([\s\S]*?)\\\))",  // 匹配 \( ... \)
                         QRegularExpression::DotMatchesEverythingOption),
      "<z_m_i>\\(\\1\\)</z_m_i>"  // 保留原始公式分隔符，外层加保护标签
  );

  // 块级公式：\[ ... \] → <z_m_b>\[ ... \]</z_m_b>
  processed.replace(
      QRegularExpression(R"(\\\[([\s\S]*?)\\\])",  // 匹配 \[ ... \]
                         QRegularExpression::DotMatchesEverythingOption |
                             QRegularExpression::MultilineOption),
      "<z_m_b>\\[\\1\\]</z_m_b>");

  return processed;
}

// 后处理：移除保护标签，将公式分隔符转换为MathJax可识别的格式
static QString postprocessMath(const QString &html) {
  QString processed = html;

  // 1. 处理行内公式：
  // 匹配模式：&lt;z_m_i&gt;(...)<br/>&lt;/z_m_i&gt;（cmark转换后的格式）
  // 替换为：$...$（移除标签，保留公式内容）
  processed.replace(
      QRegularExpression(
          R"(&lt;z_m_i&gt;\(([\s\S]*?)\)&lt;/z_m_i&gt;)",  // 关键修正：匹配转义标签+()
          QRegularExpression::DotMatchesEverythingOption),
      "$\\1$");

  // 2. 处理块级公式：
  // 匹配模式：&lt;z_m_b&gt;[...]<br/>&lt;/z_m_b&gt;（cmark转换后的格式）
  // 替换为：$$...$$（移除标签，保留公式内容）
  processed.replace(
      QRegularExpression(
          R"(&lt;z_m_b&gt;\[([\s\S]*?)\]&lt;/z_m_b&gt;)",  // 关键修正：匹配转义标签+[]
          QRegularExpression::DotMatchesEverythingOption |
              QRegularExpression::MultilineOption  // 支持跨多行匹配
          ),
      "$$\\1$$");

  return processed;
}

/////////////////////////////////////////////////////////////
QString markdownToHtmlWithMath(const QString &md) {
  // 初始化GitHub扩展
  cmark_gfm_core_extensions_ensure_registered();

  // 创建解析器并启用选项
  cmark_parser *parser =
      cmark_parser_new(CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES |
                       CMARK_OPT_UNSAFE  // 保留原始字符，避免公式符号被转义
      );

  // 附加需要的扩展（不含tagfilter，避免干扰公式）
  const char *extensions[] = {"table", "strikethrough", "tasklist", "autolink"};
  for (const char *ext_name : extensions) {
    if (cmark_syntax_extension *ext = cmark_find_syntax_extension(ext_name)) {
      cmark_parser_attach_syntax_extension(parser, ext);
    }
  }

  // 预处理公式（包裹保护标签）
  QString processedMd = preprocessMath(md);
  QByteArray utf8 = processedMd.toUtf8();
  cmark_parser_feed(parser, utf8.constData(), utf8.size());
  cmark_node *doc = cmark_parser_finish(parser);
  cmark_parser_free(parser);

  // 渲染为HTML
  char *html_cstr = cmark_render_html(doc, CMARK_OPT_UNSAFE, nullptr);
  QString html = QString::fromUtf8(html_cstr);
  free(html_cstr);

  // 处理电话号码链接（保持原逻辑）
  static const QRegularExpression phoneRegex(
      R"((?<!["'<])\b(?:\+?86\s?)?(?:1[3-9]\d{9}|(?:0\d{2,3}-?)?[2-9]\d{7,8})(?:-\d{1,4})?\b(?!["'>]))",
      QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator i = phoneRegex.globalMatch(html);
  QString result = html;
  int offset = 0;
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    if (match.hasMatch()) {
      QString phoneNumber = match.captured(0);
      int start = match.capturedStart() + offset;
      int length = match.capturedLength();
      QString replacement =
          QString("<a href=\"tel:%1\">%1</a>").arg(phoneNumber);
      result.replace(start, length, replacement);
      offset += replacement.length() - length;
    }
  }
  html = result;

  // 处理Mermaid代码块（保持原逻辑）
  static const QRegularExpression mermaidCodeBlock(
      R"(<pre><code class="language-mermaid">(.*?)</code></pre>)",
      QRegularExpression::DotMatchesEverythingOption);
  html.replace(mermaidCodeBlock, R"(<div class="mermaid">\1</div>)");

  // 后处理公式（移除标签+转换分隔符）
  html = postprocessMath(html);

  // 拼接MathJax、高亮等资源（保持原逻辑）
  QString mermaid_js = R"(
        <script src="https://cdn.jsdelivr.net/npm/mermaid@9/dist/mermaid.min.js"></script>
        <script>
            document.addEventListener('DOMContentLoaded', function() {
                mermaid.initialize({ startOnLoad: true, theme: 'neutral' });
            });
        </script>
    )";

  QString mathjax_config = R"(
        <script>
            MathJax = {
                tex: {
                    inlineMath: [['$', '$'], ['\\(', '\\)']],
                    displayMath: [['$$', '$$'], ['\\[', '\\]']],
                    processEscapes: true
                },
                startup: { typeset: true }
            };
        </script>
        <script src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
    )";

  QString highlight_js = R"(
        <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.7.0/styles/vs.min.css">
        <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.7.0/highlight.min.js"></script>
        <script>
            document.addEventListener('DOMContentLoaded', () => {
                document.querySelectorAll('pre code').forEach(el => hljs.highlightElement(el));
            });
        </script>
    )";

  QString custom_css = R"(
        <style>
            table { border-collapse: collapse; margin: 1em 0; border: 1px solid #dee2e6; }
            th, td { padding: 0.75em; border: 1px solid #dee2e6; }
            th { background-color: #f8f9fa; font-weight: 600; }
            pre code { display: block; padding: 1em; background: #f8f9fa; border-radius: 4px; overflow-x: auto; }
            code { background: #f8f9fa; padding: 0.2em 0.4em; border-radius: 3px; }
            a[href^="tel:"] { color: #0d6efd; text-decoration: underline; }
            blockquote { border-left: 4px solid #dee2e6; padding-left: 1.5em; margin: 1em 0; color: #6c757d; }
        </style>
    )";

  // 组合完整HTML
  html = "<!DOCTYPE html><html><head><meta charset='utf-8'>" + mathjax_config +
         mermaid_js + highlight_js + custom_css + "</head><body>" + html +
         "</body></html>";

  // 清理资源
  cmark_node_free(doc);
  return html;
}
