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

void Notes::initMarkdownLexer() {
#ifndef Q_OS_ANDROID
  markdownLexer = new QsciLexerMarkdown(m_EditSource);
  m_EditSource->setLexer(markdownLexer);
  m_EditSource->SendScintilla(QsciScintilla::SCI_STYLERESETDEFAULT);
  m_EditSource->setCaretForegroundColor(QColor(0, 0, 0));
  m_EditSource->recolor();
#endif
}

void Notes::initMarkdownLexerDark() {
#ifndef Q_OS_ANDROID
  m_EditSource->setLexer(nullptr);
  markdownLexer = new QsciLexerMarkdown(m_EditSource);

  markdownLexer->setDefaultPaper(QColor(0x1E1E1E));
  markdownLexer->setDefaultColor(QColor(0xE0E0E0));

  markdownLexer->setColor(QColor(0x66CCFF), QsciLexerMarkdown::Header1);
  markdownLexer->setColor(QColor(0x66FFFF), QsciLexerMarkdown::Header2);
  markdownLexer->setColor(QColor(0xFF99FF), QsciLexerMarkdown::Header3);
  markdownLexer->setColor(QColor(0x66CCFF), QsciLexerMarkdown::Link);
  markdownLexer->setColor(QColor(0xF0F0F0), QsciLexerMarkdown::CodeBlock);
  markdownLexer->setPaper(QColor(0x2B2B2B), QsciLexerMarkdown::CodeBlock);
  markdownLexer->setColor(QColor(0xD7BA7D), QsciLexerMarkdown::BlockQuote);

  m_EditSource->setLexer(markdownLexer);
  m_EditSource->setCaretLineBackgroundColor(QColor(0x2D2D30));
  m_EditSource->setCaretForegroundColor(QColor(0xFFFFFF));
  m_EditSource->setMarginsBackgroundColor(QColor(0x1E1E1E));
  m_EditSource->setMarginsForegroundColor(QColor(0x858585));
  m_EditSource->setWrapMode(QsciScintilla::WrapWord);
  m_EditSource->recolor();
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
  editor->setMarginBackgroundColor(2, QColor("#FFE4E1"));

  editor->setCaretWidth(2);
  editor->setCaretLineVisible(true);
  if (isDark) {
    editor->setCaretLineBackgroundColor(QColor(45, 45, 48, 60));
  } else {
    editor->setCaretLineBackgroundColor(QColor(255, 248, 220, 255));
  }

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
    initMarkdownLexerDark();
    m_EditSource->verticalScrollBar()->setStyleSheet(
        mw_one->m_MainHelper->darkPCScrollbarStyle);
  } else {
    initMarkdownLexer();
    m_EditSource->verticalScrollBar()->setStyleSheet(
        mw_one->m_MainHelper->lightPCScrollbarStyle);
  }
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