#ifndef QTONEDRIVELIB_GLOBAL_H
#define QTONEDRIVELIB_GLOBAL_H

#include <QtCore/qglobal.h>

#ifndef TEST_QTONEDRIVE

#if defined(QTONEDRIVELIB_LIBRARY)
#define QTONEDRIVELIBSHARED_EXPORT Q_DECL_EXPORT
#else
#define QTONEDRIVELIBSHARED_EXPORT  // Q_DECL_IMPORT
                                    // 去掉这个参数，否vs2019编译会出现“不允许
                                    // dllimport 静态数据成员 的定义”
#endif
#else
#define QTONEDRIVELIBSHARED_EXPORT
#endif
#endif  // QTONEDRIVELIB_GLOBAL_H
