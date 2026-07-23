#include "Preferences.h"

#include <QKeyEvent>

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_Preferences.h"

QFont::Weight readerFontWeight;
extern void loadTheme(bool isDark);
extern void releaseGlobalAiEngine();
extern int init_main_ai();
extern QString vecDbPath;

Preferences::Preferences(QWidget* parent)
    : QDialog(parent), ui(new Ui::Preferences) {
  ui->setupUi(this);

  m_Method->set_ToolButtonStyle(this);

  this->installEventFilter(this);
  ui->lblFontSize->installEventFilter(this);

  ui->gboxAdditional->hide();
  ui->lblAdditional->hide();
  ui->cboxEndpoint->setVisible(false);
  ui->cboxModel->setVisible(false);

  ui->lblModelTip->setText("");
  ui->lblModel->setText(tr("Local Model List:") +
                        "\n"
                        R"((~\.Knot\model))");

  ui->lblFontSize->setText(tr("Font Size") + " : " + QString::number(fontSize));
  isFontChange = false;

  chkStyle = ui->chkDark->styleSheet();
  mui->chkZip->setStyleSheet(chkStyle);
  ui->chkUIFont->setStyleSheet(chkStyle);
  ui->lblFontSize->setFixedHeight(40);

  QString lbl_style = ui->lblFontSet->styleSheet();
  ui->lblAdditional->setStyleSheet(lbl_style);
  mui->lblDataEnc->setStyleSheet(lbl_style);
  mui->lblWebDAVUrl->setStyleSheet(lbl_style);

  ui->btnCustomFont->adjustSize();
  int hei = m_Method->getFontHeight();
  ui->btnCustomFont->setFixedHeight(4 * hei);

  // 只能输入 1~50 的整数，彻底禁止输入超过 50 的数字
  QRegularExpression rx("^(?:[1-9]|[1-4][0-9]|50)$");
  QRegularExpressionValidator* validator =
      new QRegularExpressionValidator(rx, this);
  ui->editConcurrency->setValidator(validator);

  // 多语言占位提示
  ui->editConcurrency->setPlaceholderText(tr("Enter 1~50"));

  mui->editPassword->setEchoMode(QLineEdit::EchoMode::Password);
  mui->editValidate->setEchoMode(QLineEdit::EchoMode::Password);
  // ui->editAIKey->setEchoMode(QLineEdit::EchoMode::Password);

  if (isAndroid) {
    ui->sliderFontSize->setMinimum(10);
    ui->sliderFontSize->setMaximum(16);
    ui->sliderFontSize->setValue(12);
  } else {
    ui->sliderFontSize->setMinimum(8);
    ui->sliderFontSize->setMaximum(14);
    ui->sliderFontSize->setValue(10);
  }

  initLocalModelList();

  initAIConfig();

  // 下拉菜单弹出前刷新列表
  connect(ui->cboxEndpoint, &QComboBox::showPopup, this, [this]() {
    // 缓存选中记录下标
    int selRecordIdx = -1;
    int curComboIdx = ui->cboxEndpoint->currentIndex();
    if (curComboIdx >= 0) {
      selRecordIdx = ui->cboxEndpoint->itemData(curComboIdx).toInt();
    }

    // 每次弹出下拉都从json重新加载最新数据到内存
    initAIConfig();

    ui->cboxEndpoint->clear();
    for (int i = 0; i < m_aiAllRecords.size(); ++i) {
      const auto& rec = m_aiAllRecords[i];
      ui->cboxEndpoint->addItem(rec.displayText());
      ui->cboxEndpoint->setItemData(i, i);
    }

    // 恢复选中
    if (selRecordIdx >= 0 && selRecordIdx < m_aiAllRecords.size()) {
      ui->cboxEndpoint->setCurrentIndex(selRecordIdx);
    } else if (!m_aiAllRecords.empty()) {
      ui->cboxEndpoint->setCurrentIndex(0);
    } else {
      ui->cboxEndpoint->setCurrentIndex(-1);
    }
  });
}

Preferences::~Preferences() { delete ui; }

void Preferences::keyReleaseEvent(QKeyEvent* event) { Q_UNUSED(event); }

bool Preferences::eventFilter(QObject* watch, QEvent* evn) {
  QMouseEvent* event = static_cast<QMouseEvent*>(evn);
  if (evn->type() == QEvent::KeyRelease) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evn);
    if (keyEvent->key() == Qt::Key_Back) {
      on_btnBack_clicked();
      return true;
    }
  }

  if (watch == ui->lblFontSize) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      if (devMode)
        devMode = false;
      else
        devMode = true;
      iniPreferences->setValue("/Options/DevMode", devMode);
      qDebug() << "devMode=" << devMode;
    }
  }

  return QWidget::eventFilter(watch, evn);
}

