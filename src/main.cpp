
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
#include "native_msg_host.h"
#include "src/Comm/loglogger.h"
#include "src/defines.h"
#include "ui_MainWindow.h"

std::unique_ptr<cppjieba::Jieba> jieba;

extern void RegJni(const char* myClassName);
extern void RegJni15(const char* myClassName);
extern void RegJni16(const char* myClassName);
extern void RegJni17(const char* myClassName);
extern void RegJni18(const char* myClassName);
extern void RegJni19(const char* myClassName);
extern void RegJni20(const char* myClassName);

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
int clearLockFiles(const QString& iniDir);

QString strJBDict1 = "";
QString strJBDict2 = "";
QString strJBDict3 = "";
QString strJBDict4 = "";
QString strJBDict5 = "";

#define Cross_Origin

int main(int argc, char* argv[]) {
  QElapsedTimer totalTimer;
  totalTimer.start();

  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

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

  initAndroidGPU();

#elif defined(Q_OS_LINUX)
  isLinux = true;
#endif

  QApplication app(argc, argv);

  QApplication::setStyle(QStyleFactory::create("Fusion"));

  loadLocal();

#ifdef Q_OS_ANDROID

#else
  sharedMemory.setKey(uniqueKey);
  if (!sharedMemory.create(1)) {
    QMessageBox::information(nullptr, "Knot",
                             QObject::tr("The application is already running!"),
                             QMessageBox::Ok);

    return 0;
  }

  // ========= 浏览器插件 启动原生通信监听子线程 =========
  // NativeMsgThread* msgThread = new NativeMsgThread();
  // QObject::connect(msgThread, &QThread::finished, msgThread,
  //                 &QThread::deleteLater);
  // msgThread->start();
#endif

  // ========== 闪屏改为堆对象（new创建），避免栈对象生命周期问题 =====
  SplashTimer* splash = new SplashTimer(isAndroid, 200, 90);
  splash->show();

  // 设置应用程序标识（为QML里面使用Settings做准备）
  app.setOrganizationName("KnotCompany");
  app.setOrganizationDomain("knotcompany.com");
  app.setApplicationName("Knot");
  app.setDesktopFileName("default");

  QDir dir;
  QString path;
  path = dir.currentPath();
  qDebug() << "Path:" << path;

#ifdef Q_OS_ANDROID
  defaultFontSize = 12;

  RegJni("com/x/MyService");
  RegJni20("com/x/MyService");
  RegJni(ANDROID_MAIN_ACTIVITY);
  RegJni15(ANDROID_MAIN_ACTIVITY);
  RegJni18(ANDROID_MAIN_ACTIVITY);
  RegJni19(ANDROID_MAIN_ACTIVITY);
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
  p_dir.mkpath(iniDir + "memo/readnote");
  p_dir.mkpath(iniDir + "bookini");
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

  QDir dir0;
  dir0.mkpath(iniDir);

  AppLogger::instance().initLogger(appName);

  qInfo() << "****************************************************************"
             "********************";
  clearLockFiles(iniDir);
  clearLockFiles(privateDir);

  iniPreferences =
      new QSettings(privateDir + "options.ini", QSettings::IniFormat, NULL);

  fontScale = m_Method->getSystemFontScale();
  int m_fontSize =
      iniPreferences->value("/Options/FontSize", defaultFontSize).toInt();
  fontSize = m_fontSize * fontScale;
  bool isOverUIFont =
      iniPreferences->value("/Options/chkUIFont", false).toBool();
  QString customFontPath =
      iniPreferences->value("/Options/CustomFont").toString();

#ifdef Q_OS_WIN
  defaultFontFamily = "Microsoft YaHei UI";

#endif

#ifdef Q_OS_ANDROID
  defaultFontFamily = "sans-serif";

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

  MainWindow w;

  // 1. 先初始化当前主题状态（替代原有的isDark直接赋值）
  g_currentIsDark =
      (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
  isDark = g_currentIsDark;  // 兼容原有代码对isDark的依赖

  // 2. 重构信号绑定逻辑：增加主题状态校验
  QObject::connect(QGuiApplication::styleHints(),
                   &QStyleHints::colorSchemeChanged, &w,
                   [](Qt::ColorScheme scheme) {
                     bool newIsDark = (scheme == Qt::ColorScheme::Dark);
                     // 核心优化：只有主题真的变化时，才执行加载逻辑
                     if (newIsDark != g_currentIsDark) {
                       isDark = newIsDark;           // 同步原有全局isDark
                       g_currentIsDark = newIsDark;  // 更新当前状态
                       loadTheme(isDark);            // 仅在主题变化时执行
                     }
                   });

  // 3. 初始加载（确保MainWindow初始化完成后执行，避免空指针）
  loadTheme(g_currentIsDark);

  w.show();

#ifdef Q_OS_ANDROID
  // 通知 Java 层 Qt main() 已完成
  QJniObject::callStaticMethod<void>(ANDROID_MAIN_ACTIVITY, "setQtMainEnd",
                                     "(Z)V", true);
#endif

  // ========== 停止闪屏的逻辑放到事件循环中，异步执行 ==========
  // QTimer::singleShot(0)：确保在事件循环启动后执行
  QTimer::singleShot(0, &app, [&]() {
    qint64 totalElapsedMs = totalTimer.elapsed();
    double totalElapsedSec = totalElapsedMs / 1000.0;
    strStartTotalTime = QString::number(totalElapsedSec, 'f', 2);
    qDebug() << "整体启动总耗时：" << strStartTotalTime << "秒";

    // ========== 堆对象的安全停止+释放 ==========
    if (splash) {
      splash->stopAnimation();  // 停止动画
      splash->close();          // 关闭窗口
      splash->deleteLater();    // 异步释放堆对象（正确用法）
      splash = nullptr;         // 置空，避免野指针
    }
  });

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
      isDark ? ":/res/theme/MaterialDark.qss" : ":/res/theme/MaterialLight.qss";

  QFile f(themePath);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString styleSheet = QTextStream(&f).readAll();
    styleSheet.prepend(
        QString("[color-scheme=\"%1\"] ").arg(isDark ? "dark" : "light"));
    qApp->setStyleSheet(styleSheet);

    // 强制窗口重绘
    if (mw_one) {
      mw_one->init_Theme();
      QEvent updateEvent(QEvent::UpdateRequest);
      QApplication::sendEvent(mw_one, &updateEvent);
    }
  }

  // 字体大小
  QFont font = qApp->font();
  font.setPointSize(fontSize);
  qApp->setFont(font);

  mui->qwTodo->rootContext()->setContextProperty("FontSize", fontSize);
  mui->qwRecycle->rootContext()->setContextProperty("FontSize", fontSize);

  // 遍历控件刷新字体（仅字体大小变化时执行）
  if (qApp) {
    foreach (QWidget* widget, qApp->allWidgets()) {
      if (widget != mui->btnMenu && widget != mui->btnHome &&
          widget != mui->btnAdd && widget != mui->btnDel &&
          widget != mui->btnSync && widget != mui->btnFind &&
          widget != mui->btnSelTab && widget != mui->btnReader &&
          widget != mui->btnTodo && widget != mui->btnNotes &&
          widget != mui->btnSteps && widget != mui->lblMonthTotal &&
          widget != mui->lblYearTotal && widget != mui->btn0 &&
          widget != mui->editAmount && widget != mui->btn1 &&
          widget != mui->btn2 && widget != mui->btn3 && widget != mui->btn4 &&
          widget != mui->btn5 && widget != mui->btn6 && widget != mui->btn7 &&
          widget != mui->btn8 && widget != mui->btn9 && widget != mui->btnDot &&
          widget != mui->btnDel_Number && widget != mui->lblMonthSum &&
          widget != mui->lblTime && widget != mui->lblGpsInfo &&
          widget != m_Steps->m_speedometer &&
          widget != mw_one->m_MainHelper->sliderButton &&
          widget != mui->lblGpsDateTime && widget != mui->btnPages &&
          widget != mui->lblBookName && widget != mui->lblShowLineSn &&
          widget != mui->lblNoteBook && widget != mui->lblNoteList &&
          widget != mui->lblSyncNote) {
        widget->setFont(qApp->font());

        font.setBold(true);
        mui->lblViewCate1->setFont(font);
        mui->lblTabTitle->setFont(font);
        mui->lblTitleEditRecord->setFont(font);
        if (mui && mui->lblSyncNote) {
          QFont mFont = font;
          if (!isAndroid)
            mFont.setPointSize(9);
          else
            mFont.setPointSize(12);
          mui->lblSyncNote->setFont(mFont);
        }
        widget->updateGeometry();
        widget->repaint();
      }
    }
  }

  // 空指针校验：避免崩溃
  if (mw_one) {
    if (m_Reader) m_Reader->initInfoShowFont();
    if (mw_one->m_Todo) {
      mw_one->m_Todo->refreshTableListsFromFile();
      mw_one->m_Todo->refreshAlarm();
    }
  }

  // 消除潜在的补全列表窗口
  mui->editCategory->setText("");

  isInitThemeEnd = true;

  if (isNeedExecDeskShortcut) {
    isNeedExecDeskShortcut = false;
    QTimer::singleShot(1000, nullptr, []() {
      if (mw_one) mw_one->execDeskShortcut();  // 空指针校验
    });
  }
}

