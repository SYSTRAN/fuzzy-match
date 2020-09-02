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
  void SuffixArray::load(Archive& archive, unsigned int)
  {
    archive
    & _sorted
    & _suffixes
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;

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
