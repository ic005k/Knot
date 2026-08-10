#include <Qsci/qscilexermarkdown.h>
#include <Qsci/qsciscintilla.h>

#include <QFontDatabase>

#include "src/MainWindow.h"
#include "src/Notes/Notes.h"

void Notes::initEditor() {
#ifndef Q_OS_ANDROID
  m_EditSource = new QsciScintilla(this);
  m_EditSource->setUtf8(true);
  m_EditSource->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_EditSource->installEventFilter(this);
  m_EditSource->viewport()->installEventFilter(this);
  m_EditSource->setContentsMargins(1, 1, 1, 1);
  m_EditSource->setStyleSheet("border:none");

  connect(m_EditSource->verticalScrollBar(), &QScrollBar::valueChanged, this,
          &Notes::editVSBarValueChanged);
  connect(m_EditSource, &QsciScintilla::textChanged, this,
          &Notes::editSource_textChanged);

  ui->frameEdit->layout()->removeWidget(ui->f_NoteLink);
  ui->frameEdit->layout()->addWidget(m_EditSource);
  ui->frameEdit->layout()->addWidget(ui->f_NoteLink);
  m_EditSource->setFocus();

  // ⚠ 可以使用 textChanged + cursorPositionChanged 组合，
  // 或者使用 QScintilla 的 SCN_CLICK 通知（通过 sendScintilla）
  // 这里用最简单可靠的方式：监听光标位置变化
  connect(m_EditSource, &QsciScintilla::cursorPositionChanged, this,
          [this](int line, int index) {
            Q_UNUSED(index);
            QString lineText = m_EditSource->text(line);

            // ✅ 安全去除行尾换行，不影响 index 的有效性
            if (lineText.endsWith(QLatin1Char('\n'))) lineText.chop(1);
            if (lineText.endsWith(QLatin1Char('\r'))) lineText.chop(1);

            // ✅ 将 index 作为光标位置传入
            updateImagePreview(lineText, index);
          });

#endif
}

