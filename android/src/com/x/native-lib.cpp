#include <jni.h>
#include <QJniEnvironment>
#include <QJniObject>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_your_package_InputMethodHelper_nativeResetReferences(JNIEnv *env, jclass clazz)
{
    // 重置所有 JNI 全局引用
    QJniEnvironment jniEnv;
    if (jniEnv.env()) {
        // 清除所有全局引用
        jniEnv.env()->DeleteGlobalRefs();
    }
    
    // 重置 Qt 的 JNI 缓存
    QJniObject::clearClassCache();
}

#ifdef __cplusplus
}
#endif