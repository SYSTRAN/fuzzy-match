#include <stdexcept>

namespace fuzzy
{
  inline const unsigned*
  SuffixArray::get_suffix(const SuffixView& p, size_t* length) const
  {
    const auto* sentence = get_sentence(p.sentence_id, length);
    const auto prefix_length = p.subsentence_pos - 1;
    if (length)
      *length -= prefix_length;
    return sentence + prefix_length;
  }

  inline const SuffixView&
  SuffixArray::get_suffix_view(size_t suffix_id) const
  {
    return _suffixes[suffix_id];
  }

  template<class Archive>
  void SuffixArray::save(Archive& archive, unsigned int) const
  {
    archive
    & _sorted
    & _suffixes
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
  }

  template<class Archive>
  void SuffixArray::load(Archive& archive, unsigned int version)
  {
    if (version == 1)
    {
      archive
      & _sorted
      & _suffixes
      & _sentence_buffer
      & _sentence_pos
      & _quickVocabAccess;
    }
    else if (version == 0) // Old format using std::pair
    {
      std::vector<std::pair<unsigned, unsigned short>> suffixes;

      archive
      & _sorted
      & suffixes
      & _sentence_buffer
      & _sentence_pos
      & _quickVocabAccess;

      _suffixes.reserve(suffixes.size());
      for (const auto& suffix : suffixes)
      {
        SuffixView suffixView;
        suffixView.sentence_id = suffix.first;
        suffixView.subsentence_pos = suffix.second;
        _suffixes.push_back(suffixView);
      }
    }
    else
      throw std::invalid_argument("Unsupported FMI format");

    compute_sentence_length();
  }

  template<class Archive>
  void SuffixView::serialize(Archive& archive, const unsigned int)
  {
    archive &
    sentence_id &
    subsentence_pos;
  }
}