void Preferences::on_btnBack_clicked() {
  mw_one->clearWidgetFocus();
  QTimer::singleShot(10, this, [this]() { close(); });
}

void Preferences::saveOptions() {
  if (this->isVisible()) {
    iniPreferences->setValue("/Options/FontSize", ui->sliderFontSize->value());
    iniPreferences->setValue("/Options/Dark", ui->chkDark->isChecked());
    iniPreferences->setValue("/Options/chkUIFont", ui->chkUIFont->isChecked());
    iniPreferences->setValue("/Options/maxcon", ui->editConcurrency->text());
    iniPreferences->setValue("/Options/ai", ui->chkAI->isChecked());
    iniPreferences->setValue("/Options/aiindex",
                             ui->cboxEndpoint->currentIndex());
  }

  iniPreferences->setValue("/Options/Zip", mui->chkZip->isChecked());
  QString password = mui->editPassword->text().trimmed();
  QString aesStr = m_CloudBackup->aesEncrypt(password, aes_key0, aes_iv0);
  iniPreferences->setValue("/zip/password", aesStr);

  isEncrypt = mui->chkZip->isChecked();
  if (isEncrypt)
    encPassword = password;
  else
    encPassword = "";
}

void Preferences::on_sliderFontSize_sliderMoved(int position) {
  if (isVisible()) {
    QFont font;

    font.setPointSize(position * fontScale);

    ui->lblFontSize->setFont(font);
    isFontChange = true;

    qApp->setFont(font);

    iniPreferences->setValue("/Options/FontSize", position);
    fontSize = position * fontScale;

    getCheckStatusChange();
  }

  // ======================= 超级简化版 =======================
  // 自动计算 0~6 级：0=极小 1=小 2=默认 3=大 4=超大 5=特大 6=最大
  int min = ui->sliderFontSize->minimum();
  int max = ui->sliderFontSize->maximum();
  int totalLevels = 7;  // 固定7档：极小~最大

  // 自动计算当前是第几档
  int level = 0;
  if (max > min) {
    level = (position - min) * totalLevels / (max - min);
    level = qBound(0, level, totalLevels - 1);  // 限制 0~6
  }

  // 自动匹配文字
  QStringList levels = {tr("ExtraSmall"), tr("Small"),  tr("Default"),
                        tr("Large"),      tr("XLarge"), tr("XXLarge"),
                        tr("XXXLarge")};

  QString sizeLevel = levels[level];
  // ==========================================================

  ui->lblFontSize->setText(tr("Font Size") + " : " + sizeLevel);
}

void Preferences::on_btnCustomFont_clicked() {
  QString fileName;
  fileName = QFileDialog::getOpenFileName(this, tr("Font"), "",
                                          tr("Font Files (*.*)"));
  if (fileName == "") return;

  QString fontName =
      setFontDemoUI(fileName, ui->btnCustomFont, ui->sliderFontSize->value());
  isFontChange = true;

  iniPreferences->setValue("/Options/CustomFont", fileName);
  iniPreferences->setValue("/Options/CustomFontName", fontName);

  QFont font = this->font();
  font.setFamily(fontName);
  if (ui->chkUIFont->isChecked()) qApp->setFont(font);

  getCheckStatusChange();
}

QString Preferences::setFontDemoUI(QString customFontPath, QToolButton* btn,
                                   int fontSize) {
  // 0. 安全校验
  if (customFontPath.isEmpty() || !btn) return "";

  QString fontName;
  int loadedFontID = -1;

  // 1. 移除旧字体 (安全方式)
  if (!mw_one->initMain && uiFontID != -1) {
    QFontDatabase::removeApplicationFont(uiFontID);
    uiFontID = -1;
  }

  // 2. 加载新字体 (同步操作，无需Sleep)
  loadedFontID = QFontDatabase::addApplicationFont(customFontPath);
  if (loadedFontID == -1) return "";

  // 3. 获取字体族
  QStringList loadedFontFamilies =
      QFontDatabase::applicationFontFamilies(loadedFontID);
  if (loadedFontFamilies.isEmpty()) {
    QFontDatabase::removeApplicationFont(loadedFontID);
    return "";
  }
  fontName = loadedFontFamilies.at(0);
  uiFontID = loadedFontID;

  // 4. 安全获取字体样式
  QStringList styles = QFontDatabase::styles(fontName);
  QString style = styles.isEmpty() ? "Normal" : styles.at(0);

  // 5. 设置字体
  QFont f;
  uiFontWeight =
      static_cast<QFont::Weight>(QFontDatabase::weight(fontName, style));
  f.setFamily(fontName);
  f.setWeight(uiFontWeight);
  f.setPointSize(fontSize);
  btn->setFont(f);

  // 6. 跨平台路径显示
  QFileInfo fi(customFontPath);
  QString displayName = fi.fileName();
  btn->setText(tr("Custom Font") + "\n" + fontName + "\n" + displayName);

  return fontName;
}

