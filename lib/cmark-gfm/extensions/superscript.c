#define CMARK_GFM_EXTENSIONS  // 必须定义此宏

#include "cmark-gfm.h"
#include "cmark-gfm-extension_api.h"
#include "superscript.h"

// 匹配 ^ 符号并创建上标节点
static cmark_node *match_superscript(
    cmark_syntax_extension *self,
    cmark_parser *parser,
    cmark_node *parent,
    unsigned char c,
    cmark_inline_parser *inline_parser
) {
    if (c != '^') return NULL;

    int start = cmark_inline_parser_get_index(inline_parser);
    int matched = cmark_inline_parser_scan_at(inline_parser, start + 1, '^', 1);
    if (matched != 1) return NULL;

    int end = cmark_inline_parser_get_index(inline_parser);
    cmark_node *sup_node = cmark_node_new(CMARK_NODE_CUSTOM_INLINE);
    cmark_node_set_syntax_extension(sup_node, self);
    cmark_node_set_string_content(sup_node, cmark_inline_parser_get_content(inline_parser) + start + 1);
    cmark_inline_parser_set_index(inline_parser, end + 1);  // 跳过结束的 ^
    return sup_node;
}

// 渲染上标 HTML
static const char *html_superscript(
    cmark_syntax_extension *extension,
    cmark_node *node,
    int indent,
    cmark_escape_mode mode
) {
    (void)extension;
    (void)indent;
    (void)mode;
    const char *content = cmark_node_get_string_content(node);
    return cmark_strdup_printf("<sup>%s</sup>", content);
}

// 创建上标扩展
cmark_syntax_extension *create_superscript_extension(void) {
    cmark_syntax_extension *ext = cmark_syntax_extension_new("superscript");
    cmark_syntax_extension_set_match_inline_func(ext, match_superscript);
    cmark_syntax_extension_set_html_render_func(ext, html_superscript);
    return ext;
}