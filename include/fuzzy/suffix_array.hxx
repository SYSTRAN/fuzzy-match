namespace fuzzy
{
  inline unsigned SuffixArray::operator[](const SuffixView& pos)const
  {
    return (_sentence_buffer[_sentence_pos[pos.sentence_id]+pos.subsentence_pos]);
  }

  inline unsigned SuffixArray::operator[](size_t s_id)const
  {
    return _sentence_pos[s_id];
  }

  inline const std::vector<unsigned> &SuffixArray::sentence_buffer() const
  {
    return _sentence_buffer;
  }

  inline const std::vector<SuffixView> &SuffixArray::suffixid2sentenceid() const
  {
    return _suffixes;
  }

  inline const unsigned*
  SuffixArray::get_sentence(std::size_t suffix_id, std::size_t* length) const
  {
    const auto offset = _sentence_pos[suffix_id];
    const auto* sentence = _sentence_buffer.data() + offset;
    *length = *sentence;
    return sentence + 1;
  }

  inline unsigned short
  SuffixArray::sentence_length(std::size_t suffix_id) const
  {
    return _sentence_length[suffix_id];
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
