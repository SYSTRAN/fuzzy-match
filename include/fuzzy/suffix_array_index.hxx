namespace fuzzy
{
  inline const SuffixArray&
  SuffixArrayIndex::get_SuffixArray() const
  {
    return _suffixArray;
  }

  inline VocabIndexer&
  SuffixArrayIndex::get_VocabIndexer()
  {
    return _vocabIndexer;
  }

  inline void
  SuffixArrayIndex::sort()
  {
    _suffixArray.sort(_vocabIndexer.size());
  }

  inline size_t
  SuffixArrayIndex::size() const
  {
    return _ids.size();
  }

  template<class Archive>
  void
  SuffixArrayIndex::serialize(Archive& ar, const unsigned int version)
  {
    ar &
    _vocabIndexer &
    _suffixArray &
    _ids &
      _real_tokens;
  }
}