void Preferences::on_chkUIFont_clicked() {
  if (ui->btnCustomFont->text() == tr("Custom Font")) {
    ui->chkUIFont->setChecked(false);
    return;
  }
  isFontChange = true;
  getCheckStatusChange();
}

void Preferences::on_sliderFontSize_valueChanged(int value) {
  on_sliderFontSize_sliderMoved(value);
}

void Preferences::setDefaultFont(QString fontFamily) {
  iniPreferences->setValue("/Options/DefaultFont", fontFamily);
}

QString Preferences::getDefaultFont() {
  return iniPreferences->value("/Options/DefaultFont", "None").toString();
}

bool Preferences::isOverUIFont() {
  bool chkUIFont = iniPreferences->value("/Options/chkUIFont", false).toBool();
  return chkUIFont;
}

bool Preferences::isOverReaderFont() {
  bool chkReaderFont =
      iniPreferences->value("/Options/chkReaderFont", false).toBool();
  return chkReaderFont;
}

void Preferences::initOptions() {
  bool chkUIFont = iniPreferences->value("/Options/chkUIFont", false).toBool();
  ui->chkUIFont->setChecked(chkUIFont);

  ui->chkDark->setChecked(
      iniPreferences->value("/Options/Dark", false).toBool());
  ui->chkAI->setChecked(iniPreferences->value("/Options/ai", false).toBool());
  ui->cboxEndpoint->setCurrentIndex(
      iniPreferences->value("/Options/aiindex", 0).toInt());

  ui->cboxModel->setCurrentText(
      iniPreferences->value("/Options/localmodel", modelFileName).toString());

  QString aesStr = iniPreferences->value("/zip/password").toString();
  QString password = m_CloudBackup->aesDecrypt(aesStr, aes_key0, aes_iv0);
  mui->editPassword->setText(password);
  mui->editValidate->setText(password);

  maxNetConcurrent = iniPreferences->value("/Options/maxcon", 10).toInt();
  ui->editConcurrency->setText(QString::number(maxNetConcurrent));

  bool isZip = iniPreferences->value("/Options/Zip", false).toBool();
  mui->chkZip->setChecked(isZip);
  isEncrypt = isZip;
  if (isEncrypt)
    encPassword = password;
  else
    encPassword = "";

  devMode = iniPreferences->value("/Options/DevMode", false).toBool();
#ifdef Q_OS_ANDROID
#else

  if (!devMode) {
    mui->btnHome->hide();
    mui->qwMainDate->hide();
    mui->qwMainEvent->hide();
    mui->btnSteps->hide();

    mui->btnReader->hide();
    mui->btnAdd->hide();
    mui->btnDel->hide();
    mui->btnFind->hide();

    mui->qwMainTab->hide();
    mui->btnSelTab->hide();
    mui->lblStats->hide();

    int s = 120;
    int qs = s - 40;
    mui->btnTodo->setFixedHeight(s);
    mui->btnTodo->setFixedWidth(s);
    mui->btnTodo->setIconSize(QSize(qs, qs));
    mui->btnNotes->setFixedHeight(s);
    mui->btnNotes->setFixedWidth(s);
    mui->btnNotes->setIconSize(QSize(qs, qs));
  }
#endif

  QString customFontFile =
      iniPreferences->value("/Options/CustomFont").toString();
  setFontDemoUI(customFontFile, ui->btnCustomFont, ui->sliderFontSize->value());

  QString readerFontFile =
      iniPreferences->value("/Options/ReaderFont").toString();
  QString readerFont;
  if (QFile::exists(readerFontFile))
    readerFont = setFontDemoUI(readerFontFile, mui->btnFont, fontSize);
  else
    readerFont = defaultFontFamily;
  mui->qwReader->rootContext()->setContextProperty("FontName", readerFont);
  mui->qwReader->rootContext()->setContextProperty("FontWeight",
                                                   readerFontWeight);
}

