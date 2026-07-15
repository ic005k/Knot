#ifndef WORDPIECETOKENIZER_H
#define WORDPIECETOKENIZER_H

#include <string>
#include <unordered_map>
#include <vector>

// 导入bert C原生接口，自动extern C隔离名称修饰
extern "C" {
#include "bert.h"
}

// 别名简化，全程匹配bert.h原生词表ID类型
using VocabId = bert_vocab_id;

class WordPieceTokenizer {
 public:
  WordPieceTokenizer() = default;
  ~WordPieceTokenizer() = default;

  // 从GGUF bert上下文读取内置vocab词表，复用原生bert_vocab_id_to_token接口
  bool loadVocabFromGguf(struct bert_ctx* ctx);

  // 完整编码入口：文本UTF8清洗 → WordPiece切分 → 添加CLS/SEP → 截断/Padding
  std::vector<VocabId> encode(const std::string& utf8Text, int maxSeqLen = 512);

  // 清空词表缓存，切换模型时调用
  void clearVocab();

 private:
  // 核心词表映射：token文本 → token id
  std::unordered_map<std::string, VocabId> m_vocab;
  // 特殊标记ID（multilingual-e5系列固定）
  VocabId m_clsId = -1;
  VocabId m_sepId = -1;
  VocabId m_padId = -1;
  VocabId m_unkId = -1;

  // 1. UTF-8文本预处理：清洗空白、全角转半角、过滤控制字符
  std::string cleanText(const std::string& raw);
  // 2. 基础空白分割，拆分原始子串
  std::vector<std::string> basicSplit(const std::string& text);
  // 3. 标准WordPiece子词拆分（兼容中文单字、英文子词）
  std::vector<std::string> wordPieceSplit(const std::string& word);
  // 4. 单个token查词表，未知词返回[UNK]
  VocabId getTokenId(const std::string& token);
};

#endif  // WORDPIECETOKENIZER_H