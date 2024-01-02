#include <fuzzy/fuzzy_match.hh>

#include <queue>
#include <vector>
#include <list>
#include <cmath>
#include <set>
#include <numeric>
#include <algorithm>

#include <unicode/normalizer2.h>
#include <fuzzy/suffix_array.hh>
#ifdef USE_EIGEN
  #include <fuzzy/bm25.hh>
  #include <fuzzy/bm25_matches.hh>
#endif
#include <fuzzy/no_filter.hh>
#include <fuzzy/no_matches.hh>
#include <fuzzy/costs.hh>
#include <fuzzy/ngram_matches.hh>
#include <fuzzy/edit_distance.hh>
#include <fuzzy/pattern_coverage.hh>

#include <onmt/Tokenizer.h>
#include <onmt/unicode/Unicode.h>
#include <boost/make_unique.hpp>

namespace fuzzy
{
  const char FuzzyMatch::version = '1';

  class CompareMatch
  {
  public:
    bool operator()(const FuzzyMatch::Match &x, const FuzzyMatch::Match &y)
    {
      return x.score < y.score || 
             (x.score == y.score && x.secondary_sort > y.secondary_sort);
    }
  };

  static std::string normalize(const std::string& text_utf8) {
    UErrorCode error_code = U_ZERO_ERROR;
    const auto* normalizer = icu::Normalizer2::getNFCInstance(error_code);
    if (U_FAILURE(error_code))
      throw std::runtime_error("Unable to get the ICU normalizer");

    const auto text = icu::UnicodeString::fromUTF8(text_utf8);
    const auto text_norm = normalizer->normalize(text, error_code);

    if (U_FAILURE(error_code))
      return text_utf8;

    std::string text_norm_utf8;
    text_norm.toUTF8String(text_norm_utf8);
    return text_norm_utf8;
  }


  FuzzyMatch::FuzzyMatch(int pt, size_t max_tokens_in_pattern, IndexType filter_type, const FilterIndexParams& params)
  : _pt(pt)
  , _filterIndex(boost::make_unique<FilterIndex>(max_tokens_in_pattern, filter_type, params))
  {
    _update_tokenizer();
  }

  FuzzyMatch::~FuzzyMatch() = default;

  void FuzzyMatch::_update_tokenizer() {
    int flags_tokenizer = onmt::Tokenizer::Flags::SegmentAlphabetChange
                          | onmt::Tokenizer::Flags::NoSubstitution
                          | onmt::Tokenizer::Flags::SupportPriorJoiners;
    if (_pt & pt_cas)
      flags_tokenizer |= onmt::Tokenizer::Flags::CaseFeature;
    if (_pt & pt_jnr)
      flags_tokenizer |= onmt::Tokenizer::Flags::JoinerNew
                         | onmt::Tokenizer::Flags::JoinerAnnotate;
    else if (_pt & pt_sep)
      flags_tokenizer |= onmt::Tokenizer::Flags::SpacerNew
                         | onmt::Tokenizer::Flags::SpacerAnnotate;

    _ptokenizer = boost::make_unique<decltype(_ptokenizer)::element_type>(onmt::Tokenizer::Mode::Aggressive, flags_tokenizer, "");
    /* segment on following alphabets */
    _ptokenizer->add_alphabet_to_segment("Han");
    _ptokenizer->add_alphabet_to_segment("Kanbun");
    _ptokenizer->add_alphabet_to_segment("Katakana");
    _ptokenizer->add_alphabet_to_segment("Hiragana");
  }

  void
  FuzzyMatch::_tokenize_and_normalize(const std::string &sentence,
                                      Sentence& real,
                                      Tokens& pattern) const {
    std::vector<std::string> tokens;
    std::vector<std::vector<std::string> > features;
    std::vector<unsigned> map_tokens;
    _tokenize_and_normalize(sentence,
                            real,
                            pattern,
                            map_tokens,
                            tokens,
                            features);
  }

