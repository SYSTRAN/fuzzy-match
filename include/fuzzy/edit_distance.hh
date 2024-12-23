#pragma once

#include <limits>
#include <algorithm>
#include <iostream>

#include <fuzzy/sentence.hh>
#include <fuzzy/costs.hh>

namespace fuzzy
{
  int   _edit_distance_char(const char *s1, int n1, const char *s2, int n2);

  float _edit_distance(const unsigned* thes, const Sentence &reals, int slen,
                       const unsigned* thep, const Tokens &realptok, int plen,
                       const std::vector<const char*>& st, const std::vector<int>& sn,
                       const std::vector<float> &idf_penalty, float idf_weight,
                       const EditCosts&,
                       const Costs&,
                       float max_fuzziness = std::numeric_limits<float>::max());
  float _edit_distance(const unsigned* s1, int n1,
                       const unsigned* s2, int n2,
                       const EditCosts& edit_costs,
                       const Costs& costs,
                       float max_fuzzyness = std::numeric_limits<float>::max());

  float _edit_distance_cover(const unsigned* thes, const Sentence &reals, int slen,
                             const unsigned* thep, const Tokens &realptok, int plen,
                             const std::vector<const char*>& st, const std::vector<int>& sn,
                             const std::vector<float> &idf_penalty, float idf_weight,
                             const EditCosts&,
                             const Costs&,
                             std::vector<float>& cover,
                             const bool idf_cover = false,
                             float max_fuzziness = std::numeric_limits<float>::max());
}

#include <fuzzy/edit_distance.hxx>
