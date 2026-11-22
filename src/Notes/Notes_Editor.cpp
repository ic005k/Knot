
#include <QFontDatabase>

#include "MainWindow.h"
#include "Notes/Notes.h"
#include "defines.h"

#include "ILexer.h"   // 确保 Scintilla::ILexer5 可见
#include <SciLexer.h> // SCE_MD_* 全部定义在这里，Scintilla.h 不含它

typedef Scintilla::ILexer5 *(*CreateLexerFn)(const char *name);
extern "C" Scintilla::ILexer5 *CreateLexer(const char *name);
static CreateLexerFn createLexer = &CreateLexer;

static inline uptr_t sciColor(const QColor &c) {
  return static_cast<uptr_t>(c.red() | (c.green() << 8) | (c.blue() << 16));
}

void Notes::initEditor() {
#ifndef Q_OS_ANDROID
  m_EditSource = new ScintillaEdit(this);
  m_EditSource->send(SCI_SETCODEPAGE, SC_CP_UTF8, 0);
  m_EditSource->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_EditSource->installEventFilter(this);
  m_EditSource->viewport()->installEventFilter(this);
  m_EditSource->setContentsMargins(1, 1, 1, 1);
  m_EditSource->setStyleSheet("border:none");

  connect(m_EditSource->verticalScrollBar(), &QScrollBar::valueChanged, this,
          &Notes::editVSBarValueChanged);
  connect(m_EditSource, &ScintillaEdit::notifyChange, this,
          [this] { editSource_textChanged(); });

  ui->frameEdit->layout()->addWidget(m_EditSource);

  m_EditSource->send(SCI_GRABFOCUS, 0, 0);

  // 监听光标位置变化
  connect(m_EditSource, &ScintillaEdit::notifyChange, this,
          [this]() { // ← 移除 (uptr_t, sptr_t) 参数
            int pos = m_EditSource->send(SCI_GETCURRENTPOS, 0, 0);
            int line = m_EditSource->send(SCI_LINEFROMPOSITION, pos, 0);
            int lineStart = m_EditSource->send(SCI_POSITIONFROMLINE, line, 0);
            int index = m_EditSource->send(SCI_COUNTCHARACTERS, lineStart, pos);

            int lineLen = m_EditSource->send(SCI_LINELENGTH, line, 0);
            QString lineText;
            if (lineLen > 0) {
              QByteArray buf(lineLen + 1, '\0');
              m_EditSource->send(SCI_GETLINE, line,
                                 reinterpret_cast<sptr_t>(buf.data()));
              lineText = QString::fromUtf8(buf.constData(), lineLen);
            }

            while (lineText.endsWith(QLatin1Char('\n')) ||
                   lineText.endsWith(QLatin1Char('\r')))
              lineText.chop(1);

            updateImagePreview(lineText, index);
          });
#endif
}

#ifndef Q_OS_ANDROID
QString Notes::getFullText() const {
  int len = m_EditSource->send(SCI_GETLENGTH, 0, 0);
  if (len <= 0)
    return {};
  QByteArray buf(len + 1, '\0');
  m_EditSource->send(SCI_GETTEXT, len + 1,
                     reinterpret_cast<sptr_t>(buf.data()));
  return QString::fromUtf8(buf.constData(), len);
}

void Notes::getCursorPosition(int &line, int &index) {
  int pos = m_EditSource->send(SCI_GETCURRENTPOS, 0, 0);
  line = m_EditSource->send(SCI_LINEFROMPOSITION, pos, 0);
  int lineStart = m_EditSource->send(SCI_POSITIONFROMLINE, line, 0);
  index = m_EditSource->send(SCI_COUNTCHARACTERS, lineStart, pos);
}

void Notes::setCursorPosition(int line, int index) {
  int lineStart = m_EditSource->send(SCI_POSITIONFROMLINE, line, 0);
  if (lineStart < 0)
    return; // 无效行号
  int pos = m_EditSource->send(SCI_POSITIONRELATIVE, lineStart, index);
  m_EditSource->send(SCI_GOTOPOS, pos, 0);
}

