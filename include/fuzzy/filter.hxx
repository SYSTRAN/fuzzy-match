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

  inline unsigned short
  Filter::get_sentence_length(size_t suffix_id) const
  {
    return _sentence_length[suffix_id];
  }


  // template<class Archive>
  // void Filter::save(Archive& archive, unsigned int) const
  // {
  //   archive
  //   & _sorted
  //   & _suffixes
  //   & _sentence_buffer
  //   & _sentence_pos
  //   & _quickVocabAccess;
  // }

  // template<class Archive>
  // void Filter::load(Archive& archive, unsigned int version)
  // {
  //     archive & _sorted;
  // //   if (version == 1)
  // //   {
  // //     archive
  // //     & _sorted
  // //     & _suffixes
  // //     & _sentence_buffer
  // //     & _sentence_pos
  // //     & _quickVocabAccess;
  // //   }
  // //   else if (version == 0) // Old format using std::pair
  // //   {
  // //     std::vector<std::pair<unsigned, unsigned short>> suffixes;

  // //     archive
  // //     & _sorted
  // //     & _suffixes
  // //     & _sentence_buffer
  // //     & _sentence_pos
  // //     & _quickVocabAccess;
  // //   }
  // //   else
  // //     throw std::invalid_argument("Unsupported FMI format");

  // //   compute_sentence_length();
  // }
}
