#include <stdexcept>

namespace fuzzy
{
  inline unsigned
  NoFilter::get_sentence_length(size_t s_id) const
  {
    if (s_id + 1 == _sentence_pos.size())
      return _sentence_buffer.size() - _sentence_pos[s_id] -  2;
    return _sentence_pos[s_id + 1] - _sentence_pos[s_id] - 2;
  }
  
  template<class Archive>
  void NoFilter::save(Archive& archive, unsigned int) const
  {
    archive
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
  }

  template<class Archive>
  void NoFilter::load(Archive& archive, unsigned int)
  {
    archive
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
  }
}
