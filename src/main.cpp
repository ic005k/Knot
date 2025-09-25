
#include <QApplication>
#include <QDir>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>  // Qt JNI 环境
#include <QJniObject>       // Qt JNI 对象封装
#endif

#include <QElapsedTimer>
#include <QFuture>
#include <QObject>
#include <QOpenGLContext>
#include <QProgressBar>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSplashScreen>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QTranslator>
#include <QWidget>

#include "MainWindow.h"
#include "SplashTimer.h"
#include "lib/cppjieba/Jieba.hpp"
#include "lib/quazip/quazip.h"
#include "lib/quazip/quazipfile.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

std::unique_ptr<cppjieba::Jieba> jieba;

extern void RegJni(const char* myClassName);
extern void RegJni15(const char* myClassName);
extern void RegJni16(const char* myClassName);
extern void RegJni17(const char* myClassName);

void loadTheme(bool isDark);
void loadLocal();

QPalette createDarkPalette();
QPalette createLightPalette();

#ifdef Q_OS_ANDROID
QString getPrivateKnotPath();
QString getPublicKnotDataPath();
int getAndroidSdkVersion();
void migrateOldDataIfNeeded();
#endif

void initAndroidGPU();

QString strJBDict1 = "";
QString strJBDict2 = "";
QString strJBDict3 = "";
QString strJBDict4 = "";
QString strJBDict5 = "";

const QString uniqueKey = "MyUniqueApplicationKey_12345";

#define Cross_Origin