#ifndef Q_OS_ANDROID
void Notes::initMarkdownEditor(QsciScintilla* editor) {
  QFont defaultFont = QFont(this->font().family(), fontSize);
  markdownLexer->setFont(defaultFont, -1);
  markdownLexer->setFont(defaultFont, QsciLexerMarkdown::CodeBlock);

  editor->setFolding(QsciScintilla::BoxedTreeFoldStyle);

  editor->setAutoIndent(true);
  editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
  editor->setMarginLineNumbers(1, true);
  editor->setMarginType(0, QsciScintilla::NumberMargin);

  QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  if (monoFont.family().isEmpty()) {
    monoFont = QFont("Consolas", 10);
  }
  editor->SendScintilla(QsciScintilla::SCI_STYLESETFONT,
                        QsciScintilla::STYLE_LINENUMBER,
                        monoFont.family().toUtf8());

  QFontMetrics metrics(monoFont);
  int maxLineNumber = 10000;
  int textWidth = metrics.horizontalAdvance(QString(" %1 ").arg(maxLineNumber));
  editor->setMarginWidth(0, textWidth + 4);

  if (isDark) {
    editor->setMarginsBackgroundColor(QColor(0x1E, 0x1E, 0x1E));
    editor->setMarginsForegroundColor(QColor(0x7F, 0x7F, 0x7F));
  } else {
    editor->setMarginsBackgroundColor(QColor(240, 240, 240));
    editor->setMarginsForegroundColor(QColor(96, 96, 96));
  }

  editor->setMarginType(1, QsciScintilla::SymbolMargin);
  editor->setMarginWidth(1, 5);
  editor->setMarginType(2, QsciScintilla::SymbolMargin);
  editor->setMarginWidth(2, 5);
  editor->setMarginBackgroundColor(2, QColor(255, 228, 225));

  //--------------------------------------------------------------

  // 光标宽度
  editor->setCaretWidth(2);
  // 启用当前行高亮
  editor->setCaretLineVisible(true);

  // 光标文字颜色
  if (isDark)
    editor->setCaretForegroundColor(Qt::white);
  else
    editor->setCaretForegroundColor(Qt::black);

  // 当前行背景 + 边框（适配 Qsci 2.14.1）
  if (isDark) {
    // 深色模式：极低对比度的灰蓝/灰白，仅比编辑器背景亮 5%~8%
    editor->setCaretLineBackgroundColor(QColor(255, 255, 255, 12));
    // ⚠ 强烈建议去掉边框，或最多设为 1px 极淡边框
    editor->setCaretLineFrameWidth(0);
  } else {
    // 浅色模式：极低对比度的灰黑，仅比编辑器背景暗 3%~5%
    editor->setCaretLineBackgroundColor(QColor(0, 0, 0, 12));
    editor->setCaretLineFrameWidth(0);
  }

  // ⚠ 关键：确保当前行在文字下方渲染，避免影响字体抗锯齿
  // ⚠ 使用原始 Scintilla 消息 ID，绕过 QScintilla 封装缺失
  // SCI_SETCARETLINELAYER = 2768
  // SC_LAYER_UNDER_TEXT   = 1
  editor->SendScintilla(2768, 1);

  //-------------------------------------------------------

  editor->SendScintilla(QsciScintilla::SCI_SETPROPERTY, "fold", "1");
  editor->SendScintilla(QsciScintilla::SCI_SETFOLDFLAGS, 16);
  editor->setWrapVisualFlags(QsciScintilla::WrapFlagByText);
  editor->setWrapIndentMode(QsciScintilla::WrapIndentSame);
  editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  const int INDICATOR_SEARCH = 1;
  editor->SendScintilla(QsciScintilla::SCI_INDICSETSTYLE, INDICATOR_SEARCH,
                        QsciScintilla::INDIC_ROUNDBOX);
  editor->SendScintilla(QsciScintilla::SCI_INDICSETFORE, INDICATOR_SEARCH,
                        QColor(Qt::yellow).rgb());
  editor->SendScintilla(QsciScintilla::SCI_INDICSETALPHA, INDICATOR_SEARCH,
                        100);
  editor->SendScintilla(QsciScintilla::SCI_INDICSETUNDER, INDICATOR_SEARCH,
                        true);

  // 文本选中高亮样式（适配 Qsci 2.14.1）
  if (isDark) {
    // 深色模式：使用 VS Code / JetBrains 风格的柔和蓝
    editor->setSelectionBackgroundColor(QColor(38, 79, 120));
    // ⚠关键：不要设置 ForegroundColor，让 Scintilla 自动计算反色
    // editor->setSelectionForegroundColor(...)
  } else {
    // 浅色模式：使用低饱和度蓝，避免刺眼
    editor->setSelectionBackgroundColor(QColor(173, 214, 255));
    // 同样不设置前景色
  }

  // ⚠关键：移除或设为 256（禁用 Alpha 混合）
  editor->SendScintilla(QsciScintilla::SCI_SETSELALPHA, 256);
}
#endif

void Notes::init_md() {
#ifndef Q_OS_ANDROID

  if (isDark) {
    m_EditSource->verticalScrollBar()->setStyleSheet(
        mw_one->m_MainHelper->darkPCScrollbarStyle);
  } else {
    m_EditSource->verticalScrollBar()->setStyleSheet(
        mw_one->m_MainHelper->lightPCScrollbarStyle);
  }

  initMarkdownLexer();
  initMarkdownEditor(m_EditSource);
#endif
}

void Notes::editSource_textChanged() { isTextChange = true; }

// 搜索功能
void Notes::searchText(const QString& text, bool forward) {
#ifndef Q_OS_ANDROID
  m_lastSearchText = text;
  int line, index;
  m_EditSource->getCursorPosition(&line, &index);
  if (!forward) {
    if (index > 0) {
      index--;
    } else if (line > 0) {
      line--;
      index = m_EditSource->lineLength(line);
    }
    m_EditSource->setCursorPosition(line, index);
  }
  bool found =
      m_EditSource->findFirst(text, false, false, false, true, forward);
  if (found) {
  }
#endif
}

void Notes::searchNext() {
  if (!m_lastSearchText.isEmpty()) {
    searchText(m_lastSearchText, true);
  }
}

void Notes::searchPrevious() {
  if (!m_lastSearchText.isEmpty()) {
    searchText(m_lastSearchText, false);
  }
}

