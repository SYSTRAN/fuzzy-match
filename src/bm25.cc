#include <fuzzy/bm25.hh>

#include <fuzzy/ngram_matches.hh>
#include <fuzzy/vocab_indexer.hh>
#include <cassert>

namespace fuzzy
{
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

  double BM25::bm25_score(unsigned term, unsigned s_id, double avg_doc_length)
  {
    return _idf[term] * ((_k1 + 1) * _tf[term][s_id]) / (_tf[term][s_id] + _k1 * ((1 - _b) + (_b * (_sentence_length[s_id] / avg_doc_length))));
  }

  // Calculate the BM25 score for a query and a document s_id
  double BM25::bm25_score(std::vector<unsigned> query, unsigned s_id)
  {
    double score = 0;
    for (VocabIndexer::index_t& term : query) {
      score += _bm25[term][s_id];
    }
    return score;
  }

  void BM25::compute_bm25_cache()
  {
    double avg_doc_length = (double)_sentence_buffer.size() / (double)_sentence_pos.size();
    _bm25 = boost::multi_array<unsigned, 2>(boost::extents[vocab_size][_sentence_pos.size()]);
    for (unsigned s_id = 0; s_id < _sentence_pos.size(); s_id++)
      for (unsigned term = 0; term < vocab_size; term++)
        _bm25[term][s_id] = bm25_score(term, s_id, avg_doc_length);
  }

  void BM25::sort(size_t vocab_size) {
    _sorted = true;
  }

  void BM25::compute_sentence_length()
  {
    // _sentence_length.resize(_suffixes.size());
    // for (std::size_t suffix_id=0; suffix_id < _suffixes.size(); suffix_id++)
    // {
    //   const auto sentence_id = _suffixes[suffix_id].sentence_id;
    //   _sentence_length[suffix_id] = _sentence_buffer[_sentence_pos[sentence_id]];
    // }
  }
}