void Preferences::on_btnReStart_clicked() {
  saveOptions();

#ifdef Q_OS_ANDROID
  // 1. 获取MyActivity实例（关键：用自己的Activity，而非QtActivity）
  QJniObject activity = QNativeInterface::QAndroidApplication::context();
  if (activity.isValid()) {
    // 标记需要重启（兜底）
    activity.callMethod<void>("markNeedRestart");
    // 主动触发重启（核心：直接调用Android侧重启方法）
    activity.callMethod<void>("triggerRestart");
  }
  // 2. 仅关闭Qt窗口，不强制exit（避免Qt析构崩溃）
  if (mw_one) {
    mw_one->close();
  }
#else

  QTimer::singleShot(1000, mw_one, []() {
    QProcess::startDetached(qApp->applicationFilePath(), QStringList());
  });

  mw_one->close();

#endif
}

void Preferences::setBakStatus(bool status) {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);

  Reg.setValue("/BakFiles/BakStatus", status);
}

bool Preferences::getBakStatus() {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);

  return Reg.value("/BakFiles/BakStatus", 0).toBool();
}

void Preferences::setLatestAction(QString action) {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);

  Reg.setValue("/BakFiles/BakAction", action);

  setBakStatus(false);
}

void Preferences::appendBakFile(QString action, QString bakfile) {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);

  int count = Reg.value("/BakFiles/BakCount", 0).toInt();
  count++;
  Reg.setValue("BakFiles/BakCount", count);
  Reg.setValue("/BakFiles/Action" + QString::number(count - 1), action);
  Reg.setValue("/BakFiles/File" + QString::number(count - 1), bakfile);
}

QStringList Preferences::getBakFilesList() {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);

  QStringList fileList;
  QString action, bakfile;
  int count = Reg.value("/BakFiles/BakCount", 0).toInt();
  for (int i = 0; i < count; i++) {
    action = Reg.value("/BakFiles/Action" + QString::number(i)).toString();
    bakfile = Reg.value("/BakFiles/File" + QString::number(i)).toString();

    QFile file(bakfile);
    if (file.exists()) {
      fileList.append(action + "-===-" + bakfile);
    }
  }

  QStringList uniqueList;
  QSet<QString> seen;

  for (const QString& str : fileList) {
    if (!seen.contains(str)) {
      seen.insert(str);
      uniqueList.append(str);
    }
  }
  fileList = uniqueList;

  return fileList;
}

void Preferences::on_chkDark_clicked(bool checked) {
  isDark = checked;
  getCheckStatusChange();
}

void Preferences::initCheckStatus() {
  if (isVisible()) {
    listCheckStatus.clear();
    listCheckStatus.append(ui->chkUIFont->isChecked());

    listCheckStatus.append(ui->chkDark->isChecked());

    listCheckStatus.append(ui->sliderFontSize->value());

    orgCustomFontText = ui->btnCustomFont->text().trimmed();
  }
}

void Preferences::getCheckStatusChange() {
  isChanged = false;
  if (ui->chkUIFont->isChecked() != static_cast<bool>(listCheckStatus.at(0)))
    isChanged = true;

  if (ui->chkDark->isChecked() != static_cast<bool>(listCheckStatus.at(1)))
    isChanged = true;

  if (ui->sliderFontSize->value() != listCheckStatus.at(2)) isChanged = true;

  if (orgCustomFontText != ui->btnCustomFont->text().trimmed())
    isChanged = true;

  if (isChanged)
    ui->btnReStart->hide();
  else
    ui->btnReStart->hide();
}

void Preferences::on_chkZip_clicked() {
  if (mui->editPassword->text().trimmed() == "" ||
      mui->editValidate->text().trimmed() == "") {
    mui->chkZip->setChecked(false);
  }

  if (mui->editPassword->text().trimmed() !=
      mui->editValidate->text().trimmed()) {
    mui->chkZip->setChecked(false);
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg("Knot", tr("Password validation error."), 1);

    return;
  }

  saveOptions();
}

void Preferences::on_editPassword_textChanged(const QString& arg1) {
  if (arg1.length() > 0) mui->chkZip->setChecked(false);
}

void Preferences::on_editValidate_textChanged(const QString& arg1) {
  on_editPassword_textChanged(arg1);
}

void Preferences::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event);
  closeTextToolBar();
  saveOptions();
  setEncSyncStatusTip();

  if (isChanged && isVisible()) {
    mui->frameMain->hide();
    loadTheme(isDark);
    mui->frameMain->show();
  }

  if (isLocalAIModel)
    mui->lblVectorStatus->show();
  else
    mui->lblVectorStatus->hide();
  m_NotesList->rebuilderNotesVector();
}

void Preferences::on_btnShowPassword_pressed() {
  mui->editPassword->setEchoMode(QLineEdit::EchoMode::Normal);
}

void Preferences::on_btnShowPassword_released() {
  mui->editPassword->setEchoMode(QLineEdit::EchoMode::Password);
}

void Preferences::on_btnShowValidate_pressed() {
  mui->editValidate->setEchoMode(QLineEdit::EchoMode::Normal);
}