bool Notes::findFirst(const QString &text, bool forward, bool wrap) {
  m_EditSource->send(SCI_SETSEARCHFLAGS, SCFIND_NONE, 0);
  QByteArray s = text.toUtf8();
  int docLen = m_EditSource->send(SCI_GETLENGTH, 0, 0);
  int cur = m_EditSource->send(SCI_GETCURRENTPOS, 0, 0);

  auto doSearch = [&](int start, int end) -> int {
    m_EditSource->send(SCI_SETTARGETSTART, start, 0);
    m_EditSource->send(SCI_SETTARGETEND, end, 0);
    return m_EditSource->send(SCI_SEARCHINTARGET, s.length(),
                              reinterpret_cast<sptr_t>(s.constData()));
  };

  int pos = forward ? doSearch(cur, docLen) : doSearch(cur, 0);
  if (pos == -1 && wrap)
    pos = forward ? doSearch(0, cur) : doSearch(docLen, cur);

  if (pos != -1) {
    int selStart = m_EditSource->send(SCI_GETTARGETSTART, 0, 0);
    int selEnd = m_EditSource->send(SCI_GETTARGETEND, 0, 0);
    m_EditSource->send(SCI_SETSEL, selStart, selEnd);
    return true;
  }
  return false;
}

void Notes::initMarkdownEditor(ScintillaEdit *editor) {
  //--------------------------------------------------------------
  // 1. 代码折叠 (Margin 2)
  //--------------------------------------------------------------
  const int FOLD_MARGIN = 2;
  editor->send(SCI_SETMARGINTYPEN, FOLD_MARGIN, SC_MARGIN_SYMBOL);
  editor->send(SCI_SETMARGINMASKN, FOLD_MARGIN, SC_MASK_FOLDERS);
  editor->send(SCI_SETMARGINWIDTHN, FOLD_MARGIN, 14);
  editor->send(SCI_SETMARGINSENSITIVEN, FOLD_MARGIN, 1);

  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND,
               SC_MARK_BOXPLUSCONNECTED);
  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID,
               SC_MARK_BOXMINUSCONNECTED);
  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
  editor->send(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);

  // 折叠图标颜色跟随主题（默认黑色在深色底上看不见）
  const QColor foldFg =
      isDark ? QColor(0xAA, 0xAA, 0xAA) : QColor(0x80, 0x80, 0x80);
  const QColor foldBg =
      isDark ? QColor(0x1E, 0x1E, 0x1E) : QColor(0xFF, 0xFF, 0xFF);
  for (int mk = SC_MARKNUM_FOLDEREND; mk <= SC_MARKNUM_FOLDEROPEN; ++mk) {
    editor->send(SCI_MARKERSETFORE, mk, sciColor(foldFg));
    editor->send(SCI_MARKERSETBACK, mk, sciColor(foldBg));
  }

  //--------------------------------------------------------------
  // 2. 行号边栏 (Margin 0)
  //--------------------------------------------------------------
  editor->send(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);

  QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  if (monoFont.family().isEmpty())
    monoFont = QFont("Consolas", 10);

  QByteArray fontName = monoFont.family().toUtf8();
  editor->send(SCI_STYLESETFONT, STYLE_LINENUMBER,
               reinterpret_cast<sptr_t>(fontName.constData()));
  editor->send(SCI_STYLESETSIZE, STYLE_LINENUMBER, monoFont.pointSize());

  QFontMetrics metrics(monoFont);
  int textWidth = metrics.horizontalAdvance(QStringLiteral(" 9999 "));
  editor->send(SCI_SETMARGINWIDTHN, 0, textWidth + 4);

  const QColor lineBg =
      isDark ? QColor(0x1E, 0x1E, 0x1E) : QColor(0xF0, 0xF0, 0xF0);
  const QColor lineFg =
      isDark ? QColor(0x7F, 0x7F, 0x7F) : QColor(0x60, 0x60, 0x60);
  editor->send(SCI_STYLESETBACK, STYLE_LINENUMBER, sciColor(lineBg));
  editor->send(SCI_STYLESETFORE, STYLE_LINENUMBER, sciColor(lineFg));

  //--------------------------------------------------------------
  // 3. 符号边栏 (Margin 1)
  //--------------------------------------------------------------
  editor->send(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
  editor->send(SCI_SETMARGINWIDTHN, 1, 5);
  // 注意：这里不要再设置 STYLE_LINENUMBER 的背景色

  //--------------------------------------------------------------
  // 4. 光标与当前行
  //--------------------------------------------------------------
  // SCI_SHOWCARET 不存在；光标可见性 = 控件焦点 + 样式，用 CARETSTYLE 兜底
  editor->send(SCI_SETCARETSTYLE, CARETSTYLE_LINE, 0);
  editor->send(SCI_SETCARETWIDTH, 2, 0);
  editor->send(SCI_SETCARETFORE,
               sciColor(isDark ? QColor(0xF0, 0xF0, 0xF0) : QColor(0, 0, 0)),
               0);
  editor->send(SCI_SETCARETLINEVISIBLE, 1, 0);
  editor->send(SCI_SETCARETLINEFRAME, 0, 0);
  editor->send(
      SCI_SETCARETLINEBACK,
      sciColor(isDark ? QColor(0x2A, 0x2A, 0x2A) : QColor(0xF5, 0xF5, 0xDC)),
      0);
  editor->send(SCI_SETCARETLINEBACKALPHA, 255, 0);
  editor->send(SCI_SETCARETLINELAYER, SC_LAYER_UNDER_TEXT, 0);

  //--------------------------------------------------------------
  // 5. 折叠属性与换行
  //--------------------------------------------------------------
  editor->send(SCI_SETPROPERTY, reinterpret_cast<uptr_t>("fold"),
               reinterpret_cast<sptr_t>("1"));
  editor->send(SCI_SETFOLDFLAGS, 16, 0);
  editor->send(SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAGLOC_DEFAULT, 0);
  editor->send(SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
  editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  //--------------------------------------------------------------
  // 6. 搜索指示器
  //--------------------------------------------------------------
  const int INDICATOR_SEARCH = 1;
  editor->send(SCI_INDICSETSTYLE, INDICATOR_SEARCH, INDIC_ROUNDBOX);
  editor->send(SCI_INDICSETFORE, INDICATOR_SEARCH,
               sciColor(QColor(Qt::yellow)));
  editor->send(SCI_INDICSETALPHA, INDICATOR_SEARCH, 100);
  editor->send(SCI_INDICSETUNDER, INDICATOR_SEARCH, 1);

  //--------------------------------------------------------------
  // 7. 选中文本
  //--------------------------------------------------------------
  editor->send(SCI_SETSELBACK, 1,
               sciColor(isDark ? QColor(38, 79, 120) : QColor(173, 214, 255)));
  editor->send(SCI_SETSELFORE, 1,
               sciColor(isDark ? QColor(0xFF, 0xFF, 0xFF) : QColor(0, 0, 0)));
  editor->send(SCI_SETSELALPHA, 255, 0); // 不是 256

  //--------------------------------------------------------------
  // 8. 主题（必须放最后）
  //--------------------------------------------------------------
  applyMdLexerTheme(isDark);
}

#endif

void Notes::init_md() {
#ifndef Q_OS_ANDROID

  if (isDark) {
    m_EditSource->verticalScrollBar()->setStyleSheet(darkPCScrollbarStyle);
  } else {
    m_EditSource->verticalScrollBar()->setStyleSheet(lightPCScrollbarStyle);
  }

  initMarkdownLexer();
  initMarkdownEditor(m_EditSource);
#endif
}

void Notes::editSource_textChanged() { isTextChange = true; }

// 搜索功能

void Notes::searchText(const QString &text, bool forward) {
#ifndef Q_OS_ANDROID
  m_lastSearchText = text;

  // 先执行一次"查找所有"，刷新列表
  // findAllAndShowResults(text);

  // 再做"下一个/上一个"的跳转定位
  int line, index;
  getCursorPosition(line, index);
  if (!forward) {
    if (index > 0) {
      index--;
    } else if (line > 0) {
      line--;
      index = m_EditSource->lineLength(line);
    }
    setCursorPosition(line, index);
  }

  bool found = findFirst(text, forward, true);
  if (found) {
    // 可选：在列表中高亮当前匹配项
    int curLine, curIndex;
    getCursorPosition(curLine, curIndex);
    highlightCurrentResult(curLine, curIndex);
  }
#endif
}

// ============================================================
// 查找所有匹配项并填充 QListWidget
// ============================================================
void Notes::findAllAndShowResults(const QString &text) {
#ifndef Q_OS_ANDROID
  ui->listSearchResults->clear();

  if (text.isEmpty()) {
    updateResultCount(0);
    return;
  }

  QString fullText = getFullText(); // 获取编辑器全部文本
  QStringList lines = fullText.split('\n');

  int totalCount = 0;

  for (int i = 0; i < lines.size(); ++i) {
    const QString &lineText = lines[i];
    int pos = 0;

    // 在同一行中查找所有出现的位置
    while ((pos = lineText.indexOf(text, pos, Qt::CaseInsensitive)) != -1) {
      TextMatch result;
      result.line = i;
      result.index = pos;
      result.length = text.length();
      result.lineText = lineText;

      // ---------- 构造带上下文的预览文本 ----------
      // 上下文：显示匹配关键词前后各若干字符
      const int contextChars = 15; // 前后各取15个字符作为上下文

      int previewStart = qMax(0, pos - contextChars);
      int previewEnd =
          qMin(lineText.length(), pos + text.length() + contextChars);
      QString prefix = lineText.mid(previewStart, pos - previewStart);
      QString match = lineText.mid(pos, text.length());
      QString suffix =
          lineText.mid(pos + text.length(), previewEnd - pos - text.length());

      // 行首省略号
      QString displayPrefix = (previewStart > 0) ? "…" : "";
      // 行尾省略号
      QString displaySuffix = (previewEnd < lineText.length()) ? "…" : "";

      QString preview =
          QString("%1%2%3%4").arg(displayPrefix, prefix, match, suffix) +
          displaySuffix;

      // ---------- 创建 QListWidgetItem ----------
      QListWidgetItem *item = new QListWidgetItem(ui->listSearchResults);

      // 行号标签 + 预览
      QString itemText = QString("[Row %1]  %2").arg(i + 1).arg(preview);
      item->setText(itemText);

      // 存储搜索结果数据
      QVariant data;
      data.setValue(result);
      item->setData(Qt::UserRole, data);

      // ---------- 使用 HTML 高亮关键词 ----------
      // 我们对 display 文本中匹配到的关键词部分上色
      buildHighlightedItem(item, i, displayPrefix, prefix, match, suffix,
                           displaySuffix);

      totalCount++;
      pos += text.length(); // 继续向后查找（支持重叠可改为 pos++）
    }
  }

  updateResultCount(totalCount);

  // 连接信号（避免重复连接）
  disconnect(ui->listSearchResults, &QListWidget::itemClicked, nullptr,
             nullptr);
  connect(ui->listSearchResults, &QListWidget::itemClicked, this,
          &Notes::onSearchResultClicked);

#endif
}

// ============================================================
// 构造高亮的 QListWidgetItem
// ============================================================
void Notes::buildHighlightedItem(QListWidgetItem *item, int lineNum,
                                 const QString &displayPrefix,
                                 const QString &prefix, const QString &match,
                                 const QString &suffix,
                                 const QString &displaySuffix) {
  auto escape = [](const QString &s) -> QString {
    QString e = s;
    e.replace("&", "&amp;");
    e.replace("<", "&lt;");
    e.replace(">", "&gt;");
    return e;
  };

  // 预构建 HTML 字符串，存入 UserRole+1
  // （UserRole 仍存 TextMatch 数据，UserRole+1 存显示用 HTML）
  QString html =
      QString("<span style='color:#888;'>[Row %1]</span>&nbsp;&nbsp;"
              "<span style='color:#aaa;'>%2%3</span>"
              "<span style='background-color:#FFEB3B; color:#000; "
              "font-weight:bold; padding:1px 2px; border-radius:2px;'>%4</span>"
              "<span style='color:#aaa;'>%5%6</span>")
          .arg(QString::number(lineNum + 1), escape(displayPrefix),
               escape(prefix), escape(match), escape(suffix),
               escape(displaySuffix));

  item->setData(Qt::UserRole + 1, html);

  // 保留纯文本用于无障碍 / tooltip
  TextMatch tm = item->data(Qt::UserRole).value<TextMatch>();
  item->setToolTip(
      QString("Row %1, Col %2").arg(tm.line + 1).arg(tm.index + 1));
}

// ============================================================
// 更新搜索结果数量标签
// ============================================================
void Notes::updateResultCount(int count) {
  ui->tabAI->setCurrentIndex(2);
  if (count == 0) {
    ui->lblSearchResults->setText(tr("No matches found"));
    if (isDark) {
      ui->lblSearchResults->setStyleSheet("color:#888888;");
    } else {
      ui->lblSearchResults->setStyleSheet("color:#999999;");
    }
  } else {
    ui->lblSearchResults->setText(tr("Matches found:") +
                                  QString::number(count));
    if (isDark) {
      ui->lblSearchResults->setStyleSheet("color:#E2E2E2;");
    } else {
      ui->lblSearchResults->setStyleSheet("color:#333333;");
    }
  }
}

// ============================================================
// 点击搜索结果 → 跳转到编辑器对应位置并高亮
// ============================================================
void Notes::onSearchResultClicked(QListWidgetItem *item) {
#ifndef Q_OS_ANDROID

  TextMatch result = item->data(Qt::UserRole).value<TextMatch>();

  // 1. 确保目标行可见 (替代 ensureLineVisible)
  m_EditSource->send(SCI_ENSUREVISIBLEENFORCEPOLICY, result.line, 0);

  // 2. 将 (line, index) 转换为字节位置
  // ⚠️ 使用 SCI_POSITIONRELPATH 或手动计算：先获取行首字节位置，再按字符数偏移
  int anchorPos = m_EditSource->send(SCI_POSITIONFROMLINE, result.line, 0);
  anchorPos = m_EditSource->send(SCI_POSITIONRELATIVE, anchorPos, result.index);

  int caretPos = m_EditSource->send(SCI_POSITIONFROMLINE, result.line, 0);
  caretPos = m_EditSource->send(SCI_POSITIONRELATIVE, caretPos,
                                result.index + result.length);

  // 3. 选中文本 (替代 setSelection(line, index, line, index))
  // ⚠️ 注意参数顺序：SCI_SETSEL(anchor, caret)，anchor 是选区起点，caret
  // 是光标终点
  m_EditSource->send(SCI_SETSEL, anchorPos, caretPos);

  // 4. 设置光标到匹配末尾 (已由 SCI_SETSEL 完成，无需额外调用)
  // 若仅需移动光标而不改变选区，才需要单独调用 SCI_GOTOPOS(caretPos, 0)

  // 5. 让编辑器获得焦点 (Qt 层 API，保留不变)
  static_cast<QWidget *>(m_EditSource)->setFocus();

#endif
}

// ============================================================
// 在列表中高亮"当前"所在的结果条目（可选）
// ============================================================
void Notes::highlightCurrentResult(int line, int index) {
  for (int i = 0; i < ui->listSearchResults->count(); ++i) {
    QListWidgetItem *item = ui->listSearchResults->item(i);
    TextMatch r = item->data(Qt::UserRole).value<TextMatch>();

    if (r.line == line && r.index == index) {
      ui->listSearchResults->setCurrentItem(item);
      ui->listSearchResults->scrollToItem(item);
      break;
    }
  }
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
  if (m_matchPositions.isEmpty())
    return;
  m_currentMatchIndex = (m_currentMatchIndex + 1) % m_matchPositions.size();
  auto pos = m_matchPositions[m_currentMatchIndex];

  // 设置选区（anchor=start, caret=end）
  m_EditSource->send(SCI_SETSEL, pos.first, pos.second);

  // 确保匹配所在行可见并遵循滚动策略
  int line = m_EditSource->send(SCI_LINEFROMPOSITION, pos.first, 0);
  m_EditSource->send(SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
#endif
}

void Notes::jumpToPrevMatch() {
#ifndef Q_OS_ANDROID
  if (m_matchPositions.isEmpty())
    return;
  m_currentMatchIndex = (m_currentMatchIndex - 1 + m_matchPositions.size()) %
                        m_matchPositions.size();
  auto pos = m_matchPositions[m_currentMatchIndex];

  // 设置选区（anchor=start, caret=end）
  m_EditSource->send(SCI_SETSEL, pos.first, pos.second);

  // 确保匹配所在行可见并遵循滚动策略
  int line = m_EditSource->send(SCI_LINEFROMPOSITION, pos.first, 0);
  m_EditSource->send(SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
#endif
}

void Notes::initMarkdownLexer() {
#ifndef Q_OS_ANDROID

  // 创建 Markdown Lexer 实例
  Scintilla::ILexer5 *pLexer = createLexer("markdown");

  // 应用到编辑器
  m_EditSource->send(SCI_SETILEXER, 0, reinterpret_cast<sptr_t>(pLexer));

  // 初始化默认亮色主题
  applyMdLexerTheme(isDark);
#endif
}

// 一键切换深色/亮色（外部切换主题直接调用这个）
void Notes::switchMdDarkTheme(bool dark) {
#ifndef Q_OS_ANDROID

  applyMdLexerTheme(dark);

#endif
}

void Notes::updateImagePreview(const QString &lineText, int cursorPos) {
  QString result = parsePreviewData(lineText, cursorPos);

  // ✅ 统一重置样式（防止状态切换时残留）
  ui->lblNoteImage->clear();
  ui->lblNoteImage->setText(tr("Link Preview"));
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
        "font-size: 28px; color: #c66;"); // 偏红暗示异常

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
        "font-size: 28px; color: #c66;"); // 与图片失效同色系

  } else {
    // ===== 场景3：当前行无任何链接 → 仅清除内容，组件保持可见 =====
    // clear() 已在开头调用，此处无需额外操作
    // 组件以空白状态存在，维持布局稳定
  }
}

