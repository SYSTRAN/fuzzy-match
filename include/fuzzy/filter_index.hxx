namespace fuzzy
{
  inline const Filter&
  FilterIndex::get_Filter() const
  {
    return *_filter;
  }

  inline const VocabIndexer&
  FilterIndex::get_VocabIndexer() const
  {
    return _vocabIndexer;
  }

  inline void
  FilterIndex::sort()
  {
    _filter->sort(_vocabIndexer.size());
  }

  inline size_t
  FilterIndex::size() const
  {
    return _ids.size();
  }

  inline size_t
  FilterIndex::max_tokens_in_pattern() const
  {
    return _max_tokens_in_pattern;
  }

  template<class Archive>
  void
  FilterIndex::save(Archive& ar, unsigned int) const
  {
    if (_type == IndexType::SUFFIX)
    {
      SuffixArray& suffix_array = static_cast<SuffixArray&>(*_filter);
      ar
        & _vocabIndexer
        & suffix_array
        & _ids
        & _real_tokens
        & _max_tokens_in_pattern;
    }
  }

  template<class Archive>
  void
  FilterIndex::load(Archive& ar, unsigned int version)
  {
    if (_type == IndexType::SUFFIX)
    {
      SuffixArray& suffix_array = static_cast<SuffixArray&>(*_filter);
      ar
        & _vocabIndexer
        & suffix_array
        & _ids
        & _real_tokens;
    }
    if (version >= 1)
      ar & _max_tokens_in_pattern;
  }

}

BOOST_CLASS_VERSION(fuzzy::FilterIndex, 1)
