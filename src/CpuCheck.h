#ifndef CPU_CHECK_H
#define CPU_CHECK_H

#include <QDebug>
#include <QtGlobal>
#include <cstdint>

/**
 * @brief 轻量级 CPU 指令集兼容性检测
 *
 * 用于在运行时判断当前 CPU 是否支持 llama.cpp 编译时所需的指令集。
 * 本项目的 CMakeLists.txt 锁定了 AVX2 + FMA + F16C，因此 x86/x64
 * 平台必须检测这三项。
 *
 * - x86/x64 (Windows/Linux/macOS Intel): 执行 CPUID 检测。
 * - ARM/AArch64 (Android/iOS/鸿蒙/Apple Silicon): 直接放行（CMake 已配置
 * NEON）。
 */
namespace CpuCheck {

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || \
    defined(_M_IX86)

// ===================== x86 / x64 架构 =====================

#ifdef _MSC_VER
// MSVC 编译器
#include <intrin.h>
static inline void run_cpuid(uint32_t eax, uint32_t ecx, uint32_t out[4]) {
  __cpuidex((int*)out, eax, ecx);
}
#else
// GCC / Clang 编译器
#include <cpuid.h>
static inline void run_cpuid(uint32_t eax, uint32_t ecx, uint32_t out[4]) {
  __cpuid_count(eax, ecx, out[0], out[1], out[2], out[3]);
}
#endif

/**
 * @brief 检查 CPU 是否支持 AVX2 + FMA + F16C (Haswell / 4代酷睿及以上)
 * @return true 兼容，false 不兼容（老 CPU）
 */
inline bool isCompatibleWithLlama() {
  uint32_t r[4] = {};
  bool has_avx = false, has_fma = false, has_f16c = false, has_avx2 = false;

  // EAX=1, ECX=0 -> ECX: bit28=AVX, bit12=FMA, bit29=F16C
  run_cpuid(1, 0, r);
  has_avx = (r[2] >> 28) & 1;
  has_fma = (r[2] >> 12) & 1;
  has_f16c = (r[2] >> 29) & 1;

  // EAX=7, ECX=0 -> EBX: bit5=AVX2
  run_cpuid(7, 0, r);
  has_avx2 = (r[1] >> 5) & 1;

  qDebug() << "[CpuCheck] x86/x64 detected:"
           << "AVX:" << has_avx << "| AVX2:" << has_avx2 << "| FMA:" << has_fma
           << "| F16C:" << has_f16c;

  // 必须与 CMakeLists.txt 中锁定的 /arch:AVX2 和 -mavx2 -mfma -mf16c 严格一致
  return has_avx && has_avx2 && has_fma && has_f16c;
}

#else

// ===================== 非 x86 架构 (ARM, RISC-V 等) =====================
// 包括 Android (ARM64), iOS, 鸿蒙, Apple Silicon (M1/M2/M3) 等。
// 这些平台在 CMake 中已配置为使用 NEON 指令集，不存在 AVX2
// 兼容性问题，直接放行。

inline bool isCompatibleWithLlama() {
  qDebug() << "[CpuCheck] Non-x86 architecture detected, assuming compatible "
              "(NEON/Other).";
  return true;
}

#endif

}  // namespace CpuCheck

#endif  // CPU_CHECK_H