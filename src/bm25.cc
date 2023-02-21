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
