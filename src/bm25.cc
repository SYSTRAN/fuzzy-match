#include <fuzzy/bm25.hh>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/vocab_indexer.hh>
#include <cassert>

namespace fuzzy
{
  BM25::BM25(const FilterIndexParams& params)
    : _k1(params.bm25_k1), _b(params.bm25_b), _ratio_idf(params.bm25_ratio_idf)
  {}
  BM25::~BM25() {}
  unsigned
  BM25::add_sentence(const std::vector<unsigned>& sentence)
  {
    size_t sidx = _sentence_pos.size();
    _sentence_pos.push_back(_sentence_buffer.size());

    /* first token in sentence buffer is the sentence size */
    _sentence_buffer.push_back(sentence.size());

    for (size_t i = 0; i < sentence.size(); i++)
    {
      _sentence_buffer.push_back(sentence[i]);
    }
    _sentence_buffer.push_back(fuzzy::VocabIndexer::SENTENCE_SEPARATOR);
    _prepared = false;

    return sidx;
  }

  void BM25::prepare(size_t vocab_size) {
    if (_prepared)
      return;
    _vocab_size = vocab_size;
    // build TF + DF
    std::unordered_map<std::pair<int, int>, float, PairHasher> tf;
    std::vector<unsigned> doc_freq(vocab_size, 0);
    std::unordered_set<unsigned> seen_in_doc;
    std::unordered_map<int, std::unordered_set<int>> inverse_index_set;
    unsigned sentence_idx = 0;
    unsigned relative_pos = 1;
    for (unsigned pos = 1; pos < _sentence_buffer.size(); pos++, relative_pos++)
    {
      if (sentence_idx + 1 < _sentence_pos.size() && _sentence_pos[sentence_idx + 1] == pos)
      {
        relative_pos = 0;
        sentence_idx++;
        seen_in_doc.clear();
      }
      else if (_sentence_buffer[pos] != 0)
      {
        tf.emplace(std::make_pair(_sentence_buffer[pos], sentence_idx), 0);
        tf[std::make_pair(_sentence_buffer[pos], sentence_idx)]++;

        auto result_insert = seen_in_doc.insert(_sentence_buffer[pos]);
        if (result_insert.second)
          doc_freq[_sentence_buffer[pos]]++;

        inverse_index_set[_sentence_buffer[pos]].insert(sentence_idx);
      }
    }
    // build IDF from DF
    std::vector<float> idf(vocab_size);
    for (unsigned term = 0; term < vocab_size; term++)
    {
      idf[term] = std::log(
        ((float)_sentence_pos.size() - (float)doc_freq[term] + 0.5) /
        ((float)doc_freq[term] + 0.5)
      );
    }
    float threshold_idf = std::log((1 - _ratio_idf) / _ratio_idf);
    for (const auto& kvp : inverse_index_set) {
      // > 0 => appears in less than half of the sentences
      if (idf[kvp.first] > threshold_idf) // TODO: parameter or new method
        _inverse_index[kvp.first] = std::vector<int>(kvp.second.begin(), kvp.second.end());
    }
    inverse_index_set.clear();
    // build BM25 from TF and IDF
    float avg_doc_length = (float)_sentence_buffer.size() / (float)_sentence_pos.size() - 2;
    _key_value_bm25 = std::vector<std::pair<std::pair<int, int>, float>>(tf.size());
    unsigned i = 0;
    for (const auto& tf_i : tf)
    {
      _key_value_bm25[i] = {
        {tf_i.first.second, tf_i.first.first},
        bm25_score(tf_i.first.first, tf_i.first.second, avg_doc_length, tf_i.second, idf)
      };
      i++;
    }
    _prepared = true;
  }
}