void Notes::jumpToNextMatch() {
#ifndef Q_OS_ANDROID
  if (m_matchPositions.isEmpty()) return;
  m_currentMatchIndex = (m_currentMatchIndex + 1) % m_matchPositions.size();
  auto pos = m_matchPositions[m_currentMatchIndex];
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, pos.first);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, pos.second);
  int line = m_EditSource->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION,
                                         pos.first);
  m_EditSource->ensureLineVisible(line);
#endif
}

void Notes::jumpToPrevMatch() {
#ifndef Q_OS_ANDROID
  if (m_matchPositions.isEmpty()) return;
  m_currentMatchIndex = (m_currentMatchIndex - 1 + m_matchPositions.size()) %
                        m_matchPositions.size();
  auto pos = m_matchPositions[m_currentMatchIndex];
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONSTART, pos.first);
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETSELECTIONEND, pos.second);
  int line = m_EditSource->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION,
                                         pos.first);
  m_EditSource->ensureLineVisible(line);
#endif
}

int Notes::getSearchMatchCount(const QString& text) {
  if (text.isEmpty()) return 0;
#ifndef Q_OS_ANDROID
  int originalPos =
      m_EditSource->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS);
  int originalAnchor =
      m_EditSource->SendScintilla(QsciScintilla::SCI_GETANCHOR);
  int count = 0;
  bool found = m_EditSource->findFirst(text, false, false, false, false, true);
  while (found) {
    count++;
    found = m_EditSource->findNext();
  }
  m_EditSource->SendScintilla(QsciScintilla::SCI_SETCURRENTPOS, originalPos);
  m_EditSource->SendScintilla(QsciScintilla::SCI_GETANCHOR, originalAnchor);
  return count;
#endif
  return 0;
}

void Notes::initMarkdownLexer() {
#ifndef Q_OS_ANDROID
  if (markdownLexer) {
    markdownLexer->deleteLater();
    markdownLexer = nullptr;
  }

  markdownLexer = new QsciLexerMarkdown(m_EditSource);
  m_EditSource->setLexer(markdownLexer);

  // 初始化默认亮色主题
  applyMdLexerTheme(isDark);
#endif
}

// 一键切换深色/亮色（外部切换主题直接调用这个）
void Notes::switchMdDarkTheme(bool dark) {
#ifndef Q_OS_ANDROID
  if (!markdownLexer || !m_EditSource) return;
  applyMdLexerTheme(dark);
  m_EditSource->recolor();
#endif
}

