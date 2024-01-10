#pragma once

#include <vector>
#include <iostream>
#include <memory>


namespace fuzzy
{
   void get_bow_score(
    std::vector<unsigned>& sorted_pattern_terms,
    std::vector<unsigned>& count_terms,
    const unsigned* sentence,
    const unsigned sentence_length,
    float& score,
    std::vector<float>& cover);

   void get_bow_score_idf(
    std::vector<unsigned>& sorted_pattern_terms,
    std::vector<unsigned>& count_terms,
    const unsigned* sentence,
    const unsigned sentence_length,
    std::vector<float>& idf_penalty,
    float& score,
    std::vector<float>& cover);
}