QString Notes::generateSmartSummary(const QString &filePath) {
  QString cacheKey =
      filePath + "|" +
      QString::number(QFileInfo(filePath).lastModified().toSecsSinceEpoch());
  if (auto *cached = m_summaryCache.object(cacheKey))
    return *cached;

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
  for (const auto &line : std::as_const(lines)) { // 使用 std::as_const 包裹
    QString trimmed = line.trimmed();
    if (!trimmed.isEmpty())
      plainText += trimmed + " ";
  }
  plainText = plainText.trimmed();

  if (plainText.isEmpty())
    return QStringLiteral("📄 空文档");

  // 3. ✅ 国际化智能截断（约50个字符宽度）
  const int maxLen = 50;
  if (plainText.length() <= maxLen)
    return plainText;

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
      if (!lastWasCjk)
        charCount++;
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
  return result; // 统一返回带省略号的完整结果
}

QString Notes::parsePreviewData(const QString &lineText, int cursorPos) {
  // 用于存储候选结果的结构体
  struct PreviewCandidate {
    QString type;    // "IMG", "TXT", "IMG_ERR", "NOTE_ERR"
    QString data;    // 路径、摘要或错误信息
    qsizetype start; // 匹配起始位置
    qsizetype end;   // 匹配结束位置
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
    for (const auto &c : candidates) {
      if (cursorPos >= c.start && cursorPos <= c.end) {
        return c.type + ":" + c.data;
      }
    }
  }

  // ===== 4. 兜底：按文本出现顺序返回第一个 =====
  std::sort(candidates.begin(), candidates.end(),
            [](const PreviewCandidate &a, const PreviewCandidate &b) {
              return a.start < b.start;
            });

  if (!candidates.isEmpty()) {
    const auto &first = candidates.first();
    return first.type + ":" + first.data;
  }

  // ===== 无可预览内容（当前行无任何链接语法）=====
  return QString();
}

