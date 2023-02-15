#include <fuzzy/ngram_matches.hh>

#include <cmath>

namespace fuzzy
{
  unsigned compute_min_exact_match(float fuzzy, unsigned p_length)
  {
    const auto differences = (unsigned)std::ceil(p_length * (1.f - fuzzy));
    // we split (p_length - differences) in  (differences + 1) parts
    // the minimum value of the largest part size is obtained by dividing and taking ceil
    return std::ceil((p_length - differences) / (differences + 1.));
  }

  NGramMatches::NGramMatches(float fuzzy,
                             unsigned p_length,
                             unsigned min_seq_len,
                             const SuffixArray& suffixArray)
      /* add a small epsilon to avoid rounding errors counting for an error */
      : fuzzy_threshold(fuzzy),
        // max_differences_with_pattern((unsigned)std::floor(p_length * (1.f - fuzzy) + 0.00005)),
        min_exact_match(compute_min_exact_match(fuzzy, p_length)),
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

  const bool
  NGramMatches::theoretical_rejection(size_t p_length, size_t s_length, const EditCosts &edit_costs) const
  {
    const float sizeDifference = std::abs((float)p_length - (float)s_length);
    float remaining_cost = (p_length >= s_length) ? edit_costs._insert : edit_costs._delete;
    float theoretical_bound = 1.f - remaining_cost * sizeDifference / Costs::get_normalizer(p_length, s_length, edit_costs);
    
    // bool old_test = sizeDifference > max_differences_with_pattern;
    bool new_test = theoretical_bound + 0.000005 < fuzzy_threshold;

    return new_test;
    return theoretical_bound + 0.000005 < fuzzy_threshold;
  }

  const bool
  NGramMatches::theoretical_rejection_cover(size_t p_length, size_t s_length, size_t cover, const EditCosts &edit_costs) const
  {
    float theoretical_bound;
    if (edit_costs._insert + edit_costs._delete < edit_costs._replace)
    {
      theoretical_bound = 1.f - (edit_costs._insert * ((float)s_length - (float)cover) +
                                 edit_costs._delete * ((float)p_length - (float)cover)) /
                                    Costs::get_normalizer(p_length, s_length, edit_costs);
    } else {
      float cost_remaining = (p_length > s_length) ? edit_costs._insert : edit_costs._delete;
      float min_length = (p_length > s_length) ? s_length : p_length;
      float max_length = (p_length > s_length) ? p_length : s_length;
      theoretical_bound = 1.f - (edit_costs._replace * (min_length - cover) +
                                 cost_remaining * (max_length - min_length)) /
                                    Costs::get_normalizer(p_length, s_length, edit_costs);
    }
    return theoretical_bound + 0.000005 < fuzzy_threshold;
  }

  void
  NGramMatches::register_suffix_range_match(size_t begin, size_t end, unsigned match_length, const EditCosts &edit_costs)
  {
    // lazy injection feature - if match_length smaller than min_seq_len, we will not process the suffixes for the moment
    if (match_length < min_exact_match || match_length < _min_seq_len)
      return;

    // For each suffix that matches at least match_length
    for (auto i = begin; i < end; i++)
    {
      // The size difference between the suffix and the pattern is too large for the suffix to be accepted
      long int p_length = (long int)_p_length;
      long int s_length = (long int)_suffixArray.get_sentence_length(i);

      if (theoretical_rejection(p_length, s_length, edit_costs))
        continue;

      // Get or create the PatternMatch corresponding to the sentence (of the suffix that matched)
      const auto sentence_id = _suffixArray.get_suffix_view(i).sentence_id;
      auto& longest_match = _longest_matches.try_emplace(sentence_id, match_length).first.value();
      longest_match = std::max(longest_match, match_length);
    }
  }
}
