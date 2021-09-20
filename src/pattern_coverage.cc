#include "fuzzy/pattern_coverage.hh"

namespace fuzzy
{

  PatternCoverage::PatternCoverage(const std::vector<unsigned>& pattern)
    : _pattern_length(pattern.size())
  {
    for (size_t i = 0; i < pattern.size(); ++i)
      _words_positions[pattern[i]].push_back(i);
  }

  size_t PatternCoverage::count_covered_words(const unsigned* sentence, size_t sentence_length) const
  {
    std::vector<bool> covered_words(_pattern_length, false);
    size_t num_covered_words = 0;

    for (size_t i = 0; i < sentence_length; ++i)
    {
      const auto it = _words_positions.find(sentence[i]);
      if (it == _words_positions.end())  // Sentence word is not in the pattern.
        continue;

      for (const auto position : it->second)
      {
        if (!covered_words[position])
        {
          covered_words[position] = true;
          num_covered_words++;
        }
      }
    }

    return num_covered_words;
  }

}
