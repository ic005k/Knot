#include "llama.h"

// 删掉 extern "C"
int llama_build_number(void) { return 0; }
const char* llama_commit(void) { return ""; }
const char* llama_compiler(void) { return ""; }
const char* llama_build_target(void) { return ""; }
const char* llama_build_info(void) {
    static char buf[64] = "build-0";
    return buf;
}