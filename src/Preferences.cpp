#include "Preferences.h"

#include <QKeyEvent>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_Preferences.h"
extern QString iniFile, iniDir, privateDir, defaultFontFamily, customFontFamily,
    encPassword;
extern MainWindow* mw_one;
extern Method* m_Method;
extern bool isBreak, isDark, isEncrypt;
extern int fontSize;
extern QSettings* iniPreferences;
extern ReaderSet* m_ReaderSet;
extern CloudBackup* m_CloudBackup;

QFont::Weight readerFontWeight;

Preferences::Preferences(QWidget* parent)
    : QDialog(parent), ui(new Ui::Preferences) {
  ui->setupUi(this);

  QFont font0 = m_Method->getNewFont(20);
  ui->btnCustomFont->setFont(font0);
  ui->chkUIFont->setFont(font0);
  ui->chkAutoTime->setFont(font0);
  ui->chkZip->setFont(font0);
  ui->btnReStart->setFont(font0);

#ifdef Q_OS_ANDROID
  ui->sliderFontSize->setMinimum(15);

  font0.setBold(true);
  font0.setPointSize(13);
  ui->lblZipTip->setFont(font0);
#else
  ui->sliderFontSize->setMinimum(9);

  font0.setBold(true);
  font0.setPointSize(9);
  ui->lblZipTip->setFont(font0);
#endif

  mw_one->set_ToolButtonStyle(this);

  this->installEventFilter(this);
  ui->lblFontSize->installEventFilter(this);

  ui->chkAutoTime->hide();
  ui->chkAniEffects->hide();
  ui->lblZipTip->hide();

  ui->lblFontSize->setText(tr("Font Size") + " : " + QString::number(fontSize));
  isFontChange = false;

  chkStyle = ui->chkAutoTime->styleSheet();
  ui->chkDark->setStyleSheet(chkStyle);
  ui->chkZip->setStyleSheet(chkStyle);
  ui->chkUIFont->setStyleSheet(chkStyle);
  ui->lblFontSize->setFixedHeight(40);

  QString lbl_style = ui->lbl1->styleSheet();
  ui->lbl2->setStyleSheet(lbl_style);
  ui->lbl3->setStyleSheet(lbl_style);

  ui->btnCustomFont->adjustSize();
  ui->btnCustomFont->setStyleSheet(
      "background-color: rgb(255, 255, 255);color:black;");

  ui->lblZipTip->adjustSize();
  ui->lblZipTip->setWordWrap(true);
  ui->editPassword->setEchoMode(QLineEdit::EchoMode::Password);
  ui->editValidate->setEchoMode(QLineEdit::EchoMode::Password);
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

void Preferences::on_btnBack_clicked() { close(); }

void Preferences::saveOptions() {
  iniPreferences->setValue("/Options/FontSize", ui->sliderFontSize->value());
  iniPreferences->setValue("/Options/Dark", ui->chkDark->isChecked());
  iniPreferences->setValue("/Options/AutoTimeY", ui->chkAutoTime->isChecked());
  iniPreferences->setValue("/Options/Zip", ui->chkZip->isChecked());
  iniPreferences->setValue("/Options/chkUIFont", ui->chkUIFont->isChecked());
  iniPreferences->setValue("/Options/chkAniEffects",
                           ui->chkAniEffects->isChecked());

  QString password = ui->editPassword->text().trimmed();
  QString aesStr = m_CloudBackup->aesEncrypt(password, aes_key, aes_iv);
  iniPreferences->setValue("/zip/password", aesStr);

  isEncrypt = ui->chkZip->isChecked();
  if (isEncrypt)
    encPassword = password;
  else
    encPassword = "";
}

void Preferences::on_sliderFontSize_sliderMoved(int position) {
  if (isVisible()) {
    QFont font;
    font.setPointSize(position);
    ui->lblFontSize->setText(tr("Font Size") + " : " +
                             QString::number(position));
    ui->lblFontSize->setFont(font);
    isFontChange = true;

    getCheckStatusChange();
  }
}

void Preferences::on_btnCustomFont_clicked() {
  QString fileName;
  fileName = QFileDialog::getOpenFileName(this, tr("Font"), "",
                                          tr("Font Files (*.*)"));
  if (fileName == "") return;

#ifdef Q_OS_ANDROID
  fileName = m_Method->getRealPathFile(fileName);
#endif

  setFontDemoUI(fileName, ui->btnCustomFont, ui->sliderFontSize->value());
  isFontChange = true;

  iniPreferences->setValue("/Options/CustomFont", fileName);

  getCheckStatusChange();
}

QString Preferences::setFontDemoUI(QString customFontPath, QToolButton* btn,
                                   int fontSize) {
  QString fontName;
  int loadedFontID;
  QStringList loadedFontFamilies;
  QFont f;
  QString style;

  if (!mw_one->initMain) fontDatabaseUI.removeAllApplicationFonts();
  loadedFontID = fontDatabaseUI.addApplicationFont(customFontPath);
  loadedFontFamilies = fontDatabaseUI.applicationFontFamilies(loadedFontID);
  uiFontID = loadedFontID;

  if (!loadedFontFamilies.empty()) {
    fontName = loadedFontFamilies.at(0);

    QStringList styles = fontDatabaseUI.styles(fontName);
    style = styles.at(0);
    uiFontWeight = (QFont::Weight)fontDatabaseUI.weight(fontName, style);

    f.setWeight(uiFontWeight);
    f.setFamily(fontName);
    f.setPointSize(fontSize);

    btn->setFont(f);

    QString str = customFontPath;
#ifdef Q_OS_ANDROID
    str = mw_one->m_Reader->getUriRealPath(customFontPath);
#endif

    QStringList list = str.split("/");
    QString str1 = list.at(list.count() - 1);
    btn->setText(tr("Custom Font") + "\n" + fontName + "\n" + str1);
  }

  return fontName;
}

QString Preferences::setFontDemo(QString customFontPath, QToolButton* btn,
                                 int fontSize) {
  QString fontName;
  int loadedFontID;
  QStringList loadedFontFamilies;
  QFont f;
  QString style;

  if (!mw_one->initMain) {
    if (ui->chkUIFont->isChecked() && customFontFamily.length() > 0)
      fontDatabase.removeApplicationFont(readerFontID);
  }
  loadedFontID = fontDatabase.addApplicationFont(customFontPath);
  loadedFontFamilies = fontDatabase.applicationFontFamilies(loadedFontID);
  readerFontID = loadedFontID;

  if (!loadedFontFamilies.empty()) {
    fontName = loadedFontFamilies.at(0);

    QStringList styles = fontDatabase.styles(fontName);
    style = styles.at(0);
    readerFontWeight = (QFont::Weight)fontDatabase.weight(fontName, style);

    qDebug() << "readerFontWeight=" << readerFontWeight << loadedFontFamilies
             << loadedFontID << style;

    f.setWeight(readerFontWeight);
    f.setFamily(fontName);
    f.setPointSize(fontSize);

    btn->setFont(f);

    QString str = customFontPath;
#ifdef Q_OS_ANDROID
    str = mw_one->m_Reader->getUriRealPath(customFontPath);
#endif

    QStringList list = str.split("/");
    QString str1 = list.at(list.count() - 1);
    btn->setText(tr("Custom Font") + "\n" + fontName + "\n" + str1);
  }

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
  isDark = ui->chkDark->isChecked();
  ui->chkAutoTime->setChecked(
      iniPreferences->value("/Options/AutoTimeY", true).toBool());
  ui->chkAniEffects->setChecked(
      iniPreferences->value("/Options/chkAniEffects", true).toBool());

  QString aesStr = iniPreferences->value("/zip/password").toString();
  QString password = m_CloudBackup->aesDecrypt(aesStr, aes_key, aes_iv);
  ui->editPassword->setText(password);
  ui->editValidate->setText(password);

  bool isZip = iniPreferences->value("/Options/Zip", false).toBool();
  ui->chkZip->setChecked(isZip);
  isEncrypt = isZip;
  if (isEncrypt)
    encPassword = password;
  else
    encPassword = "";

  devMode = iniPreferences->value("/Options/DevMode", false).toBool();
#ifdef Q_OS_ANDROID
#else

  if (!devMode) {
    mw_one->ui->f_charts->hide();
    mw_one->ui->qwMainDate->hide();
    mw_one->ui->qwMainEvent->hide();
    mw_one->ui->btnSteps->hide();
    mw_one->ui->btnChart->hide();
    mw_one->ui->btnReader->hide();
    mw_one->ui->btnAdd->hide();
    mw_one->ui->btnDel->hide();
    mw_one->ui->btnFind->hide();
    mw_one->ui->btnModifyRecord->hide();
    mw_one->ui->btnReport->hide();
    mw_one->ui->f_cw->hide();
    mw_one->ui->qwMainTab->hide();
    mw_one->ui->btnSelTab->hide();
    mw_one->ui->lblStats->hide();

    ui->chkAniEffects->hide();

    int s = 120;
    int qs = s - 40;
    mw_one->ui->btnTodo->setFixedHeight(s);
    mw_one->ui->btnTodo->setFixedWidth(s);
    mw_one->ui->btnTodo->setIconSize(QSize(qs, qs));
    mw_one->ui->btnNotes->setFixedHeight(s);
    mw_one->ui->btnNotes->setFixedWidth(s);
    mw_one->ui->btnNotes->setIconSize(QSize(qs, qs));
  }
#endif

  QString customFontFile =
      iniPreferences->value("/Options/CustomFont").toString();
  setFontDemoUI(customFontFile, ui->btnCustomFont, ui->sliderFontSize->value());

  QString readerFontFile =
      iniPreferences->value("/Options/ReaderFont").toString();
  QString readerFont;
  if (QFile::exists(readerFontFile))
    readerFont = setFontDemo(readerFontFile, mw_one->ui->btnFont, fontSize);
  else
    readerFont = defaultFontFamily;
  mw_one->ui->qwReader->rootContext()->setContextProperty("FontName",
                                                          readerFont);
  mw_one->ui->qwReader->rootContext()->setContextProperty("FontWeight",
                                                          readerFontWeight);
}

void Preferences::on_btnReStart_clicked() {
  saveOptions();

#ifdef Q_OS_ANDROID

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QAndroidJniObject activity = QtAndroid::androidActivity();
  activity.callMethod<int>("setReOpen", "()I");
#else
  QJniObject activity = QJniObject::fromString("setReOpen");
  activity.callMethod<int>("setReOpen", "()I");
#endif

#else
  QProcess::startDetached(qApp->applicationFilePath(), QStringList());

#endif

  mw_one->close();
}

void Preferences::setBakStatus(bool status) {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  Reg.setValue("/BakFiles/BakStatus", status);
}

bool Preferences::getBakStatus() {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  return Reg.value("/BakFiles/BakStatus", 0).toBool();
}

void Preferences::setLatestAction(QString action) {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  Reg.setValue("/BakFiles/BakAction", action);

  setBakStatus(false);
}

void Preferences::appendBakFile(QString action, QString bakfile) {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

  int count = Reg.value("/BakFiles/BakCount", 0).toInt();
  count++;
  Reg.setValue("BakFiles/BakCount", count);
  Reg.setValue("/BakFiles/Action" + QString::number(count - 1), action);
  Reg.setValue("/BakFiles/File" + QString::number(count - 1), bakfile);
}

QStringList Preferences::getBakFilesList() {
  QSettings Reg(privateDir + iniBakFiles, QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  Reg.setIniCodec("utf-8");
#endif

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

  Reg.setValue("/BakFiles/BakCount", 0);
  for (int i = 0; i < fileList.count(); i++) {
    QString str = fileList.at(i);
    appendBakFile(str.split("-===-").at(0), str.split("-===-").at(1));
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
  bool isChanged = false;
  if (ui->chkUIFont->isChecked() != listCheckStatus.at(0)) isChanged = true;

  if (ui->chkDark->isChecked() != listCheckStatus.at(1)) isChanged = true;

  if (ui->sliderFontSize->value() != listCheckStatus.at(2)) isChanged = true;

  if (orgCustomFontText != ui->btnCustomFont->text().trimmed())
    isChanged = true;

  if (isChanged)
    ui->btnReStart->show();
  else
    ui->btnReStart->hide();
}

void Preferences::on_chkAniEffects_clicked(bool checked) { Q_UNUSED(checked); }

void Preferences::on_chkZip_clicked() {
  if (ui->editPassword->text().trimmed() == "" ||
      ui->editValidate->text().trimmed() == "") {
    ui->chkZip->setChecked(false);
  }

  if (ui->editPassword->text().trimmed() !=
      ui->editValidate->text().trimmed()) {
    ui->chkZip->setChecked(false);
    ShowMessage* msg = new ShowMessage(this);
    msg->showMsg("Knot", tr("Password validation error."), 1);
  }
}

void Preferences::on_editPassword_textChanged(const QString& arg1) {
  if (arg1.length() > 0) ui->chkZip->setChecked(false);
}

void Preferences::on_editValidate_textChanged(const QString& arg1) {
  on_editPassword_textChanged(arg1);
}

void Preferences::closeEvent(QCloseEvent* event) {
  Q_UNUSED(event);
  saveOptions();
  mw_one->setEncSyncStatusTip();
}

void Preferences::on_btnShowPassword_pressed() {
  ui->editPassword->setEchoMode(QLineEdit::EchoMode::Normal);
}

void Preferences::on_btnShowPassword_released() {
  ui->editPassword->setEchoMode(QLineEdit::EchoMode::Password);
}

void Preferences::on_btnShowValidate_pressed() {
  ui->editValidate->setEchoMode(QLineEdit::EchoMode::Normal);
}

void Preferences::on_btnShowValidate_released() {
  ui->editValidate->setEchoMode(QLineEdit::EchoMode::Password);
}

void Preferences::on_chkDark_clicked() {}
