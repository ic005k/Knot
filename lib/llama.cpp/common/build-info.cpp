#include "llama.h"

extern "C" {
    int llama_build_number(void) { return 0; }
    const char* llama_commit(void) { return ""; }
    const char* llama_compiler(void) { return ""; }
    const char* llama_build_target(void) { return ""; }
}