void Preferences::on_btnShowValidate_released() {
  mui->editValidate->setEchoMode(QLineEdit::EchoMode::Password);
}

void Preferences::on_chkDark_clicked() {}

void Preferences::setEncSyncStatusTip() {
  mui->lblStats->setStyleSheet(mw_one->labelNormalStyleSheet);

  if (mui->chkZip->isChecked() && mui->chkAutoSync->isChecked() &&
      mui->chkWebDAV->isChecked())
    mui->lblStats->setStyleSheet(mw_one->labelEnSyncStyleSheet);

  if (mui->chkZip->isChecked() && !mui->chkAutoSync->isChecked() &&
      !mui->chkWebDAV->isChecked())
    mui->lblStats->setStyleSheet(mw_one->labelEncStyleSheet);

  if (mui->chkZip->isChecked() && !mui->chkAutoSync->isChecked() &&
      mui->chkWebDAV->isChecked())
    mui->lblStats->setStyleSheet(mw_one->labelEncStyleSheet);

  if (mui->chkZip->isChecked() && mui->chkAutoSync->isChecked() &&
      !mui->chkWebDAV->isChecked())
    mui->lblStats->setStyleSheet(mw_one->labelEncStyleSheet);

  if (!mui->chkZip->isChecked() && mui->chkAutoSync->isChecked() &&
      mui->chkWebDAV->isChecked())
    mui->lblStats->setStyleSheet(mw_one->labelSyncStyleSheet);

  if (isAndroid)
    mui->lblStatus->hide();
  else {
    mui->lblStatus->setText("Knot   V:" + ver);
    mui->lblStatus->setStyleSheet(mui->lblStats->styleSheet());
  }
}

void Preferences::openPreferences() {
  m_NotesList->safeExitLlama();

  int x, y;
  if (isAndroid) {
    setFixedWidth(mw_one->width());
    setFixedHeight(mw_one->height());
    x = mw_one->geometry().x();
    y = mw_one->geometry().y();

  } else {
    x = mw_one->geometry().x() + (width() - width()) / 2;
    y = mw_one->geometry().y() + (height() - height()) / 2;
    setFixedWidth(350);
    setFixedHeight(mw_one->height());
  }

  if (y < 0) y = 50;

  setGeometry(x, y, width(), height());
  setModal(true);

  ui->sliderFontSize->setStyleSheet(mui->hsM->styleSheet());

  int savedPosition =
      iniPreferences->value("/Options/FontSize", defaultFontSize).toInt();
  ui->sliderFontSize->setValue(savedPosition);

  ui->lblModelStatus->setText(modelStatus);
  QString text = ui->cboxModel->currentText();
  if (text.isEmpty()) {
    ui->btnSelectModel->setText(tr("Select Model"));
  } else
    ui->btnSelectModel->setText(text);

  m_NotesList->cancelRebuildNotesVector();
  initLocalModelList();

  show();
  initCheckStatus();

  // init edit toolbar
  initTextToolbarDynamic(this);
  if (editFilter != nullptr) {
    mui->editPassword->removeEventFilter(editFilter);
    mui->editValidate->removeEventFilter(editFilter);
    ui->editEndpoint->removeEventFilter(editFilter);
    ui->editAIKey->removeEventFilter(editFilter);
    ui->editAIModelID->removeEventFilter(editFilter);
    delete editFilter;
    editFilter = nullptr;
  }
  editFilter = new EditEventFilter(textToolbarDynamic, this);

  mui->editPassword->installEventFilter(editFilter);
  mui->editValidate->installEventFilter(editFilter);
  ui->editEndpoint->installEventFilter(editFilter);
  ui->editAIKey->installEventFilter(editFilter);
  ui->editAIModelID->installEventFilter(editFilter);

  isChanged = false;
}

void Preferences::on_chkUIFont_clicked(bool checked) {
  QFont font = this->font();
  if (!checked) {
    font.setFamily(defaultFontFamily);
    qApp->setFont(font);
    ui->chkUIFont->setChecked(false);
  } else {
    QString fontName =
        iniPreferences->value("/Options/CustomFontName", defaultFontFamily)
            .toString();
    font.setFamily(fontName);
    qApp->setFont(font);
    ui->chkUIFont->setChecked(true);
  }
}

