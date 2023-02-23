#include <stdexcept>

namespace fuzzy
{
  inline size_t
  Filter::num_sentences() const
  {
    return _sentence_pos.size();
  }

  inline const unsigned*
  Filter::get_sentence(size_t sentence_id, size_t* length) const
  {
    const auto offset = _sentence_pos[sentence_id];
    const auto* sentence = _sentence_buffer.data() + offset;
    if (length)
      *length = *sentence;
    return sentence + 1;
  }
}
