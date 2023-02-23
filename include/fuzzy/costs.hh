#pragma once

#include <algorithm>

namespace fuzzy
{
  struct EditCosts {
    const float insert_cost;
    const float delete_cost;
    const float replace_cost;
    EditCosts():
      insert_cost(1),
      delete_cost(1),
      replace_cost(1) {}
    EditCosts(
      const float insert_cost,
      const float delete_cost,
      const float replace_cost
    ): 
      insert_cost(insert_cost),
      delete_cost(delete_cost),
      replace_cost(replace_cost) {}
    bool is_null() const {
      return (insert_cost == 0.) && (delete_cost == 0.) && (replace_cost == 0.);
    }
  };
  struct Costs
  {
    static const float get_normalizer(
      size_t pattern_length,
      size_t sentence_length,
      const EditCosts& edit_costs
    ) {
      if (edit_costs.is_null())
        return 1.;
      if (edit_costs.insert_cost + edit_costs.delete_cost <= edit_costs.replace_cost) {
        return edit_costs.insert_cost * (float)pattern_length + edit_costs.delete_cost * (float)sentence_length;
      } else if (pattern_length <= sentence_length) {
        return (edit_costs.replace_cost - edit_costs.delete_cost) * (float)pattern_length + edit_costs.delete_cost * (float)sentence_length;
      } else {
        return (edit_costs.replace_cost - edit_costs.insert_cost) * (float)sentence_length + edit_costs.insert_cost * (float)pattern_length;
      }
    }
    Costs(size_t pattern_length, size_t sentence_length)
      : diff_word(100.f / std::max(pattern_length, sentence_length))
    {
    }
    Costs(size_t pattern_length, size_t sentence_length, EditCosts edit_costs)
      : diff_word(100.f / get_normalizer(pattern_length, sentence_length, edit_costs))
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
