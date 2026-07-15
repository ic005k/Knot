#ifndef GLOBALAI_H
#define GLOBALAI_H

#include <memory>
class BaseEmbeddingEngine;
extern std::unique_ptr<BaseEmbeddingEngine> g_embEngine;

#ifdef VECTOR_SEARCH
bool initGlobalAiEngine();
#endif

#endif