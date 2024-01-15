#include <fuzzy/edit_distance.hh>

namespace fuzzy
{
  float
  _edit_distance(const unsigned* s1, const Sentence &real1, int n1,
                 const unsigned* s2, const Tokens &real2tok, int n2,
                 const std::vector<const char*>& st2, const std::vector<int>& sn2,
                 const std::vector<float> &idf_penalty, float idf_weight,
                 const EditCosts& edit_costs,
                 const Costs& costs,
                 float max_fuzzyness)
  {
    boost::multi_array<float, 2> arr(boost::extents[n1+1][n2+1]);
    boost::multi_array<int, 2> cost_tag(boost::extents[n1+1][n2+1]);
    /* idf_penalty(w) = log(nbre seqs / nbre occ w) */ 
    /* idf_weight = weight * costs.diff_word / log(nbre seqs) */ 

    std::vector<const char*> st1(n1+1, nullptr);
    std::vector<int> sn1(n1+1, 0);
    real1.get_itoks(st1, sn1);
    Tokens real1tok = (Tokens)real1;

    /* we have a fixed cost corresponding to trailing penalty_tokens */
    arr[0][0] = _edit_distance_char(st1[n1], sn1[n1], st2[n2], sn2[n2]);
    cost_tag[0][0] = _edit_distance_char(st1[0], sn1[0], st2[0], sn2[0]);

    for (int i = 1; i < n1 + 1; i++) {
      /* initialize distance source side (real1) */
      arr[i][0] = arr[i-1][0] + costs.diff_word * edit_costs.delete_cost + sn1[i];
      cost_tag[i][0] = _edit_distance_char(st1[i], sn1[i], st2[0], sn2[0]);
    }
    for (int j = 1; j < n2 + 1; j++) {
      /* initialize distance target side (real2tok) */
      arr[0][j] = arr[0][j-1] + costs.diff_word * edit_costs.insert_cost + sn2[j];
      if (idf_weight)
        arr[0][j] += idf_penalty[j-1] * idf_weight;
      cost_tag[0][j] = _edit_distance_char(st1[0], sn1[0], st2[j], sn2[j]);
    }

    for (int i = 1; i < n1 + 1; i++)
    {
      float min = std::numeric_limits<float>::max();
      for (int j = 1; j < n2 + 1; j++)
      {
        float diff = 0;
        float penalty_j1 = 0;
        if (idf_weight)
          penalty_j1 = idf_penalty[j-1] * idf_weight;
        if (s1[i-1] != s2[j-1]) {
          diff = edit_costs.replace_cost * costs.diff_word + penalty_j1;
        }
        else if (real1tok[i-1] != real2tok[j-1]) {
          /* is difference only a case difference */
          if (strchr("LUMC", real1tok[i-1][0]))
            diff = edit_costs.replace_cost * costs.diff_case;
          else {
            diff = edit_costs.replace_cost * costs.diff_real;
          }
        }

        cost_tag[i][j] = _edit_distance_char(st1[i], sn1[i], st2[j], sn2[j]);
        const auto distance = std::min(
          {
            arr[i - 1][j] + edit_costs.delete_cost * costs.diff_word + cost_tag[i - 1][j],
            arr[i][j - 1] + edit_costs.insert_cost * costs.diff_word + cost_tag[i][j - 1] + penalty_j1,
            arr[i - 1][j - 1] + diff + cost_tag[i - 1][j - 1]
          });

        arr[i][j] = distance;
        min = std::min(min, distance);
      }
      if (min > max_fuzzyness)
        return min;
    }
    return arr[n1][n2];
  }

  float
  _edit_distance(const unsigned* s1, int n1,
                 const unsigned* s2, int n2,
                 const EditCosts& edit_costs,
                 const Costs& costs,
                 float max_fuzzyness)
  {
    boost::multi_array<float, 2> arr(boost::extents[n1+1][n2+1]);

    for (int i = 1; i < n1 + 1; i++) {
      /* initialize distance source side (real1) */
      arr[i][0] = arr[i-1][0] + costs.diff_word * edit_costs.delete_cost;
    }
    for (int j = 1; j < n2 + 1; j++) {
      /* initialize distance target side (real2tok) */
      arr[0][j] = arr[0][j-1] + costs.diff_word * edit_costs.insert_cost;
    }

    for (int i = 1; i < n1 + 1; i++)
    {
      float min = std::numeric_limits<float>::max();
      for (int j = 1; j < n2 + 1; j++)
      {
        float diff = 0;

        if (s1[i-1] != s2[j-1]) {
          diff = edit_costs.replace_cost * costs.diff_word;
        }

        const auto distance = std::min(
          {
            arr[i - 1][j] + edit_costs.delete_cost * costs.diff_word,
            arr[i][j - 1] + edit_costs.insert_cost * costs.diff_word,
            arr[i - 1][j - 1] + diff
          });

        arr[i][j] = distance;
        min = std::min(min, distance);
      }
      if (min > max_fuzzyness)
        return min;
    }
    return arr[n1][n2];
  }

