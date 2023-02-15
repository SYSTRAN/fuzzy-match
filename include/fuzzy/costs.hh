#pragma once

#include <algorithm>

namespace fuzzy
{
  struct EditCosts {
    const float _insert;
    const float _delete;
    const float _replace;
    EditCosts():
      _insert(1),
      _delete(1),
      _replace(1) {}
    EditCosts(
      const float insert_cost,
      const float delete_cost,
      const float replace_cost
    ): 
      _insert(insert_cost),
      _delete(delete_cost),
      _replace(replace_cost) {}
    bool is_null() {
      return (_insert == 0.) && (_delete == 0.) && (_replace == 0.);
    }
  };
  struct Costs
  {
    public:
      static const float get_normalizer(
        size_t pattern_length,
        size_t sentence_length,
        EditCosts edit_costs
      ) {
        if (edit_costs.is_null())
          return 1.;
        if (edit_costs._insert + edit_costs._delete <= edit_costs._replace) {
          return edit_costs._insert * (float)pattern_length + edit_costs._delete * (float)sentence_length;
        } else if (pattern_length <= sentence_length) {
          return (edit_costs._replace - edit_costs._delete) * (float)pattern_length + edit_costs._delete * (float)sentence_length;
        } else {
          return (edit_costs._replace - edit_costs._insert) * (float)sentence_length + edit_costs._insert * (float)pattern_length;
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
