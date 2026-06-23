#include "Preferences.h"

#include <QKeyEvent>

#include "MainWindow.h"
#include "src/defines.h"
#include "ui_MainWindow.h"
#include "ui_Preferences.h"

QFont::Weight readerFontWeight;
extern void loadTheme(bool isDark);

Preferences::Preferences(QWidget* parent)
    : QDialog(parent), ui(new Ui::Preferences) {
  ui->setupUi(this);

  m_Method->set_ToolButtonStyle(this);

  this->installEventFilter(this);
  ui->lblFontSize->installEventFilter(this);

  ui->gboxAdditional->hide();
  ui->lblAdditional->hide();

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

  if (isAndroid) {
    ui->sliderFontSize->setMinimum(10);
    ui->sliderFontSize->setMaximum(16);
    ui->sliderFontSize->setValue(12);
  } else {
    ui->sliderFontSize->setMinimum(8);
    ui->sliderFontSize->setMaximum(14);
    ui->sliderFontSize->setValue(10);
  }
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

#ifdef Q_OS_WIN

#else

#endif

#ifdef Q_OS_ANDROID

#endif

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

  show();
  initCheckStatus();

  // init edit toolbar
  initTextToolbarDynamic(this);
  if (editFilter != nullptr) {
    mui->editPassword->removeEventFilter(editFilter);
    mui->editValidate->removeEventFilter(editFilter);
    delete editFilter;
    editFilter = nullptr;
  }
  editFilter = new EditEventFilter(textToolbarDynamic, this);

  mui->editPassword->installEventFilter(editFilter);
  mui->editValidate->installEventFilter(editFilter);

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
