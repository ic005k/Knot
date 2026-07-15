#ifndef GLOBALAI_H
#define GLOBALAI_H

#include <memory>

#include "BaseEmbeddingEngine.h"

// 向量库仅在开启VECTOR_SEARCH时引入、声明
#ifdef VECTOR_SEARCH
#include "VectorDb.h"
extern VectorDb g_vecDb;
#endif

extern std::unique_ptr<BaseEmbeddingEngine> g_embEngine;

bool initGlobalAiEngine();

#endif