  void
  FuzzyMatch::_tokenize_and_normalize(const std::string &sentence,
                                      Sentence& real,
                                      Tokens& pattern,
                                      std::vector<unsigned> &map_tokens,
                                      std::vector<std::string> &tokens,
                                      std::vector<std::vector<std::string>> &features) const {
    const std::string sentence_norm = normalize(sentence);
    _ptokenizer->tokenize(sentence_norm, tokens, features);

    real.reserve(tokens.size());
    pattern.reserve(tokens.size());

    map_tokens.push_back(0);

    for(size_t real_i = 0, i = 0; i < tokens.size(); i++)
    {
      const std::string &token = tokens[i];
      if (token == onmt::Tokenizer::spacer_marker || token == onmt::Tokenizer::joiner_marker) {
        real.set_itok(real_i, " ");
        continue;
      }
      /* for word tokens - keep only the case feature */
      if ((_pt & pt_cas) && features[0][i] != "N") {
        pattern.emplace_back(token);
        real.push_back(features[0][i]);
        real_i++;
        map_tokens.push_back(i+1);
      }
      else {
        if (onmt::Tokenizer::is_placeholder(token)) {
          size_t ph_begin = token.find(onmt::Tokenizer::ph_marker_open);
          static const std::string ph_id_sep("＃");
          static const std::string ph_value_sep("：");
          size_t ph_end = token.find(ph_id_sep, ph_begin);
          if (ph_end == std::string::npos) {
            ph_end = token.find(ph_value_sep, ph_begin);
            if (ph_end == std::string::npos) {
              ph_end = token.find(onmt::Tokenizer::ph_marker_close, ph_begin);
              if (ph_end == std::string::npos)
                ph_end = token.length();
            }
          }
          std::string ent = token.substr(ph_begin+onmt::Tokenizer::ph_marker_open.size(),
                                         ph_end-ph_begin-onmt::Tokenizer::ph_marker_open.size());
          if (ent.length()>=2 && ent.substr(0,2)=="it")
            ent = "it";
          if (ent == "it" && (_pt & pt_tag))
            real.set_itok(real_i, "T");
          else {
            pattern.emplace_back(onmt::Tokenizer::ph_marker_open+ent+onmt::Tokenizer::ph_marker_close);
            real.push_back(token);
            real_i++;
            map_tokens.push_back(i+1);
          }
        } else {
          unsigned int l;
          auto cp = onmt::unicode::utf8_to_cp((const unsigned char*)token.c_str(), l);
          if (onmt::unicode::is_number(cp)) {
            if (_pt & pt_nbr) {
              pattern.emplace_back(onmt::Tokenizer::ph_marker_open+"num"+onmt::Tokenizer::ph_marker_close);
              real.push_back(token);
              real_i++;
              map_tokens.push_back(i+1);
            } else {
              pattern.emplace_back(token);
              real.push_back(token);
              real_i++;
              map_tokens.push_back(i+1);
            }
          }
          else {
            if (!onmt::unicode::is_letter(cp) && _pt & pt_pct) {
              real.set_itok(real_i, token);
            }
            else {
              pattern.emplace_back(token);
              real.push_back(token);
              real_i++;
              map_tokens.push_back(i+1);
            }
          }
        }
      }
    }
  }

  /* backward compatibility */
  bool
  FuzzyMatch::add_tm(const std::string& id, const Tokens& norm, bool prepare)
  {
    const Sentence real(norm);
    _filterIndex->add_tm(id, real, norm, prepare);

    return true;
  }

  bool
  FuzzyMatch::add_tm(const std::string& id, const Sentence& source, const Tokens& norm, bool prepare)
  {
    _filterIndex->add_tm(id, source, norm, prepare);

    return true;
  }

  bool FuzzyMatch::add_tm(const std::string &id, const std::string &sentence, bool prepare)
  {
    Sentence real;
    Tokens norm;
    _tokenize_and_normalize(sentence, real, norm);
    if (norm.size()==0) {
      std::cerr<<"WARNING: cannot index empty segment: "<<sentence<<" ("<<id<<")"<<std::endl;
      return false;
    }
    add_tm(id, real, norm, prepare);
    return true;
  }

#ifndef NDEBUG
  std::ostream& FuzzyMatch::dump(std::ostream& os) const {
    return _filterIndex->dump(os);
  }
#endif

