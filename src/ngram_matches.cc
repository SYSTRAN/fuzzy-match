#include <fuzzy/ngram_matches.hh>

#include <cmath>

namespace fuzzy
{
  NGramMatches::NGramMatches(float fuzzy,
                             unsigned p_length,
                             unsigned min_seq_len,
                             const SuffixArray& suffixArray)
    /* add a small epsilon to avoid rounding errors counting for an error */
    : max_differences_with_pattern((unsigned)std::floor(p_length * (1.f - fuzzy) + 0.00005)),
      _p_length(p_length),
      _min_seq_len(min_seq_len),
      _suffixArray(suffixArray)
  {
  }

  std::vector<std::pair<unsigned, unsigned>>
  NGramMatches::get_longest_matches() const
  {
    std::vector<std::pair<unsigned, unsigned>> sorted_matches(_longest_matches.begin(),
                                                              _longest_matches.end());
    std::sort(sorted_matches.begin(), sorted_matches.end(),
              [](const std::pair<unsigned, unsigned>& a, const std::pair<unsigned, unsigned>& b) {
                return a.second > b.second || (a.second == b.second && a.first < b.first);
              });
    return sorted_matches;
  }

  void
  NGramMatches::register_suffix_range_match(size_t begin, size_t end, unsigned match_length)
  {
    // lazy injection feature - if match_length smaller than min_seq_len, we will not process the suffixes for the moment
    if (match_length < _min_seq_len)
      return;

    // For each suffix that matches at least match_length
    for (auto i = begin; i < end; i++)
    {
      // The size difference between the suffix and the pattern is too large for the suffix to be accepted
      const auto sizeDifference = std::abs((long int)_p_length - (long int)_suffixArray.get_sentence_length(i));
      if (sizeDifference > max_differences_with_pattern)
        continue;

      // Get or create the PatternMatch corresponding to the sentence (of the suffix that matched)
      const auto sentence_id = _suffixArray.get_suffix_view(i).sentence_id;
      auto& longest_match = _longest_matches.try_emplace(sentence_id, match_length).first.value();
      longest_match = std::max(longest_match, match_length);
    }
  }
}
