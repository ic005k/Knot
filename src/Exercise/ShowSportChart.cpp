#include "defines.h"
#include "src/Exercise/Steps.h"

void Steps::showSportsData() {
  /*
// 1. 关闭已有对话框
if (statsDialog != nullptr) {
  delete statsDialog;
}
statsDialog = new QDialog(this);
statsDialog->setWindowTitle(tr("Sports Statistics"));

if (this->parentWidget()) {
  statsDialog->resize(this->parentWidget()->size());
} else {
  statsDialog->resize(400, 710);  // 增加10px容纳频次曲线，保持紧凑
}

// 2. 读取 INI 文件（硬读1-12月，解决数据错乱）
QString title = "";  // m_Steps->ui->btnSelGpsDate->text();
QStringList list = title.split("-");
if (list.size() < 2) {
  QLabel* errLabel = new QLabel(tr("Date format error!"), statsDialog);
  QVBoxLayout* errLayout = new QVBoxLayout(statsDialog);
  errLayout->setContentsMargins(15, 15, 15, 15);
  errLayout->addWidget(errLabel);
  statsDialog->setLayout(errLayout);
  statsDialog->exec();
  return;
}

QString stry = list.at(0).trimmed();

QString yearGroup = stry;

QSettings reg(iniDir + stry + "-gpslist.ini", QSettings::IniFormat);

// 3. 数据处理（硬读1-12月，确保顺序和完整性）

QVector<MonthData> monthDataList(12);  // 索引0-11对应1-12月

// 硬读1-12月，不依赖childKeys，确保每个月都被处理
reg.beginGroup(yearGroup);
for (int month = 1; month <= 12; ++month) {  // 从1到12循环
  QString monthKey = QString::number(month);
  QString value =
      reg.value(monthKey).toString();  // 直接读取该月数据，空则说明无数据
  if (value.isEmpty()) {
    continue;  // 无数据则保持默认0
  }

  QStringList parts = value.split("-=-");
  if (parts.size() != 8) {
    qWarning() << "[INI Error] Month" << month
               << "data format invalid:" << value;
    continue;
  }

  // 填充对应月份数据（索引=月份-1）
  MonthData& data = monthDataList[month - 1];
  data.cyclingDist = parts[2].toDouble();
  data.cyclingCount = parts[3].toInt();
  data.hikingDist = parts[4].toDouble();
  data.hikingCount = parts[5].toInt();
  data.runningDist = parts[6].toDouble();
  data.runningCount = parts[7].toInt();
}
reg.endGroup();

//
------------------------------------------------------------------------------
// 新增：自定义频次曲线Widget
// 提取三种运动的频次数据（直接复用monthDataList）
QVector<int> cyclingCounts, hikingCounts, runningCounts;
for (const auto& data : std::as_const(monthDataList)) {
  cyclingCounts.append(data.cyclingCount);
  hikingCounts.append(data.hikingCount);
  runningCounts.append(data.runningCount);
}
// 创建自定义曲线Widget（传入数据、主题、父窗口）
FrequencyCurveWidget* countCurveWidget = new FrequencyCurveWidget(
    cyclingCounts, hikingCounts, runningCounts, isDark, statsDialog);
//
--------------------------------------------------------------------------------

// 4. 创建水平条形图（原有里程图表，保持不变）
QChart* chart = new QChart();
chart->setTitle(tr("%1 Monthly Sports Statistics").arg(stry));
chart->legend()->setAlignment(Qt::AlignBottom);
chart->setMargins(QMargins(5, 0, 0, 5));  // 彻底去除图表内边距，给Y轴腾空间
if (isDark) {
  chart->setTheme(QChart::ChartThemeDark);
} else {
  chart->setTheme(QChart::ChartThemeLight);
}

// 4.1 创建水平条形系列和集合
QHorizontalBarSeries* barSeries = new QHorizontalBarSeries();
QBarSet* cyclingSet = new QBarSet(tr("Ride"));
QBarSet* hikingSet = new QBarSet(tr("Hike"));
QBarSet* runningSet = new QBarSet(tr("Run"));

// 主题适配颜色
if (isDark) {
  cyclingSet->setBrush(QColor(90, 189, 94));
  hikingSet->setBrush(QColor(255, 171, 44));
  runningSet->setBrush(QColor(183, 70, 201));
} else {
  cyclingSet->setBrush(QColor(76, 175, 80));
  hikingSet->setBrush(QColor(255, 152, 0));
  runningSet->setBrush(QColor(156, 39, 176));
}

// 4.2 填充数据
for (int i = 0; i < 12; ++i) {
  const MonthData& data = monthDataList[i];
  *cyclingSet << data.cyclingDist;
  *hikingSet << data.hikingDist;
  *runningSet << data.runningDist;
}

barSeries->append(cyclingSet);
barSeries->append(hikingSet);
barSeries->append(runningSet);
barSeries->setBarWidth(0.85);  // 最大化条形宽度，减少垂直方向空隙

// 隐藏图表文字标签
barSeries->setLabelsVisible(false);

// 4.3 配置坐标轴（✅ 根治：显示...+字体不生效+刻度错位半格
QBarCategoryAxis* axisY =
    new QBarCategoryAxis();  // 柱状图专属分类轴，根治错位
QStringList monthLabels;
for (int i = 1; i <= 12; ++i) {
  monthLabels << QString::number(i);  // 数字标签 1-12月
}

// 直接追加标签列表，无需手动指定坐标，天然对齐0-11
axisY->append(monthLabels);

axisY->setLabelsAngle(0);
axisY->setTitleText(tr("Month"));
axisY->setTitleVisible(false);

QFont yAxisFont;
yAxisFont.setPointSize(isAndroid ? 15 : 8);  // 字体大小
axisY->setLabelsFont(yAxisFont);             // 月份文字字体
axisY->setTitleFont(yAxisFont);              // Y轴标题字体

// X轴配置
QValueAxis* axisX = new QValueAxis();
axisX->setTitleText(tr("Distance (KM)"));
axisX->setTickCount(5);

// 4.4 添加系列和轴到图表
chart->addSeries(barSeries);
// 把【手动配置好的坐标轴】添加到图表中
chart->addAxis(axisY, Qt::AlignLeft);    // Y轴(月份)靠左显示，固定写法
chart->addAxis(axisX, Qt::AlignBottom);  // X轴(里程)靠下显示，固定写法
// 让条形系列 绑定 坐标轴
barSeries->attachAxis(axisY);
barSeries->attachAxis(axisX);

// 4.5 图表视图配置（增加高度，去除边框）
CustomChartView* chartView = new CustomChartView();
chartView->setChart(chart);
chartView->setMonthData(monthDataList);

chartView->setRenderHint(QPainter::Antialiasing);
chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
int mh = 600;
if (!isAndroid) mh = 450;
chartView->setMinimumHeight(mh);
chartView->setMaximumHeight(mh);
chartView->setFrameStyle(QFrame::NoFrame);
chartView->setContentsMargins(0, 0, 0, 0);

// 5. 整合布局（用自定义曲线Widget替换原countChartView）
QVBoxLayout* mainLayout = new QVBoxLayout(statsDialog);
mainLayout->setContentsMargins(5, 5, 5, 5);  // 最小化整体边距
mainLayout->setSpacing(5);

QFont font = this->font();
font.setBold(true);

QLabel* totalLabel =
    new QLabel(tr("Total Distance") + " : " + strTotalDistance, statsDialog);
totalLabel->setFrameShape(QFrame::Box);
totalLabel->setFrameShadow(QFrame::Plain);
totalLabel->setLineWidth(1);
totalLabel->setContentsMargins(5, 5, 5, 5);
totalLabel->setAlignment(Qt::AlignCenter);

totalLabel->setFont(font);

QLabel* monthlyLabel = new QLabel(m_monthlyStatsText, statsDialog);
monthlyLabel->setAlignment(Qt::AlignLeft);
monthlyLabel->setWordWrap(true);
monthlyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

QLabel* yearlyLabel = new QLabel(m_yearlyStatsText, statsDialog);
yearlyLabel->setAlignment(Qt::AlignLeft);
yearlyLabel->setWordWrap(true);
yearlyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

if (isAndroid)
  font.setPointSize(13);
else
  font.setPointSize(9);
monthlyLabel->setFont(font);
yearlyLabel->setFont(font);

monthlyLabel->setFrameShape(QFrame::Box);     // 边框形状：矩形框
monthlyLabel->setFrameShadow(QFrame::Plain);  // 边框阴影：无阴影（简洁风格）
monthlyLabel->setLineWidth(1);                // 边框线宽：1px
monthlyLabel->setContentsMargins(5, 5, 5, 5);
yearlyLabel->setFrameShape(QFrame::Box);
yearlyLabel->setFrameShadow(QFrame::Plain);
yearlyLabel->setLineWidth(1);
yearlyLabel->setContentsMargins(5, 5, 5, 5);

QHBoxLayout* hboxLayout = new QHBoxLayout();
hboxLayout->addWidget(monthlyLabel);
hboxLayout->addWidget(yearlyLabel);

// 缩小的关闭按钮
QPushButton* closeButton = new QPushButton(tr("Close"), statsDialog);
QFont btnFont = closeButton->font();
btnFont.setPointSize(10);
closeButton->setFont(btnFont);
closeButton->setStyleSheet(isDark ? R"(
      QPushButton {
          background-color: #66bb6a;
          color: white;
          border: none;
          padding: 5px 14px;
          border-radius: 4px;
      }
      QPushButton:hover { background-color: #4caf50; }
      QPushButton:pressed { background-color: #388e3c; }
  )"
                                  : R"(
      QPushButton {
          background-color: #4CAF50;
          color: white;
          border: none;
          padding: 5px 14px;
          border-radius: 4px;
      }
      QPushButton:hover { background-color: #45a049; }
      QPushButton:pressed { background-color: #3e8e41; }
  )");

// 布局顺序：totalLabel → 标签横向布局 → 自定义频次曲线 → 里程图表 → 按钮
mainLayout->addWidget(totalLabel);
mainLayout->addLayout(hboxLayout);
mainLayout->addWidget(countCurveWidget);  // 替换为自定义曲线Widget
mainLayout->addWidget(chartView);         // 原有：里程图表
mainLayout->addSpacing(3);
mainLayout->addWidget(closeButton, 0, Qt::AlignCenter);
closeButton->hide();

// 6. 信号连接
connect(closeButton, &QPushButton::clicked, statsDialog, &QDialog::close);
connect(statsDialog, &QDialog::finished, this, [this, chartView, chart](int) {
  chartView->deleteLater();
  chart->deleteLater();
  delete this->statsDialog;
  this->statsDialog = nullptr;
});

statsDialog->setLayout(mainLayout);
statsDialog->exec();
*/
}