// 启动后台搜索（QPointer 守卫）
void Notes::startBackgroundSearch(const QString &keyword) {
#ifndef Q_OS_ANDROID
  if (keyword.isEmpty()) {
    ui->listSearchResults->clear();
    updateResultCount(0);
    return;
  }

  const QString fullText = getFullText();
  const int currentGeneration = ++m_searchGeneration;
  QPointer<Notes> guard(this);

  (void)QtConcurrent::run([guard, fullText, keyword, currentGeneration]() {
    // ========== 后台线程 ==========
    QStringList lines = fullText.split('\n');
    QList<TextMatch> matches;
    QStringList htmlList;

    auto escape = [](const QString &s) -> QString {
      QString e = s;
      e.replace("&", "&amp;");
      e.replace("<", "&lt;");
      e.replace(">", "&gt;");
      return e;
    };

    for (int i = 0; i < lines.size(); ++i) {
      const QString &lineText = lines[i];
      int pos = 0;

      while ((pos = lineText.indexOf(keyword, pos, Qt::CaseInsensitive)) !=
             -1) {
        TextMatch result;
        result.line = i;
        result.index = pos;
        result.length = keyword.length();
        result.lineText = lineText;

        const int contextChars = 15;
        int previewStart = qMax(0, pos - contextChars);
        int previewEnd =
            qMin(lineText.length(), pos + keyword.length() + contextChars);

        QString displayPrefix =
            (previewStart > 0) ? QStringLiteral("…") : QString();
        QString prefix = lineText.mid(previewStart, pos - previewStart);
        QString match = lineText.mid(pos, keyword.length());
        QString suffix = lineText.mid(pos + keyword.length(),
                                      previewEnd - pos - keyword.length());
        QString displaySuffix =
            (previewEnd < lineText.length()) ? QStringLiteral("…") : QString();

        // 后台线程中，HTML 只包含结构和类名，永远不变
        QString html =
            QStringLiteral("<span class='ln'>[Row %1]</span>&nbsp;&nbsp;"
                           "<span class='ctx'>%2%3</span>"
                           "<span class='hl'>%4</span>"
                           "<span class='ctx'>%5%6</span>")
                .arg(QString::number(i + 1), escape(displayPrefix),
                     escape(prefix), escape(match), escape(suffix),
                     escape(displaySuffix));

        matches.append(result);
        htmlList.append(html);
        pos += keyword.length();
      }
    }

    // ========== 回到主线程 ==========
    if (!guard)
      return;
    QMetaObject::invokeMethod(
        guard.data(),
        [guard, matches, htmlList, currentGeneration]() {
          if (!guard)
            return;
          Notes *self = guard.data();
          if (currentGeneration != self->m_searchGeneration)
            return;
          self->applySearchResults(matches, htmlList);
        },
        Qt::QueuedConnection);
  });
#endif
}

