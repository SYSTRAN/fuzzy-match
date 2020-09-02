#pragma once

namespace fuzzy
{
  struct Costs
  {
    /* cost when 2 normalized tokens are identical but differ on real forms
       for instance numbers */
    const float diff_real = 2.0;
    /* cost of case difference */
    const float diff_case = 1.0;
    /* cost if one of the token (only) is a penalty token */
    const int penalty = 1;

    float diff_word;
  };
}