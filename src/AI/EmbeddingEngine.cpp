#include "EmbeddingEngine.h"

#include <QByteArray>
#include <cstring>

#include "WordPieceTokenizer.h"

EmbeddingEngine::EmbeddingEngine(const QString& ggufPath) {
  // Qt字符串转标准UTF8 C字符串
  std::string pathUtf8 = ggufPath.toUtf8().toStdString();
  // bert.h 标准加载接口 bert_load_from_file
  m_ctx = bert_load_from_file(pathUtf8.c_str());
}

EmbeddingEngine::~EmbeddingEngine() {
  if (m_ctx) {
    bert_free(m_ctx);
    m_ctx = nullptr;
  }
}

bool EmbeddingEngine::isValid() const { return m_ctx != nullptr; }

QVector<float> EmbeddingEngine::encode(const QString& text) {
  QVector<float> resultVec;
  if (!isValid()) return resultVec;

  // 1. Qt文本统一转UTF8，适配全平台中文不乱码
  std::string utf8Text = text.toUtf8().toStdString();
  WordPieceTokenizer tokenizer;
  if (!tokenizer.loadVocabFromGguf(m_ctx)) return resultVec;

  // 2. 分词生成标准vocab id序列
  std::vector<VocabId> tokenIds =
      tokenizer.encode(utf8Text, bert_n_max_tokens(m_ctx));
  const int32_t tokenCount = static_cast<int32_t>(tokenIds.size());
  const int32_t threadCount = 4;  // 可根据设备动态调整

  // 3. 获取向量维度，预分配结果容器
  const int32_t embedDim = bert_n_embd(m_ctx);
  resultVec.resize(embedDim);

  // 4. 调用原生bert_eval执行推理（分离分词/推理接口，可控性更强）
  bert_eval(m_ctx, threadCount, tokenIds.data(), tokenCount, resultVec.data());

  return resultVec;
}