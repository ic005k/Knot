#include <Qsci/qscilexermarkdown.h>
#include <Qsci/qsciscintilla.h>

#include <QFontDatabase>

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
  ui->frameEdit->layout()->addWidget(m_EditSource);
  m_EditSource->setFocus();
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

  // 当前行背景+边框
  if (isDark) {
    editor->setCaretLineBackgroundColor(QColor(180, 180, 0));
    editor->setCaretLineFrameWidth(1);
  } else {
    editor->setCaretLineBackgroundColor(QColor(255, 255, 0, 50));
    editor->setCaretLineFrameWidth(0);
  }

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