void Notes::applyMdLexerTheme(bool darkMode) {
#ifndef Q_OS_ANDROID
  using MdLex = QsciLexerMarkdown;
  QColor bgMain, fgNormal, caretColor, caretLineBg, selBg, marginBg, marginFg;
  // 修复：手动构造等宽字体，避开 defaultFont(style) 参数问题
  QFont monoFont("Consolas, Fira Code, Menlo", 11);

  if (darkMode) {
    bgMain = QColor(0x1E1E1E);
    fgNormal = QColor(0xE0E0E0);
    caretColor = Qt::white;
    caretLineBg = QColor(0x2D2D30);
    selBg = QColor(0x404048);
    marginBg = QColor(0x1E1E1E);
    marginFg = QColor(0x858585);

    markdownLexer->setDefaultPaper(bgMain);
    markdownLexer->setDefaultColor(fgNormal);

    // 0 普通正文
    markdownLexer->setColor(fgNormal, MdLex::Default);
    markdownLexer->setPaper(bgMain, MdLex::Default);

    // 各级标题
    markdownLexer->setColor(QColor(0x66CCFF), MdLex::Header1);
    markdownLexer->setPaper(bgMain, MdLex::Header1);
    markdownLexer->setColor(QColor(0x66FFFF), MdLex::Header2);
    markdownLexer->setPaper(bgMain, MdLex::Header2);
    markdownLexer->setColor(QColor(0xFF99FF), MdLex::Header3);
    markdownLexer->setPaper(bgMain, MdLex::Header3);
    markdownLexer->setColor(QColor(0xB0C4FF), MdLex::Header4);
    markdownLexer->setPaper(bgMain, MdLex::Header4);
    markdownLexer->setColor(QColor(0xB0C4FF), MdLex::Header5);
    markdownLexer->setPaper(bgMain, MdLex::Header5);
    markdownLexer->setColor(QColor(0xB0C4FF), MdLex::Header6);
    markdownLexer->setPaper(bgMain, MdLex::Header6);

    // 链接 + 下划线
    markdownLexer->setColor(QColor(0x66CCFF), MdLex::Link);
    markdownLexer->setPaper(bgMain, MdLex::Link);
    m_EditSource->SendScintilla(QsciScintilla::SCI_STYLESETUNDERLINE,
                                MdLex::Link, 1);

    // 行内代码 `xxx` / ``xxx``
    markdownLexer->setColor(QColor(0xA6E3A1), MdLex::CodeBackticks);
    markdownLexer->setPaper(QColor(0x2B2B2B), MdLex::CodeBackticks);
    markdownLexer->setColor(QColor(0xA6E3A1), MdLex::CodeDoubleBackticks);
    markdownLexer->setPaper(QColor(0x2B2B2B), MdLex::CodeDoubleBackticks);

    // 代码块 ``` ... ```
    markdownLexer->setColor(QColor(0xF0F0F0), MdLex::CodeBlock);
    markdownLexer->setPaper(QColor(0x2B2B2B), MdLex::CodeBlock);

    // 引用 > xxx
    markdownLexer->setColor(QColor(0xD7BA7D), MdLex::BlockQuote);
    markdownLexer->setPaper(bgMain, MdLex::BlockQuote);
    QFont fontQuote = monoFont;
    fontQuote.setItalic(true);
    markdownLexer->setFont(fontQuote, MdLex::BlockQuote);

    // 无序列表 - *
    markdownLexer->setColor(QColor(0xFFB86C), MdLex::UnorderedListItem);
    markdownLexer->setPaper(bgMain, MdLex::UnorderedListItem);
    // 有序列表 1.
    markdownLexer->setColor(QColor(0xFFB86C), MdLex::OrderedListItem);
    markdownLexer->setPaper(bgMain, MdLex::OrderedListItem);

    // 加粗 ** / __
    QFont fontBold = monoFont;
    fontBold.setBold(true);
    markdownLexer->setColor(Qt::white, MdLex::StrongEmphasisAsterisks);
    markdownLexer->setPaper(bgMain, MdLex::StrongEmphasisAsterisks);
    markdownLexer->setFont(fontBold, MdLex::StrongEmphasisAsterisks);

    markdownLexer->setColor(Qt::white, MdLex::StrongEmphasisUnderscores);
    markdownLexer->setPaper(bgMain, MdLex::StrongEmphasisUnderscores);
    markdownLexer->setFont(fontBold, MdLex::StrongEmphasisUnderscores);

    // 斜体 * / _
    QFont fontItalic = monoFont;
    fontItalic.setItalic(true);
    markdownLexer->setColor(QColor(0xC0C0C0), MdLex::EmphasisAsterisks);
    markdownLexer->setPaper(bgMain, MdLex::EmphasisAsterisks);
    markdownLexer->setFont(fontItalic, MdLex::EmphasisAsterisks);

    markdownLexer->setColor(QColor(0xC0C0C0), MdLex::EmphasisUnderscores);
    markdownLexer->setPaper(bgMain, MdLex::EmphasisUnderscores);
    markdownLexer->setFont(fontItalic, MdLex::EmphasisUnderscores);

    // 删除线 ~~ 修复：使用数字2031代替不存在的枚举
    markdownLexer->setColor(QColor(0x888888), MdLex::StrikeOut);
    markdownLexer->setPaper(bgMain, MdLex::StrikeOut);
    m_EditSource->SendScintilla(2031, MdLex::StrikeOut, 1);

    // 分割线 HR
    markdownLexer->setColor(QColor(0x707070), MdLex::HorizontalRule);
    markdownLexer->setPaper(bgMain, MdLex::HorizontalRule);
  } else {
    // 亮色主题
    bgMain = Qt::white;
    fgNormal = QColor(0x24292F);
    caretColor = Qt::black;
    caretLineBg = QColor(0xF0F4F8);
    selBg = QColor(0xB3D7FF);
    marginBg = Qt::white;
    marginFg = QColor(0x656D76);

    markdownLexer->setDefaultPaper(bgMain);
    markdownLexer->setDefaultColor(fgNormal);

    markdownLexer->setColor(fgNormal, MdLex::Default);
    markdownLexer->setPaper(bgMain, MdLex::Default);

    // 标题
    markdownLexer->setColor(QColor(0x0969DA), MdLex::Header1);
    markdownLexer->setPaper(bgMain, MdLex::Header1);
    markdownLexer->setColor(QColor(0x0969DA), MdLex::Header2);
    markdownLexer->setPaper(bgMain, MdLex::Header2);
    markdownLexer->setColor(QColor(0x8250DF), MdLex::Header3);
    markdownLexer->setPaper(bgMain, MdLex::Header3);
    markdownLexer->setColor(QColor(0x0969DA), MdLex::Header4);
    markdownLexer->setPaper(bgMain, MdLex::Header4);
    markdownLexer->setColor(QColor(0x0969DA), MdLex::Header5);
    markdownLexer->setPaper(bgMain, MdLex::Header5);
    markdownLexer->setColor(QColor(0x0969DA), MdLex::Header6);
    markdownLexer->setPaper(bgMain, MdLex::Header6);

    // 链接
    markdownLexer->setColor(QColor(0x0969DA), MdLex::Link);
    markdownLexer->setPaper(bgMain, MdLex::Link);
    m_EditSource->SendScintilla(QsciScintilla::SCI_STYLESETUNDERLINE,
                                MdLex::Link, 1);

    // 行内代码
    markdownLexer->setColor(QColor(0xCF222E), MdLex::CodeBackticks);
    markdownLexer->setPaper(QColor(0xF6F8FA), MdLex::CodeBackticks);
    markdownLexer->setColor(QColor(0xCF222E), MdLex::CodeDoubleBackticks);
    markdownLexer->setPaper(QColor(0xF6F8FA), MdLex::CodeDoubleBackticks);

    // 代码块
    markdownLexer->setColor(fgNormal, MdLex::CodeBlock);
    markdownLexer->setPaper(QColor(0xF6F8FA), MdLex::CodeBlock);

    // 引用
    markdownLexer->setColor(QColor(0x656D76), MdLex::BlockQuote);
    markdownLexer->setPaper(bgMain, MdLex::BlockQuote);
    QFont fontQuote = monoFont;
    fontQuote.setItalic(true);
    markdownLexer->setFont(fontQuote, MdLex::BlockQuote);

    // 列表标记
    markdownLexer->setColor(QColor(0x9A6700), MdLex::UnorderedListItem);
    markdownLexer->setPaper(bgMain, MdLex::UnorderedListItem);
    markdownLexer->setColor(QColor(0x9A6700), MdLex::OrderedListItem);
    markdownLexer->setPaper(bgMain, MdLex::OrderedListItem);

    // 加粗
    QFont fontBold = monoFont;
    fontBold.setBold(true);
    markdownLexer->setColor(fgNormal, MdLex::StrongEmphasisAsterisks);
    markdownLexer->setPaper(bgMain, MdLex::StrongEmphasisAsterisks);
    markdownLexer->setFont(fontBold, MdLex::StrongEmphasisAsterisks);
    markdownLexer->setColor(fgNormal, MdLex::StrongEmphasisUnderscores);
    markdownLexer->setPaper(bgMain, MdLex::StrongEmphasisUnderscores);
    markdownLexer->setFont(fontBold, MdLex::StrongEmphasisUnderscores);

    // 斜体
    QFont fontItalic = monoFont;
    fontItalic.setItalic(true);
    markdownLexer->setColor(QColor(0x57606A), MdLex::EmphasisAsterisks);
    markdownLexer->setPaper(bgMain, MdLex::EmphasisAsterisks);
    markdownLexer->setFont(fontItalic, MdLex::EmphasisAsterisks);
    markdownLexer->setColor(QColor(0x57606A), MdLex::EmphasisUnderscores);
    markdownLexer->setPaper(bgMain, MdLex::EmphasisUnderscores);
    markdownLexer->setFont(fontItalic, MdLex::EmphasisUnderscores);

    // 删除线 数字指令
    markdownLexer->setColor(QColor(0x8C959F), MdLex::StrikeOut);
    markdownLexer->setPaper(bgMain, MdLex::StrikeOut);
    m_EditSource->SendScintilla(2031, MdLex::StrikeOut, 1);

    // 分割线
    markdownLexer->setColor(QColor(0xD1D9E0), MdLex::HorizontalRule);
    markdownLexer->setPaper(bgMain, MdLex::HorizontalRule);
  }

  // 全局编辑器UI同步
  m_EditSource->setCaretForegroundColor(caretColor);
  m_EditSource->setCaretLineBackgroundColor(caretLineBg);
  m_EditSource->setMarginsBackgroundColor(marginBg);
  m_EditSource->setMarginsForegroundColor(marginFg);
  m_EditSource->setSelectionBackgroundColor(selBg);
  m_EditSource->setSelectionForegroundColor(fgNormal);
#endif
}

