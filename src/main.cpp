
#include <QApplication>
#include <QDir>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>  // Qt JNI 环境
#include <QJniObject>       // Qt JNI 对象封装
#endif

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
#include <QtConcurrent>

#include "MainWindow.h"
#include "lib/cppjieba/Jieba.hpp"
#include "lib/quazip/quazip.h"
#include "lib/quazip/quazipfile.h"

std::unique_ptr<cppjieba::Jieba> jieba;

extern QString iniFile, txtFile, appName, iniDir, privateDir, bakfileDir,
    customFontFamily, defaultFontFamily;
extern int fontSize;
extern void RegJni(const char* myClassName);
extern bool isDark;
extern MainWindow* mw_one;
extern QSettings* iniPreferences;
extern Method* m_Method;

void loadTheme(bool isDark);
void loadLocal();

void showSplash();
namespace Shapes {
enum ShapeType { Triangle, Rectangle, Square, Circle, Ellipse };
}
constexpr int ShapeTypeCount = 5;  // 显式定义枚举数量
Shapes::ShapeType safeConvertShapeType(int value);

bool unzipToDir(const QString& zipPath, const QString& destDir);
int deleteDirfile(QString dirName);
QString loadText(QString textFile);
QString getTextEditLineText(QTextEdit* txtEdit, int i);
void TextEditToFile(QTextEdit* txtEdit, QString fileName);
void StringToFile(QString buffers, QString fileName);
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

bool zh_cn = false;
bool isAndroid, isIOS;
bool isInitThemeEnd;
bool isNeedExecDeskShortcut = false;

QSplashScreen* splash;

#define Cross_Origin

