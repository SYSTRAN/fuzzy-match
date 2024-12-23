
namespace fuzzy
{
    inline
    std::vector<NGram> get_sorted_ngrams(
        const unsigned N_const,
        const unsigned* sentence,
        const unsigned sentence_length)
    {
        std::vector<NGram> all_ngrams;
        const unsigned N = std::min(N_const, sentence_length);
        all_ngrams.reserve(N * sentence_length - N * (N - 1) / 2);
        for (unsigned n = 1; n <= N; n++)
            for (unsigned i = 0; i < sentence_length - n + 1; i++)
                all_ngrams.push_back(NGram(sentence + i, n));
        std::sort(all_ngrams.begin(), all_ngrams.end());
        return all_ngrams;
    }

    template <typename T>
    inline
    void get_score(
        std::vector<T>& sorted_pattern_terms,
        std::vector<T>& sorted_sentence_terms,
        std::vector<unsigned>& count_terms,
        float& score,
        std::vector<float>& cover,
        std::vector<float>& idf_penalty)
    {
        cover = std::vector<float>(sorted_pattern_terms.size(), 0.f);
        // std::cerr << sorted_pattern_terms.size() << "|"
        //           << sorted_sentence_terms.size() << ">" 
        //           << std::flush;
        score = 0.f;
        for (
            unsigned i = 0, j = 0;
            (i < sorted_pattern_terms.size()) && (j < sorted_sentence_terms.size());
            j++)
        {
            while (
                (i < sorted_pattern_terms.size() - 1) && 
                (sorted_pattern_terms[i] < sorted_sentence_terms[j]))
                i++;

            if (sorted_pattern_terms[i] == sorted_sentence_terms[j])
                if (idf_penalty.size() > 0)
                {
                    if ((float)count_terms[i] > cover[i] / idf_penalty[i] + 1e-6f)
                    {
                        cover[i] += idf_penalty[i];
                        score += idf_penalty[i];
                    }
                }
                else if ((float)count_terms[i] > cover[i] + 1e-6f)
                {
                    cover[i] += 1.f;
                    score += 1.f;
                }
        }
    }

    template <typename T>
    inline
    void get_unique_with_count(
        std::vector<T>& sorted_salient,
        std::vector<T>& unique,
        std::vector<unsigned>& count)
    {
        unique.reserve(sorted_salient.size());
        count.reserve(sorted_salient.size());
        if (sorted_salient.size() > 0)
        {
          T& current_salient = sorted_salient[0];
          unsigned current_count = 1;
          for (unsigned i = 1; i < sorted_salient.size(); i++)
          {
            if (!(current_salient == sorted_salient[i]))
            {
              unique.push_back(current_salient);
              count.push_back(current_count);
              current_salient = sorted_salient[i];
              current_count = 1;
            }
            else
              current_count++;
          }
          unique.push_back(current_salient);
          count.push_back(current_count);
        }
    }
}