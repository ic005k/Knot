#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm.h>

#include <QRegularExpression>
#include <QString>

QString markdownToHtmlWithMath(const QString &md);

/////////////////////////////////////////////////////////////
QString markdownToHtmlWithMath(const QString &md) {
  // 初始化所有 GitHub 扩展
  cmark_gfm_core_extensions_ensure_registered();

  // 创建解析器并启用关键选项
  cmark_parser *parser = cmark_parser_new(
      CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES |  // 表格样式优化
      CMARK_OPT_UNSAFE                           // 保留原始字符（如 $）
  );

  // 附加所有需要的扩展
  const char *extensions[] = {"table", "strikethrough", "tasklist", "autolink",
                              "tagfilter"};
  for (const char *ext_name : extensions) {
    if (cmark_syntax_extension *ext = cmark_find_syntax_extension(ext_name)) {
      cmark_parser_attach_syntax_extension(parser, ext);
    }
  }

  // 解析 Markdown
  QByteArray utf8 = md.toUtf8();
  cmark_parser_feed(parser, utf8.constData(), utf8.size());
  cmark_node *doc = cmark_parser_finish(parser);
  cmark_parser_free(parser);

  // 渲染 HTML（保留原始内容）
  char *html_cstr = cmark_render_html(doc, CMARK_OPT_UNSAFE, nullptr);
  QString html = QString::fromUtf8(html_cstr);

  // --- 使用静态正则表达式对象避免警告 ---
  // 静态电话号码正则表达式
  static const QRegularExpression phoneRegex(
      R"((?<!["'<])\b(?:\+?86\s?)?(?:1[3-9]\d{9}|(?:0\d{2,3}-?)?[2-9]\d{7,8})(?:-\d{1,4})?\b(?!["'>]))",
      QRegularExpression::CaseInsensitiveOption);

  // 使用迭代器找到所有匹配项
  QRegularExpressionMatchIterator i = phoneRegex.globalMatch(html);
  QString result = html;
  int offset = 0;  // 跟踪替换导致的位置偏移

  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    if (match.hasMatch()) {
      // 获取匹配到的电话号码
      QString phoneNumber = match.captured(0);

      // 计算在原始字符串中的位置（考虑偏移）
      int start = match.capturedStart() + offset;
      int length = match.capturedLength();

      // 创建替换的链接字符串
      QString replacement =
          QString("<a href=\"tel:%1\">%1</a>").arg(phoneNumber);

      // 执行替换
      result.replace(start, length, replacement);

      // 更新偏移量（新字符串长度 - 原匹配长度）
      offset += replacement.length() - length;
    }
  }

  // 将处理后的结果赋值回html
  html = result;

  // --- 处理 Mermaid 代码块 - 使用静态正则表达式 ---
  static const QRegularExpression mermaidCodeBlock(
      R"(<pre><code class="language-mermaid">(.*?)</code></pre>)",
      QRegularExpression::DotMatchesEverythingOption);
  html.replace(mermaidCodeBlock, R"(<div class="mermaid">\1</div>)");

  // --- 定义 Mermaid 脚本 ---
  QString mermaid_js = R"(
      <script
  src="https://cdn.jsdelivr.net/npm/mermaid@9/dist/mermaid.min.js"></script>
      <script>
          document.addEventListener('DOMContentLoaded', function() {
              mermaid.initialize({ startOnLoad: true, theme: 'neutral' });
              mermaid.init();
          });
      </script>
  )";

  // 插入 MathJax 和语法高亮支持
  QString mathjax_config = R"(
        <script>
            MathJax = {
                tex: {
                    inlineMath: [['$', '$'], ['\\(', '\\)']],
                    displayMath: [['$$', '$$'], ['\\[', '\\]']],
                    processEscapes: true,
                    packages: {'[+]': ['base', 'ams', 'newcommand']}
                },
                options: {
                    ignoreHtmlClass: 'tex-ignore',
                    processHtmlClass: 'tex-process'
                },
                startup: {
                    typeset: false
                }
            };
        </script>
        <script src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>
        <script>MathJax.startup.document.state(0); MathJax.startup.defaultReady();</script>
    )";

  QString highlight_js = R"(
        <link rel="stylesheet"
    href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.7.0/styles/vs.min.css">
        <script
    src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.7.0/highlight.min.js"></script>
        <script>
            document.addEventListener('DOMContentLoaded', (event) => {
                document.querySelectorAll('pre code').forEach((el) => {
                    hljs.highlightElement(el);
                });
                if (typeof MathJax !== 'undefined') {
                    MathJax.typesetPromise();
                }
            });
        </script>
    )";

  // 添加 CSS 样式
  QString custom_css = R"(
        <style>
            table {
                border-collapse: collapse;
                margin: 1em 0;
                border: 1px solid #dee2e6;
            }
            th, td {
                padding: 0.75em;
                border: 1px solid #dee2e6;
            }
            th {
                background-color: #f8f9fa;
                font-weight: 600;
            }
            pre code {
                display: block;
                padding: 1em;
                background: #f8f9fa;
                border-radius: 4px;
                overflow-x: auto;
            }
            .tex-process {
                color: #d63384;
            }
            code {
                background: #f8f9fa;
                padding: 0.2em 0.4em;
                border-radius: 3px;
                font-family: monospace;
            }
            /* 电话号码链接样式 */
            a[href^="tel:"] {
                color: #0d6efd;
                text-decoration: underline;
            }
            sup {
                vertical-align: super;
                font-size: smaller;
            }
            sub {
                vertical-align: sub;
                font-size: smaller;
            }
            blockquote {
                border-left: 4px solid #dee2e6;
                padding-left: 1.5em;
                margin-left: 0;
                color: #6c757d;
                margin: 1em 0;
            }
        </style>
    )";

  // 组合完整 HTML
  html =
      "<!DOCTYPE html><html><head>"
      "<meta charset='utf-8'>" +
      mathjax_config + mermaid_js + highlight_js + custom_css +
      "</head><body>" + html + "</body></html>";

  // 清理资源
  free(html_cstr);
  cmark_node_free(doc);

  return html;
}

////////////////////////////////////////////////////////////////////