/**
 * @brief 从 INI 文件中读取指定年月的运动统计数据
 * @param year  年份字符串，如 "2026"
 * @param month 月份 (1-12)，传 0 表示读取全年12个月数据
 * @return QVector<MonthData>
 *         - month > 0: 返回仅含1个元素的向量（对应当月）
 *         - month == 0: 返回含12个元素的向量（索引0-11对应1-12月）
 *         - 读取失败或无数据时返回空向量或零值向量
 */
QVector<MonthData> Steps::loadSportsData(const QString& year, int month) const {
  QVector<MonthData> result;

  // 参数校验
  if (year.isEmpty()) {
    qWarning() << "[loadSportsData] Year parameter is empty";
    return result;
  }

  const bool loadSingleMonth = (month >= 1 && month <= 12);
  const bool loadAllMonths = (month == 0);

  if (!loadSingleMonth && !loadAllMonths) {
    qWarning() << "[loadSportsData] Invalid month:" << month
               << "(expected 0 for all, or 1-12)";
    return result;
  }

  // 读取 INI 文件
  const QString iniPath = iniDir + year + "-gpslist.ini";
  QSettings reg(iniPath, QSettings::IniFormat);

  if (!QFile::exists(iniPath)) {
    qWarning() << "[loadSportsData] INI file not found:" << iniPath;
    return loadAllMonths ? QVector<MonthData>(12) : result;
  }

  // 确定要读取的月份范围
  const int startMonth = loadSingleMonth ? month : 1;
  const int endMonth = loadSingleMonth ? month : 12;

  // 预分配空间
  result.resize(endMonth - startMonth + 1);

  reg.beginGroup(year);
  for (int m = startMonth; m <= endMonth; ++m) {
    const QString value = reg.value(QString::number(m)).toString();
    if (value.isEmpty()) {
      continue;  // 保持默认零值
    }

    const QStringList parts = value.split("-=-");
    if (parts.size() != 8) {
      qWarning() << "[loadSportsData] Month" << m
                 << "data format invalid (expected 8 fields):" << value;
      continue;
    }

    // 计算结果向量中的索引
    const int idx = m - startMonth;
    MonthData& data = result[idx];
    data.cyclingDist = parts[2].toDouble();
    data.cyclingCount = parts[3].toInt();
    data.hikingDist = parts[4].toDouble();
    data.hikingCount = parts[5].toInt();
    data.runningDist = parts[6].toDouble();
    data.runningCount = parts[7].toInt();
  }
  reg.endGroup();

  for (int i = 0; i < result.size(); ++i) {
    const auto& d = result[i];
    qDebug() << QString(
                    "Month %1: cycle=%2km/%3次, hike=%4km/%5次, run=%6km/%7次")
                    .arg(i + 1)
                    .arg(d.cyclingDist)
                    .arg(d.cyclingCount)
                    .arg(d.hikingDist)
                    .arg(d.hikingCount)
                    .arg(d.runningDist)
                    .arg(d.runningCount);
  }

  return result;
}

