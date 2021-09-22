#pragma once

#include <algorithm>

namespace fuzzy
{
  struct Costs
  {
    Costs(size_t pattern_length, size_t sentence_length)
      : diff_word(100.f / std::max(pattern_length, sentence_length))
    {
    }

    // Cost when 2 normalized words are different.
    const float diff_word;
    // Cost when 2 normalized words are identical but their original forms are different (e.g. numbers).
    const float diff_real = 2.0;
    // Cost when 2 normalized words are identical but their original forms only differ in casing.
    const float diff_case = 1.0;
  };
}