void Notes::applySearchResults(const QList<TextMatch> &matches,
                               const QStringList &htmlList) {
#ifndef Q_OS_ANDROID
  ui->listSearchResults->setUpdatesEnabled(false);
  ui->listSearchResults->clear();

  for (int i = 0; i < matches.size(); ++i) {
    const TextMatch &match = matches[i];

    QListWidgetItem *item = new QListWidgetItem(ui->listSearchResults);
    item->setText(
        QStringLiteral("[Row %1]  %2").arg(match.line + 1).arg(match.lineText));

    QVariant data;
    data.setValue(match);
    item->setData(Qt::UserRole, data);
    item->setData(Qt::UserRole + 1, htmlList[i]);
    item->setToolTip(QStringLiteral("Row %1, Col %2")
                         .arg(match.line + 1)
                         .arg(match.index + 1));
  }

  ui->listSearchResults->setUpdatesEnabled(true);
  updateResultCount(matches.size());

  disconnect(ui->listSearchResults, &QListWidget::itemClicked, this,
             &Notes::onSearchResultClicked);
  connect(ui->listSearchResults, &QListWidget::itemClicked, this,
          &Notes::onSearchResultClicked);
#endif
}

void Notes::applyMdLexerTheme(bool darkMode) {
#ifndef Q_OS_ANDROID
  ScintillaEdit *editor = m_EditSource;
  if (!editor)
    return;

  if (!darkMode) {
    editor->send(SCI_STYLESETBACK, STYLE_DEFAULT,
                 sciColor(QColor(0xFF, 0xFF, 0xFF)));
    editor->send(SCI_STYLESETFORE, STYLE_DEFAULT,
                 sciColor(QColor(0x00, 0x00, 0x00)));
    editor->send(SCI_COLOURISE, 0, -1);
    return;
  }

  // ---- 深色模式 ----
  const QColor bg(0x1E, 0x1E, 0x1E);
  const QColor fg(0xD4, 0xD4, 0xD4);

  // 默认样式 & 基础文字
  editor->send(SCI_STYLESETBACK, STYLE_DEFAULT, sciColor(bg));
  editor->send(SCI_STYLESETFORE, STYLE_DEFAULT, sciColor(fg));
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_DEFAULT, sciColor(fg));

  // 标题 H1-H6 (值 6~11，连续)
  for (int s = SCE_MARKDOWN_HEADER1; s <= SCE_MARKDOWN_HEADER6; ++s)
    editor->send(SCI_STYLESETFORE, s, sciColor(QColor(0x56, 0x9C, 0xD6)));

  // 水平分割线
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_HRULE,
               sciColor(QColor(0x80, 0x80, 0x80)));

  // 链接
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_LINK,
               sciColor(QColor(0x4E, 0xC9, 0xB0)));

  // 行内代码 & 代码块
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_CODE,
               sciColor(QColor(0xCE, 0x91, 0x69)));
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_CODE2,
               sciColor(QColor(0xCE, 0x91, 0x69)));
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_CODEBK,
               sciColor(QColor(0x6A, 0x99, 0x55)));
  editor->send(SCI_STYLESETBACK, SCE_MARKDOWN_CODEBK,
               sciColor(QColor(0x2D, 0x2D, 0x2D)));

  // 斜体 (*em* / _em_)
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_EM1,
               sciColor(QColor(0xC5, 0x86, 0xC0)));
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_EM2,
               sciColor(QColor(0xC5, 0x86, 0xC0)));

  // 粗体 (**strong** / __strong__)
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_STRONG1,
               sciColor(QColor(0xC5, 0x86, 0xC0)));
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_STRONG2,
               sciColor(QColor(0xC5, 0x86, 0xC0)));

  // 引用块
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_BLOCKQUOTE,
               sciColor(QColor(0x9C, 0x9C, 0x9C)));
  editor->send(SCI_STYLESETBACK, SCE_MARKDOWN_BLOCKQUOTE,
               sciColor(QColor(0x25, 0x25, 0x25)));

  // 无序列表 & 有序列表
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_ULIST_ITEM,
               sciColor(QColor(0xDC, 0xD2, 0xA0)));
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_OLIST_ITEM,
               sciColor(QColor(0xDC, 0xD2, 0xA0)));

  // 删除线
  editor->send(SCI_STYLESETFORE, SCE_MARKDOWN_STRIKEOUT,
               sciColor(QColor(0x80, 0x80, 0x80)));

  editor->send(SCI_COLOURISE, 0, -1);
#endif
}