  void
  FuzzyMatch::prepare()
  {
    _filterIndex->prepare();
  }

  struct Subseq {
    float weight;
    size_t position;
    size_t length;

    bool operator<(const Subseq& other) const
    {
      return weight < other.weight || (weight == other.weight && position > other.position);
    }
  };

  /* subsequence operator */
  bool FuzzyMatch::subsequence(const std::string &sentence,
                         unsigned number_of_matches,
                         bool no_perfect,
                         std::vector<Match>& matches,
                         int min_subseq_length,
                         float min_subseq_ratio,
                         bool idf_weighting) const {

    Sentence real;
    Tokens pattern;
    std::vector<std::string> tokens;
    std::vector<std::vector<std::string>> features;
    std::vector<unsigned> map_tokens;

    _tokenize_and_normalize(sentence, real, pattern, map_tokens, tokens, features);

    size_t p_length = pattern.size();

    if ((int)(min_subseq_ratio*p_length) > min_subseq_length)
      min_subseq_length = min_subseq_ratio*p_length;

    if ((int)p_length < min_subseq_length)
      return false;

    FilterIndex& SAI = *_filterIndex;

    /* get vocab id once for all */
    std::vector<unsigned> pidx = SAI.get_VocabIndexer().getIndex(pattern);
    const std::vector<float> idf_penalty = compute_idf_penalty(pidx,
                                                               /*unknown_vocab_word_penalty=*/-1);

    /* sort the subsequences by idf weight */
    std::priority_queue<Subseq> subseq_queue;
    for(size_t it=0; it < p_length; it++) {
      float idf_weight = 0;
      for(size_t jt=it; jt < p_length; jt++) {
        float weight = idf_penalty[jt];
        if (weight == -1) break;
        idf_weight += idf_weighting?weight:1;
        if (int(jt-it+1) >= min_subseq_length)
          subseq_queue.emplace(Subseq{idf_weight, it, jt-it+1});
      }
    }

    int max_distance = 10000;
    Match best_match;

    std::set<unsigned> candidates;
    std::set<unsigned> perfect;

    std::vector<const char*> st(p_length+1);
    std::vector<int> sn(p_length+1);
    Tokens realtok = (Tokens)real;
    real.get_itoks(st, sn);

    while(!subseq_queue.empty() &&
          max_distance == 10000) {
      auto &subseq = subseq_queue.top();

      size_t current_min_suffixid = 0;
      size_t current_max_suffixid = 0;
      const Filter& filter = SAI.get_Filter();
      const SuffixArray& suffix_array = dynamic_cast<const SuffixArray&>(filter);
      std::pair<size_t, size_t> range_suffixid = suffix_array.equal_range(pidx.data() + subseq.position, subseq.length, current_min_suffixid, current_max_suffixid);

      for(auto suffixIt=range_suffixid.first; suffixIt < range_suffixid.second &&
                                            candidates.size()<number_of_matches; suffixIt++) {
        size_t s_id = suffix_array.get_suffix_view(suffixIt).sentence_id;
        if (candidates.find(s_id) == candidates.end() &&
            perfect.find(s_id) == perfect.end()) {
          size_t s_length = 0;
          const auto* thes = SAI.get_Filter().get_sentence(s_id, &s_length);

          const EditCosts edit_costs;
          const Costs costs(p_length, s_length, edit_costs);

          /* let us calculate edit_distance  */
          float cost = _edit_distance(thes, SAI.real_tokens(s_id), s_length,
                                      pidx.data(), realtok, p_length,
                                      st, sn,
                                      idf_penalty, 0, 
                                      edit_costs,
                                      costs, max_distance);
          if (cost==0 && no_perfect) {
            perfect.insert(s_id);
            continue;
          }
          if (cost < max_distance) {
            Match m;
            best_match.score = int(10000-cost*100)/10000.0;
            best_match.max_subseq = subseq.length;
            best_match.s_id = s_id;
            best_match.secondary_sort = s_id;
            best_match.id = SAI.id(s_id);
            unsigned org_it = map_tokens[subseq.position];
            unsigned org_jt = map_tokens[subseq.position + subseq.length];
            std::vector<std::string> tokens_subseq(tokens.begin()+org_it, tokens.begin()+org_jt);
            std::vector<std::vector<std::string> > features_subseq;
            if (features.size()) {
              features_subseq.push_back(std::vector<std::string>(features[0].begin()+org_it, features[0].begin()+org_jt));
            }
            best_match.id += "\t" + _ptokenizer->detokenize(tokens_subseq, features_subseq);

            max_distance = cost;
            if (cost == 0) break;
          }
          candidates.insert(s_id);
        }
      }

      subseq_queue.pop();
    }

    if (max_distance != 10000) {
      matches.push_back(best_match);
      return true;
    }
    return false;
  }

