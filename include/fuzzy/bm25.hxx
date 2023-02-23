#include <stdexcept>

namespace fuzzy
{
  inline unsigned
  BM25::get_sentence_length(size_t s_id) const
  {
    if (s_id + 1 == _sentence_pos.size())
      return _sentence_buffer.size() - _sentence_pos[s_id];
    return _sentence_pos[s_id + 1] - _sentence_pos[s_id];
  }
  template<class Archive>
  void BM25::save(Archive& archive, unsigned int) const
  {
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
  }
}