void Preferences::on_btnAISelect_clicked() {
  // 1. 创建菜单
  QMenu* menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  // 确保菜单获取焦点，以便 Android 返回键可用
  menu->setFocusPolicy(Qt::StrongFocus);
  menu->setFocus();

  // 基础样式：隐藏勾选框，优化滚动条
  menu->setStyleSheet(R"(
        QMenu {
            padding: 5px;
            border: 1px solid #cccccc;
            border-radius: 8px;
        }
        QMenu::scroller {
            width: 4px;
            background: transparent;
        }
        QMenu::indicator {
            width: 0px;
        }
    )");

  // 2. 准备颜色变量 (循环外获取，提升性能)
  QPalette pal = ui->cboxEndpoint->palette();
  QString textColor = pal.color(QPalette::Text).name();
  QString placeholderColor = pal.color(QPalette::PlaceholderText).name();
  QString highlightBg = pal.color(QPalette::Highlight).name();
  QString highlightText = pal.color(QPalette::HighlightedText).name();
  QString highlightSub =
      pal.color(QPalette::HighlightedText).darker(120).name();

  // 3. 遍历并添加条目
  for (int i = 0; i < ui->cboxEndpoint->count(); ++i) {
    QString rawText = ui->cboxEndpoint->itemText(i);
    QStringList parts = rawText.split("||", Qt::SkipEmptyParts);

    QString title = parts.value(0).trimmed();
    QString subtitle;

    // 解析副标题逻辑
    if (parts.size() >= 3) {
      QString path = parts.value(1).trimmed();
      QString key = parts.value(2).trimmed();
      // 简单的 Key 脱敏处理
      QString maskedKey = key.length() > 4 ? key.left(4) + "••••••" : key;
      subtitle = QString("%1 | %2").arg(path, maskedKey);
    } else {
      subtitle = parts.mid(1).join("||").trimmed();
    }

    // 根据是否选中，决定文字颜色
    bool isSelected = (i == ui->cboxEndpoint->currentIndex());
    QString currentTitleColor = isSelected ? highlightText : textColor;
    QString currentSubColor = isSelected ? highlightSub : placeholderColor;

    // 构建 HTML 内容
    QString html =
        QString(
            "<div style='line-height:1.4; padding: 4px 0;'>"
            "  <div style='font-size:15px; font-weight:600; color:%1;'>%2</div>"
            "  <div style='font-size:12px; color:%3; margin-top:2px;'>%4</div>"
            "</div>")
            .arg(currentTitleColor, title.toHtmlEscaped(), currentSubColor,
                 subtitle.toHtmlEscaped());

    // 创建 Label 容器
    QLabel* label = new QLabel(html);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setContentsMargins(15, 10, 15, 10);  // 设置内边距，增加点击区域感

    // 如果是选中项，给 Label 设置一个背景色块 (静态高亮)
    if (isSelected) {
      label->setStyleSheet(QString("background-color: %1; border-radius: 6px;")
                               .arg(highlightBg));
    } else {
      label->setStyleSheet("background: transparent; border-radius: 6px;");
    }

    // 使用 QWidgetAction 将 Label 放入菜单
    QWidgetAction* widgetAction = new QWidgetAction(menu);
    widgetAction->setDefaultWidget(label);

    // 绑定点击事件
    connect(widgetAction, &QWidgetAction::triggered, this, [this, i]() {
      ui->cboxEndpoint->setCurrentIndex(i);
      // 这里可以添加额外的保存逻辑，如果需要的话
    });

    menu->addAction(widgetAction);
  }

  // 4. 计算弹出位置 (兼容 Qt6)
  QPoint pos =
      ui->btnAISelect->mapToGlobal(QPoint(0, ui->btnAISelect->height()));

  // 防止菜单超出屏幕底部 (简单处理)
  // 注意：Qt6 中去掉了 qApp->desktop()，改用 screenAt 或 primaryScreen
  if (QScreen* screen = QGuiApplication::screenAt(pos)) {
    QRect screenGeom = screen->availableGeometry();
    int maxH = screenGeom.height() * 0.6;
    // 如果位置太靠下，尝试向上弹出 (可选)
    if (pos.y() + maxH > screenGeom.bottom()) {
      pos.setY(screenGeom.bottom() - maxH);
    }
  }

  menu->popup(pos);
}

void Preferences::on_btnAITest_clicked() {
  isTestBtnClicked = true;
  mw_one->aiChatQuery("Hello!");

  return;

  QString ep = ui->editEndpoint->text().trimmed();
  QString key = ui->editAIKey->text().trimmed();
  QString mid = ui->editAIModelID->text().trimmed();

  if (ep.isEmpty() || key.isEmpty() || mid.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Warning"),
                 tr("Endpoint / API Key / Model ID cannot be empty"), 1);
    return;
  }

  AiSingleRecord tempCfg;
  tempCfg.endpoint = ep;
  tempCfg.apiKey = key;
  tempCfg.modelId = mid;
  tempCfg.temperature = 0.1;
  tempCfg.timeoutSec = 10;
  tempCfg.maxTokens = 1024;

  // 只做连通，无后续操作，传空回调
  mw_one->checkAiConnectivity(tempCfg, nullptr);
}