  float FuzzyMatch::compute_max_idf_penalty() const {
    const unsigned num_sentences = _filterIndex->get_Filter().num_sentences();
    return std::log(num_sentences);
  }

  std::vector<float> FuzzyMatch::compute_idf_penalty(const std::vector<unsigned>& pattern_wids,
                                                     float unknown_vocab_word_penalty) const {
    std::vector<float> idf_penalty;
    idf_penalty.reserve(pattern_wids.size());

    const unsigned num_sentences = _filterIndex->get_Filter().num_sentences();

    const std::vector<unsigned>& word_frequency_in_sentences = _filterIndex->get_VocabIndexer().getSFreq();

    for (const auto wid : pattern_wids) {
      // https://en.wikipedia.org/wiki/TF-IDF
      if (wid != fuzzy::VocabIndexer::VOCAB_UNK)
        idf_penalty.push_back(std::log((float)num_sentences/(float)word_frequency_in_sentences[wid]));
      else
        idf_penalty.push_back(unknown_vocab_word_penalty);
    }

    return idf_penalty;
  }

  /* interface with integrated tokenization */
  bool FuzzyMatch::match(const std::string &sentence,
                         float fuzzy,
                         unsigned number_of_matches,
                         bool no_perfect,
                         std::vector<Match>& matches,
                         int min_subseq_length,
                         float min_subseq_ratio,
                         float vocab_idf_penalty,
                         const EditCosts& edit_costs,
                         float contrastive_factor,
                         ContrastReduce reduce,
                         int contrast_buffer,
                         IndexType filter_type,
                         int bm25_buffer,
                         float bm25_cutoff) const {

    Sentence real;
    Tokens norm;
    _tokenize_and_normalize(sentence, real, norm);
    return match(real, norm, fuzzy, number_of_matches, no_perfect, matches,
                 min_subseq_length, min_subseq_ratio, vocab_idf_penalty,
                 edit_costs, contrastive_factor, reduce, contrast_buffer,
                 filter_type, bm25_buffer, bm25_cutoff);
  }

  /* backward compatibility */
  bool
  FuzzyMatch::match(const Tokens& pattern,
                    float fuzzy,
                    unsigned number_of_matches,
                    std::vector<Match>& matches,
                    int min_subseq_length,
                    float min_subseq_ratio,
                    float vocab_idf_penalty,
                    const EditCosts& edit_costs,
                    float contrastive_factor,
                    ContrastReduce reduce,
                    int contrast_buffer,
                    IndexType filter_type,
                    int bm25_buffer,
                    float bm25_cutoff) const
  {
    const Sentence real(pattern);
    return match(real, pattern, fuzzy, number_of_matches, false, matches,
                 min_subseq_length, min_subseq_ratio, vocab_idf_penalty,
                 edit_costs, contrastive_factor, reduce, contrast_buffer,
                 filter_type, bm25_buffer, bm25_cutoff);
  }

