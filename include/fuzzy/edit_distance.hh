#pragma once

#include <fuzzy/sentence.hh>
#include <fuzzy/costs.hh>

namespace fuzzy
{
  int   _edit_distance_char(const char *s1, int n1, const char *s2, int n2);

  float _edit_distance(const unsigned* thes, const Sentence &reals, int slen,
                       const unsigned* thep, const Tokens &realptok, int plen,
                       const std::vector<const char*>& st, const std::vector<int>& sn,
                       const std::vector<float> &idf_penalty, float idf_weight,
                       const Costs&, float max_fuzziness);
}

#include <fuzzy/edit_distance.hxx>
