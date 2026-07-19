#ifndef GLOBALAI_H
#define GLOBALAI_H

#include <memory>

class BaseEmbeddingEngine;
class VectorDb;

extern std::unique_ptr<BaseEmbeddingEngine> g_embEngine;

// 全局向量数据库
extern std::unique_ptr<VectorDb> g_vectorDb;

#ifdef VECTOR_SEARCH
bool initGlobalAiEngine();
#endif

#endif