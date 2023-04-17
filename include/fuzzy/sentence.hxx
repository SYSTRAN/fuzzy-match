namespace fuzzy
{
  inline Sentence::Sentence() {}

  inline bool Sentence::empty() const {
    return _tokstring.empty();
  }

  inline void Sentence::reserve(size_t s) {
    _tokstring.reserve(s);
  }

  inline void Sentence::set_itok(size_t idx, const std::string &itok) {
    _itoks[idx] += itok;
  }

  inline void Sentence::push_back(const std::string &s) {
    if (!_tokstring.empty())
      _tokstring += "\t";
    _tokstring += s;
  }

  template<class Archive>
  void
  Sentence::serialize(Archive& ar, const unsigned int)
  {
    ar &
      _tokstring &
      _itoks;
  }

}
