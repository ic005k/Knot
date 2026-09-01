
#include <QApplication>
#include <QDir>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QJniObject>
#endif

#include <ggml-cpu.h>

#include <QElapsedTimer>
#include <QFuture>
#include <QObject>
#include <QOpenGLContext>
#include <QProgressBar>
#include <QSGRendererInterface>
#include <QSplashScreen>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QTranslator>
#include <QWidget>

#include "MainWindow.h"
#include "NativeMsgThread.h"
#include "SplashTimer.h"
#include "defines.h"
#include "lib/llama.cpp/ggml/include/ggml-backend.h"
#include "lib/llama.cpp/include/llama.h"
#include "lib/quazip/quazip.h"
#include "lib/quazip/quazipfile.h"
#include "src/AI/GlobalAI.h"
#include "src/Comm/AppLogger.h"

extern void RegJni(const char* myClassName);
extern void RegJni15(const char* myClassName);
extern void RegJni16(const char* myClassName);
extern void RegJni17(const char* myClassName);
extern void RegJni18(const char* myClassName);
extern void RegJni19(const char* myClassName);
extern void RegJni20(const char* myClassName);
extern void RegJniAiLink(const char* myClassName);
extern void RegJniParsePreview(const char* myClassName);
extern void RegJniSendQuestion(const char* myClassName);
extern void RegJniPublicJavaCallCpp(const char* myClassName);

extern void releaseGlobalAiEngine();

QString defaultModel = "multilingual-e5-small-fp32.gguf";

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

#define Cross_Origin

/////////////////////////////////////////////////////////////

#ifdef _MSC_VER
// 强制让链接器把 ggml_backend_cpu_reg 拉进来
// extern "C" 所以 x64 下名字就是 ggml_backend_cpu_reg，不需要前导下划线
#pragma comment(linker, "/INCLUDE:ggml_backend_cpu_reg")

// 防止 BERT 被 OPT:REF / LTCG 裁剪
#pragma comment(linker, "/INCLUDE:llama_model_load_from_file")
#pragma comment(linker, "/INCLUDE:llama_encode")
#pragma comment(linker, "/INCLUDE:llama_get_embeddings_seq")
#endif

// 【全平台统一兜底】永久持有CPU后端注册句柄，阻止所有编译器优化裁剪后端静态符号
static ggml_backend_reg_t g_global_cpu_backend = nullptr;

// 在全局作用域，强制让编译器认为这些符号被"使用"
static volatile void* g_force_link_cpu_backend[] = {
    (void*)&ggml_backend_cpu_reg,
    (void*)&ggml_backend_cpu_init,
};

int init_main_ai();

///////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
  ///////////////////////////////////////////////////////////////////////
  // 防止整个数组被优化
  (void)g_force_link_cpu_backend;

  int value = init_main_ai();
  qDebug() << "init_main_ai=" << value;
  if (value == 1) return 1;

  ///////////////////////////////////////////////////////////////////////////

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

  // ========= 浏览器插件 启动原生通信监听子线程 =====================
  // NativeMsgThread* msgThread = new NativeMsgThread();
  // QObject::connect(msgThread, &QThread::finished, msgThread,
  //                 &QThread::deleteLater);
  // msgThread->start();
