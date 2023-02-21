#include <stdexcept>

namespace fuzzy
{
  inline size_t
  BM25::num_sentences() const
  {
    return _sentence_pos.size();
  }

  inline const unsigned*
  BM25::get_sentence(size_t sentence_id, size_t* length) const
  {
    const auto offset = _sentence_pos[sentence_id];
    const auto* sentence = _sentence_buffer.data() + offset;
    if (length)
      *length = *sentence;
    return sentence + 1;
  }

  inline unsigned short
  BM25::get_sentence_length(size_t id) const
  {
    return _sentence_length[id];
  }


  template<class Archive>
  void BM25::save(Archive& archive, unsigned int) const
  {
    archive
    & _sorted
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
  }

  template<class Archive>
  void BM25::load(Archive& archive, unsigned int version)
  {
    archive
    & _sorted
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;

    compute_sentence_length();
  }
}