  float
  _edit_distance_cover(const unsigned* s1, const Sentence &real1, int n1,
                       const unsigned* s2, const Tokens &real2tok, int n2,
                       const std::vector<const char*>& st2, const std::vector<int>& sn2,
                       const std::vector<float> &idf_penalty, float idf_weight,
                       const EditCosts& edit_costs,
                       const Costs& costs,
                       std::vector<float>& cover,
                       const bool idf_cover,
                       float max_fuzzyness)
  {
    boost::multi_array<float, 2> arr(boost::extents[n1+1][n2+1]);
    boost::multi_array<unsigned, 2> traceback(boost::extents[n1+1][n2+1]);
    boost::multi_array<int, 2> cost_tag(boost::extents[n1+1][n2+1]);
    /* idf_penalty(w) = log(nbre seqs / nbre occ w) */ 
    /* idf_weight = weight * costs.diff_word / log(nbre seqs) */ 

    std::vector<const char*> st1(n1+1, nullptr);
    std::vector<int> sn1(n1+1, 0);
    real1.get_itoks(st1, sn1);
    Tokens real1tok = (Tokens)real1;

    /* we have a fixed cost corresponding to trailing penalty_tokens */
    arr[0][0] = _edit_distance_char(st1[n1], sn1[n1], st2[n2], sn2[n2]);
    cost_tag[0][0] = _edit_distance_char(st1[0], sn1[0], st2[0], sn2[0]);

    for (int i = 1; i < n1 + 1; i++) {
      /* initialize distance source side (real1) */
      arr[i][0] = arr[i-1][0] + costs.diff_word * edit_costs.delete_cost + sn1[i];
      traceback[i][0] = 0;
      cost_tag[i][0] = _edit_distance_char(st1[i], sn1[i], st2[0], sn2[0]);
    }
    for (int j = 1; j < n2 + 1; j++) {
      /* initialize distance target side (real2tok) */
      arr[0][j] = arr[0][j-1] + costs.diff_word * edit_costs.insert_cost + sn2[j];
      traceback[0][j] = 1;
      if (idf_weight)
        arr[0][j] += idf_penalty[j-1] * idf_weight;
      cost_tag[0][j] = _edit_distance_char(st1[0], sn1[0], st2[j], sn2[j]);
    }

    for (int i = 1; i < n1 + 1; i++)
    {
      float min = std::numeric_limits<float>::max();
      for (int j = 1; j < n2 + 1; j++)
      {
        float diff = 0.f;
        float penalty_j1 = 0.f;
        if (idf_weight)
          penalty_j1 = idf_penalty[j-1] * idf_weight;
        if (s1[i-1] != s2[j-1]) {
          diff = edit_costs.replace_cost * costs.diff_word + penalty_j1;
        }
        else if (real1tok[i-1] != real2tok[j-1]) {
          /* is difference only a case difference */
          if (strchr("LUMC", real1tok[i-1][0]))
            diff = edit_costs.replace_cost * costs.diff_case;
          else {
            diff = edit_costs.replace_cost * costs.diff_real;
          }
        }

        cost_tag[i][j] = _edit_distance_char(st1[i], sn1[i], st2[j], sn2[j]);
        const auto previous = {
          arr[i - 1][j] + edit_costs.delete_cost * costs.diff_word + cost_tag[i - 1][j],
          arr[i][j - 1] + edit_costs.insert_cost * costs.diff_word + cost_tag[i][j - 1] + penalty_j1,
          arr[i - 1][j - 1] + diff + cost_tag[i - 1][j - 1]
        };
        const auto min_it = std::min_element(std::begin(previous), std::end(previous));
        unsigned argmin = std::distance(std::begin(previous), min_it);
        const auto distance = *min_it;
        if (argmin == 2 && diff == 0.f)
          argmin = 3;
        traceback[i][j] = argmin; 

        arr[i][j] = distance;
        min = std::min(min, distance);
      }
      if (min > max_fuzzyness)
        return min;
    }
    int i = n1;
    int j = n2;
    while (i != 0 || j != 0)
      switch(traceback[i][j])
      {
        case 0:
          i--;
          break;
        case 1:
          j--;
          break;
        case 2:
          i--;
          j--;
          break;
        case 3:
          i--;
          j--;
          // std::cerr << "idf cover " << j << "  " << idf_penalty.size() << std::endl << std::flush;
          cover[j] = idf_cover ? idf_penalty[j] : 1.f;
          break;
        default:
          ;
      };

    return arr[n1][n2];
  }
}