  /* check for the pattern in the suffix-array index SAI */ 
  bool
  FuzzyMatch::match(const Sentence& real,
                    const Tokens& pattern,
                    float fuzzy,
                    unsigned number_of_matches,
                    bool no_perfect,
                    std::vector<Match>& matches,
                    int min_subseq_length,
                    float min_subseq_ratio,
                    float vocab_idf_penalty,
                    const EditCosts& edit_costs,
                    float contrastive_factor,
                    ContrastReduce reduce,
                    int contrast_buffer,
                    IndexType filter_type,
                    int bm25_buffer,
                    float bm25_cutoff) const
  {
    size_t p_length = pattern.size();
    if (contrast_buffer == -1)
      contrast_buffer = number_of_matches;

    // performance guard
    if (p_length > max_tokens_in_pattern())
    {
      return false; // no matches
    }

    if (!p_length)
      return false;

    if ((std::size_t)(min_subseq_length) > pattern.size())
      min_subseq_length = pattern.size();

    if ((int)(min_subseq_ratio*p_length) > min_subseq_length)
      min_subseq_length = min_subseq_ratio*p_length;
    /* get vocab id once for all */
    const auto pattern_wids = _filterIndex->get_VocabIndexer().getIndex(pattern);

    float idf_max = 0.01;
    std::vector<float> idf_penalty;
    if (vocab_idf_penalty) {
      idf_penalty = compute_idf_penalty(pattern_wids);
      idf_max = compute_max_idf_penalty();
    }

    /* result map - normalized error => sentence */
    std::priority_queue<Match, std::vector<Match>, CompareMatch> result;

    const Filter& filter = _filterIndex->get_Filter();
    // FilterMatches* filter_matches = nullptr;
    // std::unique_ptr<FilterMatches> filter_matches;
    std::shared_ptr<FilterMatches> filter_matches;
    if (filter_type == IndexType::SUFFIX) {
      const SuffixArray& suffix_array = static_cast<const SuffixArray&>(filter);
      // filter_matches = new NGramMatches(fuzzy, p_length, min_subseq_length, suffix_array);
      auto nGramMatches = std::make_shared<NGramMatches>(fuzzy, p_length, min_subseq_length, suffix_array);
      filter_matches = std::shared_ptr<FilterMatches>(nGramMatches, &*nGramMatches);
      // NGramMatches& nGramMatches = static_cast<NGramMatches&>(*filter_matches);

      if (p_length == 1)
      {
        std::pair<size_t, size_t> range_suffixid = suffix_array.equal_range(pattern_wids.data(), p_length);

        if (range_suffixid.first != range_suffixid.second)
          nGramMatches->register_suffix_range_match(range_suffixid.first,
                                                  range_suffixid.second,
                                                  p_length,
                                                  edit_costs);
      }

      for (size_t it=0; it < p_length; it++)
      {
        std::pair<size_t, size_t> previous_range_suffixid(0, 0);
        size_t subseq_length = 0;

        for (size_t jt = it; jt < p_length; jt++)
        {
          ++subseq_length;
          /*
            the set of solution will be a decreasing range
            pos-i     ngram
            pos-i+1   ngram
            pos-i+2   ngram   (n+1)gram
            pos-i+3   ngram   (n+1)gram   (n+2)gram
            pos-i+4   ngram   (n+1)gram
            pos-i+5   ngram

            and we will only keep the matches:
            pos-i     ngram
            pos-i+1   ngram
            pos-i+2   (n+1)gram
            pos-i+3   (n+2)gram
            pos-i+4   (n+1)gram
            pos-i+5   ngram
          */
          std::pair<size_t, size_t> range_suffixid = suffix_array.equal_range(pattern_wids.data() + it, subseq_length, previous_range_suffixid.first, previous_range_suffixid.second);


          if (range_suffixid.first != range_suffixid.second)
          {
            /* do not register unigrams - yet */
            if (subseq_length >= 2)
            {
              /* register (n-1) grams */
              nGramMatches->register_suffix_range_match(previous_range_suffixid.first,
                                                      range_suffixid.first,
                                                      subseq_length - 1,
                                                      edit_costs);
              nGramMatches->register_suffix_range_match(range_suffixid.second,
                                                      previous_range_suffixid.second,
                                                      subseq_length - 1,
                                                      edit_costs);
            }

            previous_range_suffixid = std::move(range_suffixid);
          }
          else
          {
            --subseq_length;
            break;
          }
        }
        if (subseq_length >= 1)
          nGramMatches->register_suffix_range_match(previous_range_suffixid.first,
                                                  previous_range_suffixid.second,
                                                  subseq_length,
                                                  edit_costs);
      }
    }
#ifdef USE_EIGEN
    else if (filter_type == IndexType::BM25)
    {
      const BM25& bm25 = static_cast<const BM25&>(filter);
      filter_matches = std::make_shared<BM25Matches>(fuzzy, p_length, min_subseq_length, bm25, bm25_buffer, bm25_cutoff);
      // filter_matches = new BM25Matches(fuzzy, p_length, min_subseq_length, bm25, bm25_buffer, bm25_cutoff);
      BM25Matches& bm25Matches = static_cast<BM25Matches&>(*filter_matches);
      bm25Matches.register_pattern(pattern_wids, edit_costs);
    }
#endif
    else if (filter_type == IndexType::NO)
    {
      const NoFilter& no_filter = static_cast<const NoFilter&>(filter);
      filter_matches = std::make_shared<NoMatches>(fuzzy, p_length, min_subseq_length, no_filter);
      NoMatches& no_matches = static_cast<NoMatches&>(*filter_matches);
      no_matches.load_all();
    }
    /* Consolidation of the results */

    /* now explore for the best segments */

    PatternCoverage pattern_coverage(pattern_wids);
    std::vector<const char*> st(p_length+1);
    std::vector<int> sn(p_length+1);
    Tokens pattern_realtok = (Tokens)real;

    real.get_itoks(st, sn);

    // We track the lowest costs in order the call the edit distance with an upper bound
    // and possibly return earlier. The default upper bound is FLT_MAX (i.e. no restriction).
    // The restriction will only start when we pop this value from the heap.
    std::priority_queue<float> lowest_costs;
    lowest_costs.push(std::numeric_limits<float>::max());

    unsigned cpt = 0;
    // unsigned num_filtered = 0;

    // ONLY N-grams
    // for (const auto& pair : filter_matches->get_best_matches())
    // {
    //   const auto s_id = pair.first;
    //   const auto longest_match = pair.second;
    //   size_t s_length = 0;
    //   const auto* sentence_wids = _filterIndex->get_Filter().get_sentence(s_id, &s_length);
    //   Match m(sentence_wids, s_length);
    //   m.score = (float)longest_match / (float)s_length;
    //   m.max_subseq = longest_match;
    //   m.s_id = s_id;
    //   m.id = _filterIndex->id(s_id);
    //   m.secondary_sort = s_id;
    //   m.penalty = 0;
    //   result.push(m);
    //   cpt++;
    //   if (cpt > contrast_buffer)
    //     break;
    // }

    // ONLY BM25
    // for (const auto& pair : filter_matches->get_best_matches())
    // {
    //   const auto s_id = pair.first;
    //   const auto bm25_score = pair.second;
    //   size_t s_length = 0;
    //   const auto* sentence_wids = _filterIndex->get_Filter().get_sentence(s_id, &s_length);
    //   Match m(sentence_wids, s_length);
    //   m.score = (float)bm25_score / (float)1000.;
    //   m.max_subseq = 0;
    //   m.s_id = s_id;
    //   m.id = _filterIndex->id(s_id);
    //   m.secondary_sort = s_id;
    //   m.penalty = 0;
    //   result.push(m);
    //   cpt++;
    //   if (cpt > contrast_buffer)
    //     break;
    // }


    for (const auto& pair : filter_matches->get_best_matches())
    {
      // num_filtered++;
      const auto s_id = pair.first;
      const auto longest_match = pair.second;
      size_t s_length = 0;
      const auto* sentence_wids = _filterIndex->get_Filter().get_sentence(s_id, &s_length);
      const auto num_covered_words = (longest_match < p_length
                                      ? pattern_coverage.count_covered_words(sentence_wids, s_length)
                                      : p_length);
      /* do not care checking sentences that do not have enough ngram matches for the fuzzy threshold */
      // if (!filter_matches->theoretical_rejection_cover(p_length, s_length, num_covered_words, edit_costs))
      // {
        const Costs costs(p_length, s_length, edit_costs);

        /* let us check the candidates */
        const auto sentence_realtok = _filterIndex->real_tokens(s_id);
        const auto cost_upper_bound = lowest_costs.top();
        float cost = _edit_distance(sentence_wids, sentence_realtok, s_length,
                                    pattern_wids.data(), pattern_realtok, p_length,
                                    st, sn,
                                    idf_penalty, costs.diff_word*vocab_idf_penalty/idf_max,
                                    edit_costs,
                                    costs, cost_upper_bound);
        // float cost = 0.1;
        if ((no_perfect && cost == 0 && (s_length == p_length)) || cost > cost_upper_bound)
          continue;

        float score = int(10000-cost*100)/10000.0;


        lowest_costs.push(cost);
        if (score < fuzzy || (contrast_buffer > 0 && (int)lowest_costs.size() > contrast_buffer))
          lowest_costs.pop();
        if (score >= fuzzy) {
          Match m(sentence_wids, s_length);
          m.score = score;
          m.max_subseq = longest_match;
          m.s_id = s_id;
          m.id = _filterIndex->id(s_id);
          m.secondary_sort = (filter_type == IndexType::SUFFIX) ? s_id : cpt;
          m.penalty = 0;
          result.push(m);
          cpt++;
        }
      // }
    }
    // COUT filter
    // std::cerr << num_filtered << std::endl;
    // std::cerr << filter_matches->get_best_matches().size() << std::endl;
    // delete filter_matches;
    /* Contrastive reranking */
    if (contrastive_factor > 0)
    {
      std::list<Match> candidates;
      while (!result.empty())
      {
        auto match = result.top();
        candidates.push_back(match);
        result.pop();
      }
      auto comp = [contrastive_factor](const Match& m1, const Match& m2) {
          return (m1.score - contrastive_factor * m1.penalty) < (m2.score - contrastive_factor * m2.penalty);
      };
      /* for memoization optimization */
      std::unordered_map<std::pair<int, int>, float, PairHasher> edit_cost_memory;
      while (!candidates.empty() && (number_of_matches == 0 || matches.size() < number_of_matches))
      {
        EditCosts internal_edit_cost;
        // rescore penalties of candidates
        for (Match &match : candidates)
        {
          std::vector<float> penalties;
          for (Match &match_memory : matches)
          {
            auto it = edit_cost_memory.find({match.s_id, match_memory.s_id});
            float penalty;
            if (it != edit_cost_memory.end()) {
              penalty = it->second;
            } else {
              const Costs costs(match.length, match_memory.length, internal_edit_cost);
              penalty = _edit_distance(
                match.s, match.length,
                match_memory.s, match_memory.length,
                internal_edit_cost,
                costs
              );
              edit_cost_memory.insert({{match.s_id, match_memory.s_id}, penalty});
            }
            penalty = int(10000 - penalty * 100) / 10000.0;
            penalties.push_back(penalty);
          }
          if (!penalties.empty())
          {
            if (reduce == ContrastReduce::MAX)
            { // max
              match.penalty = *std::max_element(penalties.cbegin(), penalties.cend());
            } else
            { // mean
              match.penalty = std::accumulate(penalties.cbegin(), penalties.cend(), 0.f) / penalties.size();
            }
          }
        }
        auto it_max = std::max_element(candidates.begin(), candidates.end(), comp);
        matches.push_back(*it_max);
        candidates.erase(it_max);
      }
    }
    else
    {
      while (!result.empty() && (number_of_matches == 0 || matches.size() < number_of_matches))
      {
        auto match = result.top();
        matches.push_back(match);

        result.pop();
      }
    }
    return matches.size() > 0;
  }
}