int main(int argc, char* argv[]) {
  QElapsedTimer totalTimer;
  totalTimer.start();

  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

  QApplication::setStyle(QStyleFactory::create("Fusion"));

  // 禁用文本选择（针对所有的可输入的编辑框）
  qputenv("QT_QPA_NO_TEXT_HANDLES", "1");

#ifdef Q_OS_WIN
  isWindows = true;
#elif defined(Q_OS_MACOS)
  isMacOS = true;
#elif defined(Q_OS_IOS)
  isIOS = true;
#elif defined(Q_OS_ANDROID)
  isAndroid = true;
#elif defined(Q_OS_LINUX)
  isLinux = true;
#endif

  QApplication app(argc, argv);

  loadLocal();

#ifndef Q_OS_ANDROID
  QSharedMemory sharedMemory(uniqueKey);
  if (!sharedMemory.create(1)) {
    QMessageBox::information(nullptr, "Knot",
                             QObject::tr("The application is already running!"),
                             QMessageBox::Ok);

    return 0;
  }
#endif

  // showSplash();
  SplashTimer splash(isAndroid, 320, 100);
  splash.show();

  // 设置应用程序标识（为QML里面使用Settings做准备）
  app.setOrganizationName("KnotCompany");
  app.setOrganizationDomain("knotcompany.com");
  app.setApplicationName("Knot");

  QDir dir;
  QString path;
  path = dir.currentPath();
  qDebug() << "Path:" << path;

  int defaultFontSize;

#ifdef Q_OS_ANDROID
  defaultFontSize = 17;

  RegJni("com/x/MyService");
  RegJni(ANDROID_MAIN_ACTIVITY);
  RegJni15(ANDROID_MAIN_ACTIVITY);
  RegJni("com/x/ClockActivity");
  RegJni("com/x/ShareReceiveActivity");
  RegJni("com/x/NoteEditor");
  RegJni16("com/x/NoteEditor");
  RegJni("com/x/MDActivity");
  RegJni("com/x/NewTodo");
  RegJni("com/x/NewNote");
  RegJni("com/x/AddRecord");
  RegJni("com/x/ContinueReading");
  RegJni("com/x/Desk_Exercise");
  RegJni("com/x/FilePicker");
  RegJni("com/xhh/pdfui/PDFActivity");
  RegJni("com/x/DefaultOpen");
  RegJni("com/x/DateTimePicker");
  RegJni17("com/x/WebViewActivity");

  iniDir = "/storage/emulated/0/KnotData/";
  privateDir = "/storage/emulated/0/.Knot/";

  // 使用app的专属沙盒存储数据
  // iniDir = getPublicKnotDataPath();
  // privateDir = getPrivateKnotPath();

#else
  defaultFontSize = QApplication::font().pointSize();

  isAndroid = false;
  iniDir = QDir::homePath() + "/" + appName + "Data/";
  privateDir = QDir::homePath() + "/." + appName + "/";

#endif

  QDir p_dir;
  p_dir.mkpath(privateDir);
  p_dir.mkpath(iniDir);
  QString bak_dir = iniDir;
  bak_dir = bak_dir.replace("KnotData", "KnotBak");
  p_dir.mkpath(bak_dir);
  bakfileDir = bak_dir;
  p_dir.mkpath(privateDir + "KnotData");
  p_dir.mkpath(privateDir + "KnotData/memo");
  p_dir.mkpath(privateDir + "KnotData/memo/images");
  p_dir.mkpath(privateDir + "KnotData/memo/gps");

  strJBDict1 = privateDir + "dict/jieba.dict.utf8";
  strJBDict2 = privateDir + "dict/hmm_model.utf8";
  strJBDict3 = privateDir + "dict/user.dict.utf8";
  strJBDict4 = privateDir + "dict/stop_words.utf8";
  strJBDict5 = privateDir + "dict/idf.utf8";

  if (!QFile::exists(strJBDict1) || !QFile::exists(strJBDict2) ||
      !QFile::exists(strJBDict3)) {
    QString resFile = ":/res/jbdict/dict.zip";
    deleteDirfile(privateDir + "dict");

    m_Method->decompressWithPassword(resFile, privateDir, "");
  }

  // 初始化结巴分词
  jieba = std::make_unique<cppjieba::Jieba>(strJBDict1.toStdString(),
                                            strJBDict2.toStdString(),
                                            strJBDict3.toStdString());

  iniFile = iniDir + appName + ".ini";

  QDir dir0;
  dir0.mkpath(iniDir);

  iniPreferences =
      new QSettings(privateDir + "options.ini", QSettings::IniFormat, NULL);

  fontSize =
      iniPreferences->value("/Options/FontSize", defaultFontSize).toInt();
  bool isOverUIFont =
      iniPreferences->value("/Options/chkUIFont", false).toBool();
  QString customFontPath =
      iniPreferences->value("/Options/CustomFont").toString();

  // Qt>=6.5.0
  isDark =
      QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;

#ifdef Q_OS_WIN
  defaultFontFamily = "Microsoft YaHei UI";

#endif

#ifdef Q_OS_ANDROID
  defaultFontFamily = "";  // "DroidSansFallback";

#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
  defaultFontFamily = "Noto Sans CJK SC";  //"Sans";

#endif

  if (isOverUIFont) {
    if (QFile(customFontPath).exists()) {
      int loadedFontID = QFontDatabase::addApplicationFont(customFontPath);
      QStringList loadedFontFamilies =
          QFontDatabase::applicationFontFamilies(loadedFontID);
      if (!loadedFontFamilies.empty()) {
        customFontFamily = loadedFontFamilies.at(0);
      }
    }
  } else
    customFontFamily = defaultFontFamily;

  // Set Font
  QFont m_font;
  if (isOverUIFont) {
    if (customFontFamily.length() > 0) {
      m_font.setFamily(customFontFamily);
    }
  } else {
    if (defaultFontFamily.length() > 0) {
      m_font.setFamily(defaultFontFamily);
    }
  }

  m_font.setPointSize(fontSize);
  app.setFont(m_font);

#ifdef Q_OS_ANDROID

#else

#endif

  MainWindow w;
  QObject::connect(QGuiApplication::styleHints(),
                   &QStyleHints::colorSchemeChanged, &w,
                   [](Qt::ColorScheme scheme) {
                     isDark = (scheme == Qt::ColorScheme::Dark);

                     loadTheme(isDark);
                   });
  loadTheme(isDark);

  w.show();

#ifdef Q_OS_ANDROID
  // 通知 Java 层 Qt main() 已完成
  QJniObject::callStaticMethod<void>(ANDROID_MAIN_ACTIVITY, "setQtMainEnd",
                                     "(Z)V", true);
#endif

  QTimer::singleShot(0, &app, [&]() {
    qint64 totalElapsedMs = totalTimer.elapsed();
    double totalElapsedSec = totalElapsedMs / 1000.0;

    strStartTotalTime = QString::number(totalElapsedSec, 'f', 2);

    qDebug() << "整体启动总耗时：" << strStartTotalTime << "秒";
  });

  splash.close();

  return app.exec();
}

