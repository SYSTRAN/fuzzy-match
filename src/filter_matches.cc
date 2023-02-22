#include <fuzzy/filter_matches.hh>

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

  FilterMatches::FilterMatches(float fuzzy,
                               unsigned p_length,
                               unsigned min_seq_len,
                               const Filter& filter)
      /* add a small epsilon to avoid rounding errors counting for an error */
      : fuzzy_threshold(fuzzy),
        min_exact_match(compute_min_exact_match(fuzzy, p_length)),
        _min_seq_len(min_seq_len),
        _p_length(p_length),
        _filter(filter)
  {}

  const bool
  FilterMatches::theoretical_rejection(size_t p_length, size_t s_length, const EditCosts &edit_costs) const
  {
    const float sizeDifference = std::abs((float)p_length - (float)s_length);
    float remaining_cost = (p_length >= s_length) ? edit_costs._insert : edit_costs._delete;
    float theoretical_bound = 1.f - remaining_cost * sizeDifference / Costs::get_normalizer(p_length, s_length, edit_costs);
    return theoretical_bound + 0.000005 < fuzzy_threshold;
  }

  const bool
  FilterMatches::theoretical_rejection_cover(size_t p_length, size_t s_length, size_t cover, const EditCosts &edit_costs) const
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
}
