#include <fuzzy/edit_distance.hh>

namespace fuzzy
{
  float
  _edit_distance(const unsigned* s1, const Sentence &real1, int n1,
                 const unsigned* s2, const Tokens &real2tok, int n2,
                 const std::vector<const char*>& st2, const std::vector<int>& sn2,
                 const std::vector<float> &idf_penalty, float idf_weight,
                 const Costs& costs,
                 float max_fuzzyness)
  {
    boost::multi_array<float, 2> arr(boost::extents[n1+1][n2+1]);
    boost::multi_array<int, 2> cost_tag(boost::extents[n1+1][n2+1]);

    std::vector<const char*> st1(n1+1, nullptr);
    std::vector<int> sn1(n1+1, 0);
    real1.get_itoks(st1, sn1);
    Tokens real1tok = (Tokens)real1;

    /* we have a fixed cost corresponding to trailing penalty_tokens */
    arr[0][0] = _edit_distance_char(st1[n1], sn1[n1], st2[n2], sn2[n2]);
    cost_tag[0][0] = costs.penalty * _edit_distance_char(st1[0], sn1[0], st2[0], sn2[0]);

    for (int i = 1; i < n1 + 1; i++) {
      arr[i][0] = arr[i-1][0] + costs.diff_word + costs.penalty*sn1[i];
      cost_tag[i][0] = costs.penalty * _edit_distance_char(st1[i], sn1[i], st2[0], sn2[0]);
    }
    for (int j = 1; j < n2 + 1; j++) {
      arr[0][j] = arr[0][j-1] + costs.diff_word + costs.penalty*sn2[j];
      if (idf_weight)
        arr[0][j] += idf_penalty[j-1]*idf_weight;
      cost_tag[0][j] = costs.penalty * _edit_distance_char(st1[0], sn1[0], st2[j], sn2[j]);
    }

    for (int i = 1; i < n1 + 1; i++)
    {
      float min = std::numeric_limits<float>::max();
      for (int j = 1; j < n2 + 1; j++)
      {
        int diff = 0;
        float penalty_j1 = 0;
        if (idf_weight)
          penalty_j1 = idf_penalty[j-1]*idf_weight;
        if (s1[i-1] != s2[j-1]) {
          diff = costs.diff_word + penalty_j1;
        }
        else if (real1tok[i-1] != real2tok[j-1]) {
          /* is difference only a case difference */
          if (strchr("LUMC", real1tok[i-1][0]))
            diff = costs.diff_case;
          else {
            diff = costs.diff_real;
          }
        }

        cost_tag[i][j] = costs.penalty * _edit_distance_char(st1[i], sn1[i], st2[j], sn2[j]);

        const auto distance = std::min(
          {
            arr[i - 1][j] + costs.diff_word + cost_tag[i - 1][j],
            arr[i][j - 1] + costs.diff_word + cost_tag[i][j - 1] + penalty_j1,
            arr[i - 1][j - 1] + diff + cost_tag[i - 1][j - 1]
          });

        arr[i][j] = distance;
        min = std::min(min, distance);
      }
      if (min > max_fuzzyness)
        return min;
    }
#ifdef DEBUG
    printf("---\n");
    for(int i = 0; i < n1 + 1; i++)
    {
      for (int j = 0; j < n2 + 1; j++) printf("%6.2f ", arr[i][j]);
      printf("\n");
    }
#endif
    return arr[n1][n2];
  }
}
