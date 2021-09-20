#pragma once

#include <map>
#include <vector>

namespace fuzzy
{

  class PatternCoverage
  {
  public:
    PatternCoverage(const std::vector<unsigned>& pattern);

    // Counts the number of words in the pattern that are also in the sentence.
    size_t count_covered_words(const unsigned* sentence, size_t sentence_length) const;

  private:
    std::map<unsigned, std::vector<size_t>> _words_positions;
    size_t _pattern_length;
  };

}