#endif

  // ============================ 闪屏============================
  // 先初始化当前主题状态（替代原有的isDark直接赋值）
  g_currentIsDark =
      (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
  isDark = g_currentIsDark;
  SplashTimer* splash = new SplashTimer(isAndroid, isDark, 200, 90);
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
  RegJniAiLink("com/x/NoteEditor");
  RegJniParsePreview("com/x/NoteEditor");
  RegJniSendQuestion("com/x/NoteEditor");
  RegJni("com/x/MDActivity");
  RegJni("com/x/NewTodo");
  RegJni("com/x/NewNote");
  RegJni("com/x/AddRecord");
  RegJni("com/x/ContinueReading");
  RegJni("com/x/Desk_Exercise");
  RegJni("com/x/FilePicker");
  RegJni("com/xhh/pdfui/PDFActivity");
  RegJni("com/x/artifex/mupdf/mini/DocumentActivity");
  RegJni("com/x/DefaultOpen");
  RegJni("com/x/DateTimePicker");
  RegJni17("com/x/WebViewActivity");
  RegJniPublicJavaCallCpp("com/x/ReadListActivity");
  RegJniPublicJavaCallCpp("com/x/AddEventRecord");
  RegJniPublicJavaCallCpp("com/x/MainEntrance");
  RegJniPublicJavaCallCpp("com/x/FilePicker");
  RegJniPublicJavaCallCpp("com/x/TodoActivity");
  RegJniPublicJavaCallCpp("com/x/TodoCardAdapter");
  RegJniPublicJavaCallCpp("com/x/TodoRecycleActivity");
  RegJniPublicJavaCallCpp("com/x/TodoAlarmActivity");
  RegJniPublicJavaCallCpp("com/x/MyEventActivity");
  RegJniPublicJavaCallCpp("com/x/NoteActivity");

  iniDir = "/storage/emulated/0/KnotData/";
  privateDir = "/storage/emulated/0/.Knot/";

#else
  defaultFontSize = QApplication::font().pointSize();

  isAndroid = false;
  iniDir = QDir::homePath() + "/KnotData/";
  privateDir = QDir::homePath() + "/.Knot/";

#endif

  QDir p_dir;
  p_dir.mkpath(privateDir);
  p_dir.mkpath(iniDir);
  p_dir.mkpath(iniDir + "memo/readnote");
  p_dir.mkpath(iniDir + "memo/images");
  p_dir.mkpath(iniDir + "memo/gps");
  p_dir.mkpath(iniDir + "bookini");
  QString bak_dir = iniDir;
  bak_dir = bak_dir.replace("KnotData", "KnotBak");
  p_dir.mkpath(bak_dir);
  bakfileDir = bak_dir;
  p_dir.mkpath(privateDir + "KnotData");
  p_dir.mkpath(privateDir + "KnotData/memo");
  p_dir.mkpath(privateDir + "KnotData/memo/images");
  p_dir.mkpath(privateDir + "KnotData/memo/gps");

  modelFullPath = privateDir + "model/";
  modelDataBasePath = modelFullPath + "database/";
  p_dir.mkpath(modelFullPath);
  p_dir.mkpath(modelDataBasePath);

  QDir dir0;
  dir0.mkpath(iniDir);

  // 开始输出日志
  AppLogger::instance().initLogger(appName);

  qInfo() << "The app has started to launch...";
  clearLockFiles(iniDir);
  clearLockFiles(privateDir);

  iniPreferences =
      new QSettings(privateDir + "options.ini", QSettings::IniFormat, NULL);

  modelFileName =
      iniPreferences->value("/Options/localmodel", defaultModel).toString();
  modelFingerprint = computeModelFingerprint(modelFullPath + modelFileName);

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

  isLocalAIModel = false;
#ifdef VECTOR_SEARCH
  // 执行ORT引擎、向量库完整初始化
  isLocalAIModel = initGlobalAiEngine();
  if (!isLocalAIModel) {
    qDebug() << "向量模型文件缺失...";
  } else {
    qDebug() << "向量模型初始化成功...";
  }
#endif

  MainWindow w;

  // 重构信号绑定逻辑：增加主题状态校验
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

  // 初始加载（确保MainWindow初始化完成后执行，避免空指针）
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

  int ret = app.exec();

  QThread* workerThread = WeatherFetcher::instance()->workerThread();

  // ✅ 1. 阻塞等待子线程执行完 shutdown
  QMetaObject::invokeMethod(WeatherFetcher::instance(), "shutdown",
                            Qt::BlockingQueuedConnection);

  // ✅ 2. 子线程已经清理完毕，现在 quit 是安全的
  workerThread->quit();
  workerThread->wait();
  workerThread->deleteLater();

#ifdef VECTOR_SEARCH
  // 程序退出统一释放llama、ggml全局资源
  releaseGlobalAiEngine();
#endif

  return ret;
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
      // mw_one->init_Theme();
      QEvent updateEvent(QEvent::UpdateRequest);
      QApplication::sendEvent(mw_one, &updateEvent);
    }
  }

  // 字体大小
  QFont font = qApp->font();
  font.setPointSize(fontSize);
  qApp->setFont(font);

  // 遍历控件刷新字体（仅字体大小变化时执行）
  if (qApp) {
    SliderButton* m_sliderButton;
    if (mw_one) {
      m_sliderButton = mw_one->m_MainHelper->sliderButton;
    } else
      return;

    foreach (QWidget* widget, qApp->allWidgets()) {
      if (widget != mw_one->ui->btnMenu && widget != mw_one->ui->btnHome &&
          widget != mw_one->ui->btnAdd && widget != mw_one->ui->btnDel &&
          widget != mw_one->ui->btnSync && widget != mw_one->ui->btnFind &&
          widget != mw_one->ui->btnSelTab && widget != mw_one->ui->btnReader &&
          widget != mw_one->ui->btnTodo && widget != mw_one->ui->btnNotes &&
          widget != mw_one->ui->btnSteps && widget != mw_one->ui->btn0 &&
          widget != mw_one->ui->editAmount && widget != mw_one->ui->btn1 &&
          widget != mw_one->ui->btn2 && widget != mw_one->ui->btn3 &&
          widget != mw_one->ui->btn4 && widget != mw_one->ui->btn5 &&
          widget != mw_one->ui->btn6 && widget != mw_one->ui->btn7 &&
          widget != mw_one->ui->btn8 && widget != mw_one->ui->btn9 &&
          widget != mw_one->ui->btnDot && widget != mw_one->ui->btnDel_Number &&
          widget != mw_one->ui->lblMonthSum && widget != mw_one->ui->lblTime &&
          widget != mw_one->ui->lblGpsInfo &&
          widget != m_Steps->m_speedometer && widget != m_sliderButton &&
          widget != mw_one->ui->lblGpsDateTime &&
          widget != mw_one->ui->lblSyncNote &&
          widget != mw_one->ui->lblVectorStatus) {
        widget->setFont(qApp->font());

        font.setBold(true);

        mw_one->ui->lblTitleEditRecord->setFont(font);
        if (mw_one->ui && mw_one->ui->lblSyncNote) {
          QFont mFont = font;
          if (!isAndroid)
            mFont.setPointSize(9);
          else
            mFont.setPointSize(12);
          mw_one->ui->lblSyncNote->setFont(mFont);
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
  mw_one->ui->editCategory->setText("");

  isInitThemeEnd = true;

  if (isNeedExecDeskShortcut) {
    isNeedExecDeskShortcut = false;

    m_Method->closeMainEntranceWindow();

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

void initAndroidGPU() {
  qputenv("QSG_RHI_BACKEND", "opengl");
  qputenv("QT_QUICK_BACKEND", "opengl");
  qputenv("QSG_INFO", "1");

  // 全局关闭Quick持久离屏图形缓存，替代不存在的静态函数
  qputenv("QSG_NO_PERSISTENT_GRAPHICS_CACHE", "1");
  qputenv("QT_RHI_NO_OFFSCREEN_BLIT", "1");
  qputenv("QT_QPA_GL_NO_PBO", "1");

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

int init_main_ai() {
  // ✅ 静态链接模式下，必须显式调用此函数注册 CPU 后端
  ggml_backend_cpu_init();

  llama_backend_init();

  // ✅ 【第一步】立即检查是否有后端被成功加载
  int num_backends = ggml_backend_reg_count();
  if (num_backends == 0) {
    fprintf(stderr,
            "❌ No backends loaded! Check that backend libraries "
            "(.so/.dll/.dylib) are present and dependencies are satisfied.\n");
    return 1;
  }

  printf("[backend] %d backend(s) loaded:\n", num_backends);
  for (int i = 0; i < num_backends; i++) {
    printf("  - %s\n", ggml_backend_reg_name(ggml_backend_reg_get(i)));
  }

  // ✅ 【第二步】确认有后端加载后，再获取 CPU 后端

  g_global_cpu_backend =
      ggml_backend_cpu_reg();  // 再次调用，拿到初始化后的有效指针

  if (!g_global_cpu_backend) {
    fprintf(stderr, "❌ CPU backend not loaded!\n");
    return 1;
  }

  // 可选调试日志：打印已加载的后端名，验证正确性
  printf("[backend] loaded: %s\n", ggml_backend_reg_name(g_global_cpu_backend));

  return 0;
}