void Notes::updateImagePreview(const QString& lineText, int cursorPos) {
  QString result = parsePreviewData(lineText, cursorPos);

  // ✅ 统一重置样式（防止状态切换时残留）
  ui->lblNoteImage->clear();
  ui->lblNoteImage->setWordWrap(false);
  ui->lblNoteImage->setStyleSheet("");
  ui->lblNoteImage->setAlignment(Qt::AlignCenter);

  if (result.startsWith(QStringLiteral("IMG:"))) {
    // ===== 场景A：图片有效 =====
    QString path = result.mid(4);
    QPixmap pixmap(path);
    if (!pixmap.isNull()) {
      QPixmap scaled = pixmap.scaled(100, 100, Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
      ui->lblNoteImage->setPixmap(scaled);
    } else {
      // 极端情况：parsePreviewData 认为存在但实际加载失败
      ui->lblNoteImage->setText(QStringLiteral("🖼️"));
      ui->lblNoteImage->setStyleSheet("font-size: 28px; color: #999;");
    }

  } else if (result.startsWith(QStringLiteral("IMG_ERR:"))) {
    // ===== 场景1：图片链接存在但文件失效 =====
    ui->lblNoteImage->setText(QStringLiteral("🖼️"));
    ui->lblNoteImage->setStyleSheet(
        "font-size: 28px; color: #c66;");  // 偏红暗示异常

  } else if (result.startsWith(QStringLiteral("TXT:"))) {
    // ===== 场景B：笔记有效 =====
    QString text = result.mid(4);
    ui->lblNoteImage->setText(text);
    ui->lblNoteImage->setWordWrap(true);
    ui->lblNoteImage->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->lblNoteImage->setStyleSheet(
        "font-size: 11px; color: #666; padding: 4px;");

  } else if (result.startsWith(QStringLiteral("NOTE_ERR:"))) {
    // ===== 场景2：笔记链接存在但文件失效 =====
    ui->lblNoteImage->setText(QStringLiteral("📄"));
    ui->lblNoteImage->setStyleSheet(
        "font-size: 28px; color: #c66;");  // 与图片失效同色系

  } else {
    // ===== 场景3：当前行无任何链接 → 仅清除内容，组件保持可见 =====
    // clear() 已在开头调用，此处无需额外操作
    // 组件以空白状态存在，维持布局稳定
  }
}

QString Notes::generateSmartSummary(const QString& filePath) {
  QString cacheKey =
      filePath + "|" +
      QString::number(QFileInfo(filePath).lastModified().toSecsSinceEpoch());
  if (auto* cached = m_summaryCache.object(cacheKey)) return *cached;

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return QStringLiteral("❌ 无法读取");

  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);

  QString content = stream.readAll();
  file.close();

  // 1. 去除 Markdown 标题标记、空行、HTML标签
  static const QRegularExpression cleanRegex(
      R"(^#+\s*|<[^>]*>|\[([^\]]*)\]\([^)]*\)|!\[[^\]]*\]\([^)]*\))",
      QRegularExpression::MultilineOption);
  content.remove(cleanRegex);

  // 2. 合并多余空白，保留纯文本
  QStringList lines = content.split('\n', Qt::SkipEmptyParts);
  QString plainText;
  for (const auto& line : lines) {
    QString trimmed = line.trimmed();
    if (!trimmed.isEmpty()) plainText += trimmed + " ";
  }
  plainText = plainText.trimmed();

  if (plainText.isEmpty()) return QStringLiteral("📄 空文档");

  // 3. ✅ 国际化智能截断（约50个字符宽度）
  const int maxLen = 50;
  if (plainText.length() <= maxLen) return plainText;

  QString summary;
  int charCount = 0;
  bool lastWasCjk = false;

  for (int i = 0; i < plainText.length() && charCount < maxLen; ++i) {
    QChar ch = plainText[i];

    if (isCjkChar(ch)) {
      summary += ch;
      charCount++;
      lastWasCjk = true;
    } else if (isSpaceChar(ch)) {
      // 非CJK区域遇到空格才追加
      if (!lastWasCjk) {
        summary += ch;
      }
      lastWasCjk = false;
    } else {
      // 拉丁/数字等字符
      summary += ch;
      // 非CJK字符每2个算1个视觉宽度（粗略估算）
      if (!lastWasCjk) charCount++;
      lastWasCjk = false;
    }
  }

  // 4. 英文避免截断在单词中间：回退到最后一个空格
  if (!lastWasCjk && summary.contains(' ')) {
    int lastSpace = summary.lastIndexOf(' ');
    if (lastSpace > maxLen / 2) {
      summary = summary.left(lastSpace);
    }
  }

  // 在插入缓存之前拼接省略号
  QString result = (plainText.length() <= maxLen) ? plainText : summary + "…";

  m_summaryCache.insert(cacheKey, new QString(result));
  return result;  // 统一返回带省略号的完整结果
}

