#include "WordPieceTokenizer.h"

#include <cctype>
#include <cstring>

bool WordPieceTokenizer::loadVocabFromGguf(struct bert_ctx* ctx) {
  clearVocab();
  if (!ctx) return false;

  // 复用原生接口遍历词表，不需要新增bert_get_vocab_size
  const int32_t maxPossibleId = bert_n_max_tokens(ctx);
  for (VocabId id = 0; id < maxPossibleId; ++id) {
    const char* tokenStr = bert_vocab_id_to_token(ctx, id);
    if (!tokenStr) continue;
    m_vocab[std::string(tokenStr)] = id;
  }

  // 查找4个特殊标记ID
  m_clsId = getTokenId("[CLS]");
  m_sepId = getTokenId("[SEP]");
  m_padId = getTokenId("[PAD]");
  m_unkId = getTokenId("[UNK]");

  // 校验关键标记是否存在
  if (m_clsId < 0 || m_sepId < 0 || m_padId < 0 || m_unkId < 0) {
    clearVocab();
    return false;
  }
  return true;
}

void WordPieceTokenizer::clearVocab() {
  m_vocab.clear();
  m_clsId = m_sepId = m_padId = m_unkId = -1;
}

std::string WordPieceTokenizer::cleanText(const std::string& raw) {
  std::string out;
  // 使用索引遍历，避免取临时字符指针
  for (size_t i = 0; i < raw.size(); ++i) {
    unsigned char ch = static_cast<unsigned char>(raw[i]);
    // 过滤不可见控制字符
    if (ch < 0x20 && ch != '\t' && ch != '\n') continue;
    // 全角空格 0xE3 0x80 0x80 判断
    if (ch == 0xE3 && i + 2 < raw.size()) {
      unsigned char c1 = static_cast<unsigned char>(raw[i + 1]);
      unsigned char c2 = static_cast<unsigned char>(raw[i + 2]);
      if (c1 == 0x80 && c2 == 0x80) {
        out += ' ';
        continue;
      }
    }
    out += static_cast<char>(ch);
  }

  // 合并连续空白
  std::string finalStr;
  bool lastSpace = false;
  for (char c : out) {
    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!lastSpace) {
        finalStr += ' ';
        lastSpace = true;
      }
    } else {
      finalStr += c;
      lastSpace = false;
    }
  }
  return finalStr;
}

std::vector<std::string> WordPieceTokenizer::basicSplit(
    const std::string& text) {
  std::vector<std::string> words;
  std::string buf;
  for (char c : text) {
    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!buf.empty()) {
        words.push_back(buf);
        buf.clear();
      }
    } else {
      buf += c;
    }
  }
  if (!buf.empty()) words.push_back(buf);
  return words;
}

std::vector<std::string> WordPieceTokenizer::wordPieceSplit(
    const std::string& word) {
  std::vector<std::string> subTokens;
  std::string remain = word;

  while (!remain.empty()) {
    std::string bestMatch;
    size_t bestLen = 0;
    // 从最长后缀匹配，标准WordPiece算法
    for (size_t l = remain.size(); l >= 1; l--) {
      std::string sub = remain.substr(0, l);
      if (!subTokens.empty()) sub = "##" + sub;
      if (m_vocab.find(sub) != m_vocab.end()) {
        bestMatch = sub;
        bestLen = l;
        break;
      }
    }
    // 无匹配子词，全部替换为UNK
    if (bestLen == 0) {
      subTokens.clear();
      subTokens.emplace_back("[UNK]");
      break;
    }
    subTokens.push_back(bestMatch);
    remain = remain.substr(bestLen);
  }
  return subTokens;
}

VocabId WordPieceTokenizer::getTokenId(const std::string& token) {
  auto iter = m_vocab.find(token);
  if (iter != m_vocab.end()) return iter->second;
  return m_unkId;
}

std::vector<VocabId> WordPieceTokenizer::encode(const std::string& utf8Text,
                                                int maxSeqLen) {
  std::vector<VocabId> ids;
  ids.push_back(m_clsId);

  std::string cleanStr = cleanText(utf8Text);
  std::vector<std::string> words = basicSplit(cleanStr);

  for (const auto& w : words) {
    auto subs = wordPieceSplit(w);
    for (const auto& sub : subs) {
      ids.push_back(getTokenId(sub));
      // 预留SEP位置，提前截断
      if ((int)ids.size() >= maxSeqLen - 1) goto truncate_end;
    }
  }
truncate_end:
  // 截断，末尾追加SEP
  if ((int)ids.size() > maxSeqLen - 1) ids.resize(maxSeqLen - 1);
  ids.push_back(m_sepId);

  // Padding补齐长度
  while ((int)ids.size() < maxSeqLen) ids.push_back(m_padId);

  return ids;
}