void Preferences::saveAIConfig() {
  // 读取界面输入
  QString endpoint = ui->editEndpoint->text().trimmed();
  QString apiKey = ui->editAIKey->text().trimmed();
  QString modelId = ui->editAIModelID->text().trimmed();

  // 空值校验
  if (endpoint.isEmpty() || apiKey.isEmpty() || modelId.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Warning"),
                 tr("Endpoint / API Key / Model ID cannot be empty"), 1);
    return;
  }

  // 构造当前记录
  AiSingleRecord newRec;
  newRec.endpoint = endpoint;
  newRec.apiKey = apiKey;
  newRec.modelId = modelId;
  newRec.temperature = 0.1;
  newRec.timeoutSec = 10;
  newRec.maxTokens = 1024;

  // 判断整套三元组是否已存在，存在则覆盖参数，不存在追加
  bool found = false;
  for (auto& item : m_aiAllRecords) {
    if (item.uniqueKey() == newRec.uniqueKey()) {
      item = newRec;
      found = true;
      break;
    }
  }
  if (!found) m_aiAllRecords.append(newRec);

  // 组装JSON写入文件
  QJsonArray arr;
  for (const auto& rec : m_aiAllRecords) {
    QJsonObject obj;
    obj["endpoint"] = rec.endpoint;
    obj["apiKey"] = rec.apiKey;
    obj["modelId"] = rec.modelId;
    obj["temperature"] = rec.temperature;
    obj["timeoutSec"] = rec.timeoutSec;
    obj["maxTokens"] = rec.maxTokens;
    arr.append(obj);
  }
  QJsonObject root;
  root["all_records"] = arr;

  QString filePath = privateDir + "/" + ai_config_json;
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    auto msg = std::make_unique<ShowMessage>(this);
    msg->showMsg(tr("Save Failed"), tr("Cannot open config file to write"), 1);
    return;
  }
  file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  file.close();

  // 保存完成，立刻刷新下拉，无需用户点开弹窗才看得到
  int selRecordIdx = -1;
  int curComboIdx = ui->cboxEndpoint->currentIndex();
  if (curComboIdx >= 0) {
    selRecordIdx = ui->cboxEndpoint->itemData(curComboIdx).toInt();
  }

  ui->cboxEndpoint->clear();
  for (int i = 0; i < m_aiAllRecords.size(); ++i) {
    const auto& rec = m_aiAllRecords[i];
    ui->cboxEndpoint->addItem(rec.displayText());
    ui->cboxEndpoint->setItemData(i, i);
  }

  if (selRecordIdx >= 0 && selRecordIdx < m_aiAllRecords.size()) {
    ui->cboxEndpoint->setCurrentIndex(selRecordIdx);
  } else if (!m_aiAllRecords.empty()) {
    ui->cboxEndpoint->setCurrentIndex(0);
  }
}

void Preferences::initAIConfig() {
  QString filePath = privateDir + "/" + ai_config_json;
  QFile file(filePath);
  m_aiAllRecords.clear();
  ui->cboxEndpoint->clear();

  if (!file.exists()) return;
  if (!file.open(QIODevice::ReadOnly)) return;

  QByteArray raw = file.readAll();
  file.close();
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
  if (err.error != QJsonParseError::NoError) return;

  QJsonObject root = doc.object();
  QJsonArray arr = root["all_records"].toArray();
  for (const auto& val : arr) {
    QJsonObject obj = val.toObject();
    AiSingleRecord rec;
    rec.endpoint = obj["endpoint"].toString();
    rec.apiKey = obj["apiKey"].toString();
    rec.modelId = obj["modelId"].toString();
    rec.temperature = obj["temperature"].toDouble(0.1);
    rec.timeoutSec = obj["timeoutSec"].toInt(10);
    rec.maxTokens = obj["maxTokens"].toInt(1024);
    m_aiAllRecords.append(rec);
  }

  // 复用结构体 displayText，绑定数组下标 itemData
  for (int i = 0; i < m_aiAllRecords.size(); ++i) {
    const auto& rec = m_aiAllRecords[i];
    ui->cboxEndpoint->addItem(rec.displayText());
    ui->cboxEndpoint->setItemData(i, i);
  }
}

