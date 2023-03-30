#include <fuzzy/bm25.hh>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/vocab_indexer.hh>
#include <cassert>

namespace fuzzy
{
  BM25::BM25(float k1, float b)
    : _k1(k1), _b(b)
  {}
  BM25::~BM25()
  {
    // if (_reverted_index != nullptr)
    //   delete _reverted_index;
  }
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
    _sorted = false;

    return sidx;
  }

  void BM25::sort(size_t vocab_size) {
    if (_sorted)
      return;
    _vocab_size = vocab_size;
    // std::cerr << "build TF + DF" << std::endl;
    // build TF + DF
    // boost::multi_array<unsigned, 2> tf(boost::extents[vocab_size][_sentence_pos.size()]);
    std::unordered_map<std::pair<int, int>, float, PairHasher> tf;
    // std::fill(tf.data(), tf.data() + tf.num_elements(), 0);
    std::vector<unsigned> doc_freq(vocab_size, 0);
    std::unordered_set<unsigned> seen_in_doc;
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
        // tf[_sentence_buffer[pos]][sentence_idx]++; // term frequency count
        tf.emplace(std::make_pair(_sentence_buffer[pos], sentence_idx), 0);
        tf[std::make_pair(_sentence_buffer[pos], sentence_idx)]++;

        auto result_insert = seen_in_doc.insert(_sentence_buffer[pos]);
        if (result_insert.second)
          doc_freq[_sentence_buffer[pos]]++;
      }
    }
    // std::cerr << "build IDF from DF" << std::endl;
    // build IDF from DF
    std::vector<float> idf(vocab_size);
    for (unsigned term = 0; term < vocab_size; term++)
    {
      idf[term] = std::log(
        ((float)_sentence_pos.size() - (float)doc_freq[term] + 0.5) /
        ((float)doc_freq[term] + 0.5)
      );
    }
    // std::cerr << "build BM25 from TF and IDF" << std::endl;
    // build BM25 from TF and IDF
    float avg_doc_length = (float)_sentence_buffer.size() / (float)_sentence_pos.size() - 2;
    // std::cerr << _sentence_pos.size() << " x " << vocab_size << std::endl;
    // _bm25 = new boost::multi_array<float, 2>(boost::extents[vocab_size][_sentence_pos.size()]);

    // _sid_vec = std::vector<int>(tf.size());
    // _term_vec = std::vector<int>(tf.size());
    // _bm25_vec = std::vector<float>(tf.size());
    _key_value_bm25 = std::vector<std::pair<std::pair<int, int>, float>>(tf.size());
    unsigned i = 0;
    for (const auto& tf_i : tf)
    {
      // _sid_vec[i] = tf_i.first.second;
      // _term_vec[i] = tf_i.first.first;
      // _bm25_vec[i] = bm25_score(_term_vec[i], _sid_vec[i], avg_doc_length, tf_i.second, idf);
      _key_value_bm25[i] = {
        {tf_i.first.second, tf_i.first.first},
        bm25_score(tf_i.first.first, tf_i.first.second, avg_doc_length, tf_i.second, idf)
      };
      i++;
    }

    // for (unsigned s_id = 0; s_id < _sentence_pos.size(); s_id++)
    //   for (unsigned term = 0; term < vocab_size; term++)
    //     (*_bm25)[term][s_id] = bm25_score(term, s_id, avg_doc_length, tf, idf);
        
    _sorted = true;
    // std::cerr << "done" << std::endl;
  }
}