void loadTheme(bool isDark) {
  isInitThemeEnd = false;
  // 设置调色板
  if (isDark) {
    qApp->setPalette(createDarkPalette());
  } else {
    qApp->setPalette(createLightPalette());
  }

  QString themePath =
      isDark ? ":/res/theme/darkstyle.qss" : ":/res/theme/lightstyle.qss";

  QFile f(themePath);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString styleSheet = QTextStream(&f).readAll();

    // 动态添加颜色方案标识
    styleSheet.prepend(
        QString("[color-scheme=\"%1\"] ").arg(isDark ? "dark" : "light"));

    qApp->setStyleSheet(styleSheet);
    mw_one->m_MainHelper->init_Theme();

    // 强制窗口重绘
    QEvent updateEvent(QEvent::UpdateRequest);
    QApplication::sendEvent(mw_one, &updateEvent);
  }

  QFont font = qApp->font();
  font.setPointSize(fontSize);
  qApp->setFont(font);

  // 强制所有窗口和部件刷新字体
  foreach (QWidget* widget, qApp->allWidgets()) {
    if (widget != mui->btnMenu && widget != mui->btnAdd &&
        widget != mui->btnDel && widget != mui->btnSync &&
        widget != mui->btnSelTab && widget != mui->btnReader &&
        widget != mui->btnTodo && widget != mui->btnNotes &&
        widget != mui->btnSteps && widget != mui->lblMonthTotal &&
        widget != mui->lblYearTotal && widget != mui->btn0 &&
        widget != mui->btn1 && widget != mui->btn2 && widget != mui->btn3 &&
        widget != mui->btn4 && widget != mui->btn5 && widget != mui->btn6 &&
        widget != mui->btn7 && widget != mui->btn8 && widget != mui->btn9 &&
        widget != mui->btnDot && widget != mui->btnDel_Number &&
        widget != mui->lblMonthSum && widget != mui->lblTime &&
        widget != mui->lblGpsInfo && widget != mui->lblTotalDistance &&
        widget != mui->lblCurrentDistance && widget != mui->lblAverageSpeed &&
        widget != mui->lblRunTime && widget != mw_one->m_Steps->m_speedometer &&
        widget != mw_one->m_MainHelper->sliderButton &&
        widget != mui->lblGpsDateTime && widget != mui->btnPages &&
        widget != mui->lblBookName && widget != mui->lblShowLineSn &&
        widget != mw_one->m_PageIndicator->ui->lblPageNumber) {
      widget->setFont(qApp->font());
      font.setBold(true);
      mui->lblViewCate1->setFont(font);
      mui->lblTitleEditRecord->setFont(font);
      widget->updateGeometry();
      widget->repaint();
    }
  }

  mw_one->m_Reader->initInfoShowFont();

  isInitThemeEnd = true;

  if (isNeedExecDeskShortcut) {
    isNeedExecDeskShortcut = false;
    QTimer::singleShot(1000, nullptr, []() { mw_one->execDeskShortcut(); });
  }
}

QPalette createDarkPalette() {
  QPalette darkPalette;

  // 基础颜色设置
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));

  // 统一选中项颜色 (关键)
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));  // 统一蓝色
  darkPalette.setColor(QPalette::HighlightedText, Qt::white);

  // 其他必要颜色配置
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::ButtonText, Qt::white);

  return darkPalette;
}

// 创建亮色调色板
QPalette createLightPalette() {
  QPalette lightPalette;

  // 使用系统默认的亮色调色板作为基础
  lightPalette = QApplication::style()->standardPalette();

  // 自定义亮色主题颜色（可选）
  lightPalette.setColor(QPalette::Highlight,
                        QColor(42, 130, 218));  // 保持与暗模式相同的高亮色
  lightPalette.setColor(QPalette::HighlightedText, Qt::white);

  // 确保禁用状态颜色合适
  lightPalette.setColor(QPalette::Disabled, QPalette::Text,
                        QColor(150, 150, 150));
  lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                        QColor(150, 150, 150));

  return lightPalette;
}

void loadLocal() {
  static QTranslator translator0;
  static QTranslator translator1;
  static QTranslator translator2;
  static QTranslator translator3;
  QLocale locale;

  if (locale.language() == QLocale::English) {
    isZH_CN = false;

  } else if (locale.language() == QLocale::Chinese) {
    bool tr = false;
    tr = translator0.load(":/src/cn.qm");
    if (tr) {
      qApp->installTranslator(&translator0);
      isZH_CN = true;
    }

    bool tr1 = false;
    tr1 = translator1.load(":/res/tr/qt_zh_CN.qm");
    if (tr1) {
      qApp->installTranslator(&translator1);
      isZH_CN = true;
    }

    bool tr2 = false;
    tr2 = translator2.load(":/res/tr/qtbase_zh_CN.qm");
    if (tr2) {
      qApp->installTranslator(&translator2);
      isZH_CN = true;
    }

    bool tr3 = false;
    tr3 = translator3.load(":/res/tr/qtlocation_zh_CN.qm");
    if (tr3) {
      qApp->installTranslator(&translator3);
      isZH_CN = true;
    }
  }
}

