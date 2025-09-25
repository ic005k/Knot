#include <qglobal.h>
#pragma once

#ifdef Q_OS_ANDROID
#define ANDROID_MAIN_ACTIVITY "com/x/MyActivity"
#endif

inline QString iniFile, iniDir, privateDir, bakfileDir, strDate, readDate,
    noteText, strStats, SaveType, strY, strM, btnYText, btnMText, btnDText,
    errorInfo, CurrentYearMonth, zipfile, txt, searchStr, currentMDFile,
    copyText, imgFileName, defaultFontFamily, customFontFamily, encPassword,
    btnYearText, btnMonthText, strPage, ebookFile, strTitle, fileName,
    strOpfPath, catalogueFile, strShowMsg, strStartTotalTime, strOpfFile,
    oldOpfPath, strEpubTitle, strPercent;

inline QString ver = "2.1.31";
inline QString appName = "Knot";

inline int fontSize, red, iPage, sPos, totallines, s_y1, s_m1, s_d1, s_y2, s_m2,
    s_d2, totalPages, currentPage, infoProgBarValue, infoProgBarMax,
    currentTabIndex, today;

inline int chartMax = 5;
inline int baseLines = 50;
inline int htmlIndex = 0;
inline int minBytes = 200000;
inline int maxBytes = 400000;

inline int zlibMethod = 1;
inline int readerFontSize = 18;
inline int epubFileMethod = 2;

inline bool isAndroid, isReadEnd, isZipOK, isMenuImport, isDownData, loading,
    isEncrypt, isIOS, isEpub, isEpubError, isText, isPDF, isInitThemeEnd,
    isUpData, isRemovedTopItem, isReport, isReadTWEnd, isWindows, isEBook;

inline bool isPasswordError = false;
inline bool isrbFreq = true;
inline bool isAdd = false;
inline bool isReadEBookEnd = true;
inline bool isSaveEnd = true;
inline bool isBreak = false;
inline bool isDark = false;
inline bool isDelData = false;
inline bool isWholeMonth = true;
inline bool isDateSection = false;
inline bool isOpen = false;
inline bool isZH_CN = false;
inline bool isNeedExecDeskShortcut = false;
