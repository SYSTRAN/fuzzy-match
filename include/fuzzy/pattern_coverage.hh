#pragma once

#include <unordered_map>
#include <vector>
#include <stddef.h>

namespace fuzzy
{

  class PatternCoverage
  {
  public:
    PatternCoverage(const std::vector<unsigned>& pattern);

    // Counts the number of words in the pattern that are also in the sentence.
    size_t count_covered_words(const unsigned* sentence, size_t sentence_length) const;

  private:
    std::unordered_map<unsigned, unsigned> _words_count;
  };

}