void Steps::printSportsDataSummary(const QVector<MonthData>& data,
                                   const QString& year) {
  if (data.isEmpty()) {
    qDebug() << "[SportsData] No data for year:" << year;
    return;
  }

  // ========== 1. 全年汇总 ==========
  double totalCycleDist = 0, totalHikeDist = 0, totalRunDist = 0;
  int totalCycleCount = 0, totalHikeCount = 0, totalRunCount = 0;
  int activeMonths = 0;

  for (const auto& d : data) {
    totalCycleDist += d.cyclingDist;
    totalCycleCount += d.cyclingCount;
    totalHikeDist += d.hikingDist;
    totalHikeCount += d.hikingCount;
    totalRunDist += d.runningDist;
    totalRunCount += d.runningCount;
    if (d.totalDistance() > 0 || d.totalCount() > 0) ++activeMonths;
  }

  double grandTotalDist = totalCycleDist + totalHikeDist + totalRunDist;
  int grandTotalCount = totalCycleCount + totalHikeCount + totalRunCount;

  qDebug().noquote() << QString("\n===== %1 年度运动数据统计 =====").arg(year);
  qDebug().noquote() << QString("📊 全年总计: %1 km / %2 次 (有效月份: %3/12)")
                            .arg(grandTotalDist, 0, 'f', 2)
                            .arg(grandTotalCount)
                            .arg(activeMonths);
  qDebug().noquote() << QString("   🚴 骑行: %1 km / %2 次")
                            .arg(totalCycleDist, 0, 'f', 2)
                            .arg(totalCycleCount);
  qDebug().noquote() << QString("   🥾 徒步: %1 km / %2 次")
                            .arg(totalHikeDist, 0, 'f', 2)
                            .arg(totalHikeCount);
  qDebug().noquote() << QString("   🏃 跑步: %1 km / %2 次")
                            .arg(totalRunDist, 0, 'f', 2)
                            .arg(totalRunCount);

  // ========== 2. 各月明细 ==========
  qDebug().noquote() << "\n----- 各月明细 -----";
  for (int i = 0; i < data.size(); ++i) {
    const auto& d = data[i];
    // 跳过完全无数据的月份（可选）
    if (d.totalDistance() == 0 && d.totalCount() == 0) continue;

    qDebug().noquote() << QString(
                              "  %1月: 🚴%2km/%3次 | 🥾%4km/%5次 | 🏃%6km/%7次 "
                              "| 合计%8km/%9次")
                              .arg(i + 1, 2)
                              .arg(d.cyclingDist, 0, 'f', 2)
                              .arg(d.cyclingCount)
                              .arg(d.hikingDist, 0, 'f', 2)
                              .arg(d.hikingCount)
                              .arg(d.runningDist, 0, 'f', 2)
                              .arg(d.runningCount)
                              .arg(d.totalDistance(), 0, 'f', 2)
                              .arg(d.totalCount());
  }
  qDebug().noquote() << "===========================\n";
}

void Steps::getMySportData(QString strYear, int m) {
  QVector<MonthData> result = loadSportsData(strYear, 0);
  printSportsDataSummary(result, strYear);
}