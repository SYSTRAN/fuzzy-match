#pragma once

#include <boost/multi_array.hpp>

namespace fuzzy
{
  inline int _edit_distance_char(const char *s1, int n1, const char *s2, int n2) {
    if (n1 == 0)
      return n2;
    if (n2 == 0)
      return n1;

    boost::multi_array<int, 2> arr(boost::extents[n1+1][n2+1]);

    arr[0][0] = 0;
    for (int i = 1; i < n1 + 1; i++)
      arr[i][0] = arr[i-1][0] + 1;
    for (int j = 1; j < n2 + 1; j++)
      arr[0][j] = arr[0][j-1] + 1;

    for (int i = 1; i < n1 + 1; i++)
    {
      for (int j = 1; j < n2 + 1; j++)
      {
        arr[i][j] = std::min(
          {
            arr[i - 1][j] + 1,
            arr[i][j - 1] + 1,
            arr[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)
          });
      }
    }

    return arr[n1][n2];
  }
}
