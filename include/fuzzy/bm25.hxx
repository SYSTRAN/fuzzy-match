#include <stdexcept>

namespace fuzzy
{
  template<class Archive>
  void BM25::save(Archive& archive, unsigned int) const
  {
    // TODO add _bm25
    archive
    & _sorted
    & _bm25
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
  }

  template<class Archive>
  void BM25::load(Archive& archive, unsigned int version)
  {
    archive
    & _sorted
    & _bm25
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;

    compute_sentence_length();
  }
}