QString Notes::parsePreviewData(const QString& lineText, int cursorPos) {
  // 用于存储候选结果的结构体
  struct PreviewCandidate {
    QString type;     // "IMG", "TXT", "IMG_ERR", "NOTE_ERR"
    QString data;     // 路径、摘要或错误信息
    qsizetype start;  // 匹配起始位置
    qsizetype end;    // 匹配结束位置
  };

  QList<PreviewCandidate> candidates;

  // ===== 1. 收集所有图片匹配项（包含失效状态）=====
  QRegularExpressionMatchIterator imgIt = m_imgRegex.globalMatch(lineText);
  while (imgIt.hasNext()) {
    QRegularExpressionMatch m = imgIt.next();
    QString relPath = iniDir + "memo/" + m.captured(1).trimmed();
    QFileInfo fi(relPath);

    if (fi.exists() && fi.isFile()) {
      candidates.append(
          {QStringLiteral("IMG"), relPath, m.capturedStart(), m.capturedEnd()});
    } else {
      // ✅ 新增：图片链接存在但文件失效
      candidates.append({QStringLiteral("IMG_ERR"), relPath, m.capturedStart(),
                         m.capturedEnd()});
    }
  }

  // ===== 2. 收集所有笔记链接匹配项（包含失效状态）=====
  QRegularExpressionMatchIterator linkIt = m_linkRegex.globalMatch(lineText);
  while (linkIt.hasNext()) {
    QRegularExpressionMatch m = linkIt.next();
    QString relPath = iniDir + m.captured(2).trimmed();
    QFileInfo fi(relPath);

    if (fi.exists() && fi.isFile()) {
      QString summary = generateSmartSummary(relPath);
      candidates.append(
          {QStringLiteral("TXT"), summary, m.capturedStart(), m.capturedEnd()});
    } else {
      // ✅ 新增：笔记链接存在但文件失效
      candidates.append({QStringLiteral("NOTE_ERR"), relPath, m.capturedStart(),
                         m.capturedEnd()});
    }
  }

  // ===== 3. 根据光标位置精确选择 =====
  if (cursorPos >= 0) {
    for (const auto& c : candidates) {
      if (cursorPos >= c.start && cursorPos <= c.end) {
        return c.type + ":" + c.data;
      }
    }
  }

  // ===== 4. 兜底：按文本出现顺序返回第一个 =====
  std::sort(candidates.begin(), candidates.end(),
            [](const PreviewCandidate& a, const PreviewCandidate& b) {
              return a.start < b.start;
            });

  if (!candidates.isEmpty()) {
    const auto& first = candidates.first();
    return first.type + ":" + first.data;
  }

  // ===== 无可预览内容（当前行无任何链接语法）=====
  return QString();
}