int main(int argc, char* argv[]) {
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

  QApplication::setStyle(QStyleFactory::create("Fusion"));

#ifdef Q_OS_ANDROID

  isAndroid = true;

#else
  // 桌面端配置

#endif

  QApplication app(argc, argv);

  loadLocal();

  showSplash();

  // 禁用文本选择（针对所有的可输入的编辑框）
  // qputenv("QT_QPA_NO_TEXT_HANDLES", "1");

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
  RegJni("com/x/MyActivity");
  RegJni("com/x/ClockActivity");
  RegJni("com/x/ShareReceiveActivity");
  RegJni("com/x/NoteEditor");
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

  isIOS = false;

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
    mw_one->init_Theme();

    // 强制窗口重绘
    QEvent updateEvent(QEvent::UpdateRequest);
    QApplication::sendEvent(mw_one, &updateEvent);
  }

  QFont font = qApp->font();
  font.setPointSize(fontSize);
  qApp->setFont(font);

  isInitThemeEnd = true;

  if (isNeedExecDeskShortcut) {
    isNeedExecDeskShortcut = false;
    mw_one->execDeskShortcut();
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
    zh_cn = false;

  } else if (locale.language() == QLocale::Chinese) {
    bool tr = false;
    tr = translator0.load(":/src/cn.qm");
    if (tr) {
      qApp->installTranslator(&translator0);
      zh_cn = true;
    }

    bool tr1 = false;
    tr1 = translator1.load(":/res/tr/qt_zh_CN.qm");
    if (tr1) {
      qApp->installTranslator(&translator1);
      zh_cn = true;
    }

    bool tr2 = false;
    tr2 = translator2.load(":/res/tr/qtbase_zh_CN.qm");
    if (tr2) {
      qApp->installTranslator(&translator2);
      zh_cn = true;
    }

    bool tr3 = false;
    tr3 = translator3.load(":/res/tr/qtlocation_zh_CN.qm");
    if (tr3) {
      qApp->installTranslator(&translator3);
      zh_cn = true;
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

int deleteDirfile(QString dirName) {
  QDir directory(dirName);
  if (!directory.exists()) {
    return true;
  }

  QString srcPath = QDir::toNativeSeparators(dirName);
  if (!srcPath.endsWith(QDir::separator())) srcPath += QDir::separator();

  QStringList fileNames = directory.entryList(
      QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
  bool error = false;
  for (QStringList::size_type i = 0; i != fileNames.size(); ++i) {
    QString filePath = srcPath + fileNames.at(i);
    QFileInfo fileInfo(filePath);
    if (fileInfo.isFile() || fileInfo.isSymLink()) {
      QFile::setPermissions(filePath, QFile::WriteOwner);
      if (!QFile::remove(filePath)) {
        error = true;
      }
    } else if (fileInfo.isDir()) {
      if (!deleteDirfile(filePath)) {
        error = true;
      }
    }
  }

  if (!directory.rmdir(QDir::toNativeSeparators(directory.path()))) {
    error = true;
  }
  return !error;
}

QString loadText(QString textFile) {
  bool isExists = QFile(textFile).exists();
  if (isExists) {
    QFile file(textFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      qDebug() << "Cannot read file";

    } else {
      QTextStream in(&file);

      QString text = in.readAll();
      return text;
    }
    file.close();
  }

  return "";
}

QString getTextEditLineText(QTextEdit* txtEdit, int i) {
  QTextBlock block = txtEdit->document()->findBlockByNumber(i);
  txtEdit->setTextCursor(QTextCursor(block));
  QString lineText = txtEdit->document()->findBlockByNumber(i).text();
  return lineText;
}

void TextEditToFile(QTextEdit* txtEdit, QString fileName) {
  QFile* file;
  file = new QFile;
  file->setFileName(fileName);
  bool ok = file->open(QIODevice::WriteOnly | QIODevice::Text);
  if (ok) {
    QTextStream out(file);
    out << txtEdit->toPlainText();
    file->close();

  } else
    qDebug() << "Write failure!" << fileName;

  delete file;
}

void StringToFile(QString buffers, QString fileName) {
  QFile* file;
  file = new QFile;
  file->setFileName(fileName);
  bool ok = file->open(QIODevice::WriteOnly | QIODevice::Text);
  if (ok) {
    QTextStream out(file);
    out << buffers;
    file->close();

  } else
    qDebug() << "Write failure!" << fileName;

  delete file;
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

void showSplash() {
  // 获取屏幕尺寸并创建全屏画布
  QSize targetSize;
  qreal dpr = qApp->devicePixelRatio();
  if (isAndroid) {
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    targetSize = screenGeometry.size() * dpr;
  } else {
    targetSize = QSize(300 * dpr, 100 * dpr);
  }

  // 创建透明画布
  QPixmap pixmap(targetSize);
  // 使用中度中性灰背景 (RGB: 100,100,100)
  QColor bgColor(100, 100, 100);
  pixmap.fill(bgColor);  // 设置中性背景色
  // 在透明背景上直接绘制文本
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);

  // 设置不规则形状的密度和大小范围
  const int shapeCount = 100;  // 形状数量
  const int minSize = 4 * dpr;
  const int maxSize = 25 * dpr;

  // 定义形状类型和半透明颜色调色板

  QVector<QColor> colors = {
      QColor(220, 220, 220, 70),  // 浅灰白
      QColor(180, 180, 200, 60),  // 灰蓝色
      QColor(200, 200, 220, 80),  // 淡紫灰
      QColor(170, 190, 210, 50)   // 灰青色
  };

  // 使用随机数引擎
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> sizeDist(minSize, maxSize);
  std::uniform_int_distribution<int> typeDist(
      0, ShapeTypeCount - 1);  // 使用枚举数量
  std::uniform_int_distribution<int> colorDist(0, colors.size() - 1);
  std::uniform_int_distribution<int> posXDist(0, pixmap.width());
  std::uniform_int_distribution<int> posYDist(0, pixmap.height());

  // 绘制随机不规则形状
  for (int i = 0; i < shapeCount; ++i) {
    int x = posXDist(gen);
    int y = posYDist(gen);
    int size1 = sizeDist(gen);
    int size2 = sizeDist(gen);
    QColor color = colors[colorDist(gen)];

    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);

    // 使用安全转换
    switch (safeConvertShapeType(typeDist(gen))) {
      case Shapes::ShapeType::Triangle:
        // 随机三角形
        {
          QPolygon triangle;
          triangle << QPoint(x, y) << QPoint(x + size1, y - size2 / 2)
                   << QPoint(x + size1 / 2, y - size1);
          painter.drawPolygon(triangle);
        }
        break;

      case Shapes::ShapeType::Rectangle:
        // 随机矩形
        painter.drawRect(x, y, size1, size2);
        break;

      case Shapes::ShapeType::Square:
        // 正方形 (尺寸相同)
        painter.drawRect(x, y, size1, size1);
        break;

      case Shapes::ShapeType::Circle:
        // 正圆
        painter.drawEllipse(x, y, size1, size1);
        break;

      case Shapes::ShapeType::Ellipse:
        // 椭圆
        painter.drawEllipse(x, y, size1, size2);
        break;
    }
  }

  // 设置文字样式
  QFont font = painter.font();
  if (isAndroid)
    font.setPointSizeF(16 * dpr);  // 根据设备DPI缩放文字大小
  else
    font.setPointSizeF(10 * dpr);
  font.setBold(true);
  painter.setFont(font);
  painter.setPen(QColor(135, 206, 250));  // 设置文字颜色
  // 计算文字位置（居中显示）
  QFontMetrics metrics(font);
  QRect textRect = metrics.boundingRect(pixmap.rect(), Qt::AlignCenter,
                                        QObject::tr("Loading, please wait..."));
  textRect.moveCenter(pixmap.rect().center());
  // 绘制文字
  painter.drawText(textRect, Qt::AlignCenter,
                   QObject::tr("Loading, please wait..."));
  painter.end();
  // 设置设备像素比
  pixmap.setDevicePixelRatio(dpr);

  splash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
  splash->setFixedSize(targetSize / dpr);
  splash->setAttribute(Qt::WA_TranslucentBackground);  // 设置为透明背景
  splash->show();
}

// 安全转换函数
Shapes::ShapeType safeConvertShapeType(int value) {
  if (value >= 0 && value < ShapeTypeCount) {
    return static_cast<Shapes::ShapeType>(value);
  }
  // 默认返回第一个枚举值或根据需求调整
  qWarning() << "Invalid ShapeType value:" << value;
  return Shapes::ShapeType::Triangle;
}

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