bool unzipToDir(const QString& zipPath, const QString& destDir) {
  QuaZip zip(zipPath);
  if (!zip.open(QuaZip::mdUnzip)) {
    qDebug() << "无法打开 ZIP 文件：" << zipPath;
    return false;
  }

  QDir dir(destDir);
  if (!dir.exists() && !dir.mkpath(".")) {
    qDebug() << "无法创建目标目录：" << destDir;
    return false;
  }

  // 遍历 ZIP 内所有文件
  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
    QuaZipFile file(&zip);
    if (!file.open(QIODevice::ReadOnly)) {
      qDebug() << "无法打开文件：" << zip.getCurrentFileName();
      continue;
    }

    QString filePath = destDir + "/" + zip.getCurrentFileName();
    QFileInfo fileInfo(filePath);

    // 创建子目录（如果需要）
    if (!fileInfo.dir().exists() && !fileInfo.dir().mkpath(".")) {
      qDebug() << "无法创建子目录：" << fileInfo.dir().path();
      continue;
    }

    // 写入文件内容
    QFile outputFile(filePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
      qDebug() << "无法写入文件：" << filePath;
      continue;
    }

    outputFile.write(file.readAll());
    outputFile.close();
    file.close();
  }

  zip.close();
  return true;
}

#ifdef Q_OS_ANDROID

QString getPrivateKnotPath() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
      "/.Knot/";
  QDir().mkpath(path);
  return path;
}

QString getPublicKnotDataPath() {
  QString path;
#if defined(Q_OS_ANDROID)
  if (getAndroidSdkVersion() >= 29) {
    // Android 10+：使用系统文档目录
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
           "/KnotData/";
  } else {
    // Android 9-：直接访问外部存储（需权限）
    path = "/storage/emulated/0/KnotData/";
  }
#else
  path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
         "/KnotData/";
#endif
  QDir().mkpath(path);
  return path;
}

int getAndroidSdkVersion() {
  // 正确方式：直接获取静态 int 字段
  return QJniObject::getStaticField<jint>("android/os/Build$VERSION",  // 类名
                                          "SDK_INT"                    // 字段名
  );
}

// 旧版本已使用硬编码路径，升级时需将数据迁移到新路径
void migrateOldDataIfNeeded() {
#if defined(Q_OS_ANDROID)
  QString oldPublicPath = "/storage/emulated/0/KnotData/";
  QString newPublicPath = getPublicKnotDataPath();

  QDir oldDir(oldPublicPath);
  if (oldDir.exists() && (oldPublicPath != newPublicPath)) {
    // 复制旧数据到新路径
    // copyDirectory(oldPublicPath, newPublicPath);
    // 删除旧目录（谨慎操作！）
    oldDir.removeRecursively();
  }
#endif
}

#endif

void initAndroidGPU() {
  // 核心配置：启用基础硬件加速 + 兼容模式
  qputenv("QT_QUICK_BACKEND", "opengl");  // 强制OpenGL ES后端（最佳兼容）
  qputenv("QSG_RENDER_LOOP", "basic");    // 使用基本渲染循环（避免线程问题）
  qputenv("QT_OPENGL_ES_ANGLE", "0");     // 禁用ANGLE中间层（直接使用系统驱动）
  QCoreApplication::setAttribute(
      Qt::AA_ShareOpenGLContexts);  // 共享OpenGL上下文

  // 设置保守的OpenGL配置
  QSurfaceFormat format;
  format.setDepthBufferSize(16);                       // 深度缓冲（最低需求）
  format.setStencilBufferSize(8);                      // 模板缓冲（最低需求）
  format.setRenderableType(QSurfaceFormat::OpenGLES);  // 强制GLES模式
  format.setVersion(2, 0);                       // 仅使用OpenGL ES 2.0核心功能
  format.setProfile(QSurfaceFormat::NoProfile);  // 不使用核心模式
  QSurfaceFormat::setDefaultFormat(format);
}
