#include <fuzzy/ngram_matches.hh>

#include <cmath>

namespace fuzzy
{
  NGramMatches::NGramMatches(float fuzzy,
                             unsigned p_length,
                             unsigned min_seq_len,
                             const SuffixArray& suffixArray)
<<<<<<< HEAD
      /* add a small epsilon to avoid rounding errors counting for an error */
      : FilterMatches(fuzzy, p_length, min_seq_len, suffixArray)
  {}
=======
    /* add a small epsilon to avoid rounding errors counting for an error */
    : fuzzy_threshold(fuzzy),
      _p_length(p_length),
      _min_seq_len(min_seq_len),
      _suffixArray(suffixArray)
  {
  }
>>>>>>> master

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
  NGramMatches::register_suffix_range_match(size_t begin, size_t end, unsigned match_length, const EditCosts &edit_costs)
  {
    // lazy injection feature - if match_length smaller than min_seq_len, we will not process the suffixes for the moment
    if (match_length < _min_seq_len)
      return;
    
    const SuffixArray& suffix_array = static_cast<const SuffixArray&>(_filter);
    // For each suffix that matches at least match_length
    for (auto i = begin; i < end; i++)
    {
      // The size difference between the suffix and the pattern is too large for the suffix to be accepted
      long int p_length = (long int)_p_length;
      long int s_length = (long int)suffix_array.get_sentence_length(i);

      if (theoretical_rejection(p_length, s_length, edit_costs))
        continue;


      // Get or create the PatternMatch corresponding to the sentence (of the suffix that matched)
      const auto sentence_id = suffix_array.get_suffix_view(i).sentence_id;
      auto& longest_match = _longest_matches.try_emplace(sentence_id, match_length).first.value();
      longest_match = std::max(longest_match, match_length);
    }
  }
}