void Preferences::on_cboxEndpoint_currentIndexChanged(int index) {
  // 仅清空密钥、模型输入框，绝不清空下拉控件本身
  ui->editAIKey->clear();
  ui->editAIModelID->clear();

  if (index < 0 || index >= m_aiAllRecords.size()) return;

  // 从隐藏数据获取真实记录下标，不依赖combo原生index
  int recordIdx = ui->cboxEndpoint->itemData(index).toInt();
  if (recordIdx < 0 || recordIdx >= m_aiAllRecords.size()) return;

  // 直接通过下标取完整原始记录，完全不依赖cbox的currentText
  const AiSingleRecord& rec = m_aiAllRecords[recordIdx];

  // 强制回填纯净原始endpoint，不受下拉展示文本干扰
  ui->editEndpoint->setText(rec.endpoint);
  ui->editAIKey->setText(rec.apiKey);
  ui->editAIModelID->setText(rec.modelId);
}

void Preferences::on_cboxEndpoint_activated(int index) {
  on_cboxEndpoint_currentIndexChanged(index);
}

void Preferences::on_btnDownloadModel_clicked() {
  // FP32:
  // https://hf-mirror.com/rodion-m/multilingual-e5-small-gguf/tree/main

  // Q8:
  // https://hf-mirror.com/keisuke-miyako/multilingual-e5-small-gguf-q8_0/tree/main

  // embeddinggemma:
  // https://hf-mirror.com/ggml-org/embeddinggemma-300M-GGUF/tree/main

  const QString targetUrl =
      "https://hf-mirror.com/rodion-m/multilingual-e5-small-gguf/tree/main";
  if (!QDesktopServices::openUrl(QUrl(targetUrl))) {
    // 打开失败提示
    qDebug() << "浏览器打开链接失败";
  }
}

void Preferences::on_cboxModel_currentIndexChanged(int index) {
  Q_UNUSED(index);
  if (this->isVisible()) {
    iniPreferences->setValue("/Options/localmodel",
                             ui->cboxModel->currentText());
  }

  if (this->isHidden()) {
    ui->lblModelTip->setText(
        computeModelFingerprint(modelFullPath + ui->cboxModel->currentText()));
  }
}

void Preferences::initLocalModelList() {
  QString curText = ui->cboxModel->currentText();

  QString path = privateDir + "model";

  QDir dir(path);

  // 配置规则：只取文件、不遍历子目录、忽略 . 和 ..
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  // 不排序（可选，按需开启 QDir::Name）
  dir.setSorting(QDir::NoSort);

  QFileInfoList fileInfos = dir.entryInfoList();
  QStringList result;

  for (int i = 0; i < fileInfos.size(); ++i) {
    const QFileInfo& info = fileInfos[i];
    if (info.fileName().contains("note_vector")) continue;

    // 仅文件名
    result << info.fileName();
    // 完整路径放开下面这句
    // result << info.absoluteFilePath();
  }

  ui->cboxModel->clear();
  ui->cboxModel->addItems(result);

  if (result.contains(curText)) {
    ui->cboxModel->setCurrentText(curText);
  }
}

void Preferences::on_cboxModel_currentTextChanged(const QString& arg1) {
  Q_UNUSED(arg1);
  if (this->isVisible()) {
    releaseGlobalAiEngine();

    modelFileName = arg1;
    modelFingerprint = computeModelFingerprint(modelFullPath + modelFileName);
    ui->lblModelTip->setText(modelFingerprint);
    int value = init_main_ai();
    if (value == 1) return;

    isLocalAIModel = initGlobalAiEngine();
    ui->lblModelStatus->setText(modelStatus);
    if (isLocalAIModel) {
      m_NotesList->initVectorSearchService();

      qCritical() << "模型切换成功";
    } else {
      qCritical() << "模型切换失败";
    }
  }
}

void Preferences::on_btnSelectModel_clicked() {
  // 构建自定义弹窗，数据完全从原 ComboBox 同步
  QMenu* menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  // ✅ 关键:确保菜单获取焦点，从而接收 Android 返回键事件
  menu->setFocusPolicy(Qt::StrongFocus);
  menu->setFocus();

  for (int i = 0; i < ui->cboxModel->count(); ++i) {
    QAction* action = menu->addAction(ui->cboxModel->itemText(i));

    // 标记当前选中项
    if (i == ui->cboxModel->currentIndex()) {
      action->setCheckable(true);
      action->setChecked(true);
    }

    // 点击时：1.设置ComboBox索引(触发原有业务逻辑) 2.更新按钮标题
    connect(action, &QAction::triggered, this, [this, i]() {
      ui->cboxModel->setCurrentIndex(i);
      ui->btnSelectModel->setText(ui->cboxModel->currentText());
    });
  }

  // 以按钮为锚点弹出
  menu->popup(
      ui->btnSelectModel->mapToGlobal(QPoint(0, ui->btnSelectModel->height())));
}