QPalette createDarkPalette() {
  QPalette darkPalette;

  // 基础颜色设置
  darkPalette.setColor(QPalette::Window, QColor(25, 35, 45));  // "#19232D"
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

  // 基础颜色设置
  lightPalette.setColor(QPalette::Window, QColor(243, 243, 243));

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

/*void initAndroidGPU() {
  // ====== 阶段1：注入环境变量（最先执行，兜底）======
  qputenv("QSG_RHI_BACKEND", "opengl");
  qputenv("QT_QUICK_BACKEND", "opengl");
  qputenv("QSG_INFO", "1");  // 打印RHI日志，验证是否OpenGL

  // ====== 阶段2：全局锁定所有Qt
  // Quick渲染后端（含QQuickWidget内部离屏窗口）======
  // 全局强制所有Quick渲染使用OpenGL ES（Android）
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  // ====== 阶段3：全局Surface强制仅OpenGLES，拒绝Vulkan EGL Surface ======
  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGLES);
  fmt.setVersion(3, 2);  // 主流手机兼容ES3.2
  fmt.setDefaultFormat(fmt);
}*/

void initAndroidGPU() {
  qputenv("QSG_RHI_BACKEND", "opengl");
  qputenv("QT_QUICK_BACKEND", "opengl");
  qputenv("QSG_INFO", "1");
  // 新增：全局关闭Quick持久离屏图形缓存，替代不存在的静态函数
  qputenv("QSG_NO_PERSISTENT_GRAPHICS_CACHE", "1");
  qputenv("QT_RHI_NO_OFFSCREEN_BLIT", "1");
  qputenv("QT_QPA_GL_NO_PBO", "1");

  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  QSurfaceFormat fmt;
  fmt.setRenderableType(QSurfaceFormat::OpenGLES);
  fmt.setVersion(3, 2);
  fmt.setDefaultFormat(fmt);
}

int clearLockFiles(const QString& iniDir) {
  // 1. 初始化目录对象并校验有效性
  QDir dir(iniDir);
  if (!dir.exists()) {
    qWarning() << "清除.lock文件失败：目录不存在 ->" << iniDir;
    return 0;  // 目录不存在，无需清理
  }

  // 2. 筛选目录中所有 .lock 后缀的文件（仅文件，不包含子目录）
  // 过滤规则：后缀为 .lock（不区分大小写，如 .LOCK 也会被匹配）
  QStringList nameFilters;
  nameFilters << "*.lock" << "*.LOCK";  // 覆盖大小写情况
  QFileInfoList fileInfos = dir.entryInfoList(nameFilters,
                                              QDir::Files,  // 只处理文件
                                              QDir::NoSort  // 无需排序
  );

  if (fileInfos.isEmpty()) {
    qInfo() << "目录中无.lock文件 ->" << iniDir;
    return 0;
  }

  // 3. 遍历并删除所有匹配的.lock文件
  int deletedCount = 0;
  // 使用const迭代器遍历，避免detach
  for (QFileInfoList::const_iterator it = fileInfos.constBegin();
       it != fileInfos.constEnd(); ++it) {
    const QFileInfo& fileInfo = *it;

    QFile file(fileInfo.absoluteFilePath());
    if (file.remove()) {
      deletedCount++;
      qDebug() << "已删除.lock文件 ->" << fileInfo.absoluteFilePath();
    } else {
      qWarning() << "删除失败 ->" << fileInfo.absoluteFilePath();
    }
  }

  qInfo() << "清除完成：目录" << iniDir << "共处理" << fileInfos.size()
          << "个.lock文件，成功删除" << deletedCount << "个";
  return deletedCount;
}
