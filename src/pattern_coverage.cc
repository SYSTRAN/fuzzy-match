#include "fuzzy/pattern_coverage.hh"

#include <algorithm>

namespace fuzzy
{

  PatternCoverage::PatternCoverage(const std::vector<unsigned>& pattern)
  {
    _words_count.reserve(pattern.size());
    for (const auto word : pattern)
      _words_count[word]++;
  }

  size_t PatternCoverage::count_covered_words(const unsigned* sentence, size_t sentence_length) const
  {
    size_t num_covered_words = 0;

    for (const auto& pair : _words_count)
    {
      const auto word = pair.first;
      const auto count = pair.second;
      if (std::find(sentence, sentence + sentence_length, word) != sentence + sentence_length)
        num_covered_words += count;
    }

    return num_covered_words;
  }

  bool equal_arrays(const size_t s_len, const size_t p_len, const unsigned* s, const unsigned* p)
  {
    if (s_len != p_len)
      return false;

    for (unsigned i = 0; i < p_len; i++)
      if (p[i] != s[i])
        return false;
    return true;
  }
}
