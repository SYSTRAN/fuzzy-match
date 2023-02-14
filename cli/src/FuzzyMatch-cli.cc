#include <string>
#include <iostream>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string.hpp>

#include <chrono>
#include <ctime>

#include <fuzzy/costs.hh>
#include <fuzzy/fuzzy_match.hh>
#include <fuzzy/fuzzy_matcher_binarization.hh>

#define TICK(msg) {\
    auto current = std::chrono::system_clock::now();\
    int elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current-start_period).count();\
    int total_time = std::chrono::duration_cast<std::chrono::milliseconds>(current-start).count();\
    std::cerr<<"STEP\t"<<msg<<"\tELAPSE\t"<<elapsed_time/1000.<<"\tTOTAL\t"<<total_time/1000.<<std::endl;\
    start_period = current;\
}

namespace po = boost::program_options;
namespace ios = boost::iostreams;

bool import_tm(fuzzy::FuzzyMatch& fuzzyMatcher, std::string tmFile, bool addTarget, bool add_target_no_index)
{
  std::istream *ofs = 0;
  size_t pos = tmFile.find(",");
  if (pos != std::string::npos) {
    std::string outFile = tmFile.substr(pos+1);
    ios::filtering_streambuf<ios::input> *pout = new ios::filtering_streambuf<ios::input>;
    std::ifstream *file = new std::ifstream(outFile, std::ios_base::in | std::ios_base::binary);
    if (boost::algorithm::ends_with(outFile, ".gz"))
      pout->push(ios::gzip_decompressor());
    pout->push(*file);
    ofs = new std::istream(pout);
    tmFile.erase(pos);
  }
  ios::filtering_streambuf<ios::input> in;
  std::ifstream file(tmFile, std::ios_base::in | std::ios_base::binary);
  if (boost::algorithm::ends_with(tmFile, ".gz"))
    in.push(ios::gzip_decompressor());
  in.push(file);
  std::istream ifs(&in);
  std::string srcLine;

  int count = 0;

  while (getline(ifs, srcLine))
  {
    std::string tgtLine;
    if (ofs)
      getline(*ofs, tgtLine);
    else {
      size_t pos = srcLine.find("\t");
      if (pos!=std::string::npos) {
        tgtLine = srcLine.substr(pos+1);
        srcLine.erase(pos);
      }
    }
    count += 1;
    std::string index = boost::lexical_cast<std::string>(count);
    if (addTarget)
      index += "="+tgtLine;
    if (add_target_no_index)
      index = tgtLine;
    fuzzyMatcher.add_tm(index, srcLine, /* sort */ false);
  }

  delete ofs;
  return true;
}


template <typename Function>
void work_loop(const Function& function,
               std::queue<std::pair<std::promise<std::string>, std::string>>& queue,
               std::mutex& mutex,
               std::condition_variable& cv,
               const bool& end_requested)
{
  while (true)
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&queue, &end_requested]{
      return !queue.empty() || end_requested;
    });

    if (end_requested)
    {
      lock.unlock();
      break;
    }

    auto work = std::move(queue.front());
    queue.pop();
    lock.unlock();

    auto& promise = work.first;
    auto& text = work.second;
    promise.set_value(function(text));
  }
}

template <typename Function>
std::pair<int, int> process_stream(const Function& function,
                                   std::istream& in,
                                   std::ostream& out,
                                   size_t num_threads,
                                   size_t buffer_size)
{
  std::string line;
  int count_nonempty = 0;
  int count_total = 0;

  if (num_threads <= 1) // Fast path for sequential processing.
  {
    while (std::getline(in, line)) {
      std::string res = function(line);
      if (!res.empty())
        count_nonempty++;
      out << res << std::endl;
    }
    return std::make_pair(count_nonempty, count_total);
  }

  std::queue<std::pair<std::promise<std::string>, std::string>> queue;
  std::mutex mutex;
  std::condition_variable cv;
  bool request_end = false;

  std::vector<std::thread> workers;
  workers.reserve(num_threads);
  for (size_t i = 0; i < num_threads; ++i)
    workers.emplace_back(&work_loop<Function>,
                         std::cref(function),
                         std::ref(queue),
                         std::ref(mutex),
                         std::ref(cv),
                         std::cref(request_end));

  std::queue<std::future<std::string>> futures;

  auto pop_results = [&futures, &out, &count_nonempty](bool blocking) {
    static const auto zero_sec = std::chrono::seconds(0);
    while (!futures.empty()
           && (blocking
               || futures.front().wait_for(zero_sec) == std::future_status::ready)) {
      std::string res = futures.front().get();
      if (!res.empty())
        count_nonempty++;
      out << res << std::endl;
      futures.pop();
    }
  };

  while (std::getline(in, line))
  {
    count_total++;
    {
      std::lock_guard<std::mutex> lock(mutex);
      queue.emplace(std::piecewise_construct,
                    std::forward_as_tuple(),
                    std::forward_as_tuple(std::move(line)));
      futures.emplace(queue.back().first.get_future());
    }

    cv.notify_one();
    if (futures.size() >= buffer_size)
      pop_results(/*blocking=*/false);
  }

  if (!futures.empty())
    pop_results(/*blocking=*/true);

  {
    std::lock_guard<std::mutex> lock(mutex);
    request_end = true;
  }

  cv.notify_all();
  for (auto& worker : workers)
    worker.join();

  return std::make_pair(count_nonempty, count_total);
}

class processor {
public:
  processor(int pt, float fuzzy, int nmatch, bool no_perfect,
            int min_subseq_length, float min_subseq_ratio,
            float idf_penalty, bool subseq_idf_weighting,
            size_t max_tokens_in_pattern, fuzzy::EditCosts edit_cost):
             _fuzzyMatcher(pt, max_tokens_in_pattern),
             _fuzzy(fuzzy),
             _nmatch(nmatch),
             _no_perfect(no_perfect),
             _min_subseq_length(min_subseq_length),
             _min_subseq_ratio(min_subseq_ratio),
             _idf_penalty(idf_penalty),
             _subseq_idf_weighting(subseq_idf_weighting),
             _cost(edit_cost) {}
  std::string match(const std::string &sentence) {
    std::vector<fuzzy::FuzzyMatch::Match> matches;

    _fuzzyMatcher.match(sentence, _fuzzy, _nmatch, _no_perfect, matches,
                        _min_subseq_length, _min_subseq_ratio, _idf_penalty, _cost);

    std::string   out;
    for(const fuzzy::FuzzyMatch::Match &m: matches) {
      std::string prefix;
      if (!out.empty()) prefix = "\t";
      out += prefix + boost::lexical_cast<std::string>(m.score) + "\t" + m.id;
    }
    return out;    
  }
  std::string subsequence(const std::string &sentence) {
    std::vector<fuzzy::FuzzyMatch::Match> matches;

    _fuzzyMatcher.subsequence(sentence, _nmatch, _no_perfect, matches,
                              _min_subseq_length, _min_subseq_ratio,
                              _subseq_idf_weighting);

    std::string   out;
    for(const fuzzy::FuzzyMatch::Match &m: matches) {
      std::string prefix;
      if (!out.empty()) prefix = "\t";
      out += prefix + boost::lexical_cast<std::string>(m.score);
      out += "\t" + boost::lexical_cast<std::string>(m.max_subseq);
      out += "\t" + m.id;
    }
    return out;    
  }
  std::pair<int, int>
  apply_stream(std::istream &in, std::ostream &out, size_t num_threads, size_t buffer_size, bool domatch) {
    if (domatch) {
      auto function_match = [this](const std::string& sentence) { 
#ifdef DEBUG
        std::cerr << "### new ###" << std::endl;
#endif
        return match(sentence);
      };
      return process_stream(function_match,
                     std::cin, std::cout, num_threads, 1000);
    } else {
      auto function_subsequence = [this](const std::string& sentence) { 
        return subsequence(sentence);
      };
      return process_stream(function_subsequence,
                     std::cin, std::cout, num_threads, 1000);      
    }
  }
  fuzzy::FuzzyMatch _fuzzyMatcher;
  std::mutex _tokenization_mutex;
private:
  float _fuzzy;
  int _nmatch;
  bool _no_perfect;
  int _min_subseq_length;
  float _min_subseq_ratio;
  float _idf_penalty;
  fuzzy::EditCosts _cost;
  bool _subseq_idf_weighting;
};

int main(int argc, char** argv)
{
  std::chrono::time_point<std::chrono::system_clock> start, start_period;
  start = std::chrono::system_clock::now();
  start_period = start;

  po::options_description cmdLineOptions("Command-line options");
  po::options_description configFileOptions;
  po::options_description fuzzyOptions("FuzzyMatch cli options");

  std::string configFile;
  cmdLineOptions.add_options()
    ("help,h", "This help message.")
    ("config", po::value(&configFile), "Path to the configuration file.")
    ;

  std::string action;
  std::string corpus;
  std::string index_file;
  std::string penalty_tokens;
  float idf_penalty;
  float insert_cost;
  float delete_cost;
  float replace_cost;
  float fuzzy;
  int nmatch;
  int nthreads;
  int min_subseq_length;
  float min_subseq_ratio;
  size_t max_tokens_in_pattern;
  fuzzyOptions.add_options()
    ("action,a", po::value(&action)->default_value("index"), "Action on the corpus (index|match|subseq"
#ifndef NDEBUG
                                                             "|dump"
#endif
      )
    ("index,i", po::value(&index_file), "index file")
    ("add-target", po::bool_switch(), "add target in the index")
    ("add-target-no-index", po::bool_switch(), "add target side with no index")
    ("corpus,c", po::value(&corpus), "Corpus file to index. Either bitext or 2 files comma-separated. Can be gzipped.")
    ("fuzzy,f", po::value(&fuzzy)->default_value(0.8), "fuzzy match threshold")
    ("ml", po::value(&min_subseq_length)->default_value(3), "minimal subsequence length")
    ("mr", po::value(&min_subseq_ratio)->default_value(0.3), "minimal subsequence ratio")    
    ("nmatch,n", po::value(&nmatch)->default_value(5), "number of fuzzy matches to return")
    ("no-perfect,P", po::bool_switch(), "discard perfect match")
    ("penalty-tokens,p", po::value(&penalty_tokens)->default_value("nbr,tag,cas"), "Tokens only considered as penalty and not included in the fuzzy match."
                                                                        "This option is only available when building index and applies for match."
                                                                        "Values: none (no such token) or a comma-separated list of `cas`, `nbr`, `tag`, "
                                                                        "`sep`/`jnr` and/or `pct`.")
    ("idf-penalty,I", po::value(&idf_penalty)->default_value(0), "if not 0, apply idf-penalty on missing tokens")
    ("insert-cost", po::value(&insert_cost)->default_value(1), "custom cost for insert in edit distance")
    ("delete-cost", po::value(&delete_cost)->default_value(1), "custom cost for delete in edit distance")
    ("replace-cost", po::value(&replace_cost)->default_value(1), "custom cost for replace in edit distance")
    ("subseq-idf-weighting,w", po::bool_switch(), "use idf weighting in finding longest subsequence")
    ("max-tokens-in-pattern", po::value(&max_tokens_in_pattern)->default_value(fuzzy::DEFAULT_MAX_TOKENS_IN_PATTERN), "Patterns containing more tokens than this value are ignored")
    ("nthreads,N", po::value(&nthreads)->default_value(4), "number of thread to use for match")
    ;

  configFileOptions
    .add(fuzzyOptions)
    ;
  cmdLineOptions.add(configFileOptions);

  std::vector<std::string> v_penalty_tokens;
  int pt = fuzzy::FuzzyMatch::pt_none;

  po::variables_map vm;

  try {
    // parse argv
    po::store(po::command_line_parser(argc, argv).options(cmdLineOptions).run(), vm);
    notify(vm);

    // parse config file
    if (vm.count("config"))
    {
      po::store(po::parse_config_file<char>(configFile.c_str(), configFileOptions), vm);
      po::notify(vm);
    }

    if (penalty_tokens == "")
      throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value,
                                                     "--penalty-tokens", "");
    if (penalty_tokens != "none") {
      boost::split(v_penalty_tokens, penalty_tokens, boost::is_any_of(","));
      for(auto &ptok: v_penalty_tokens) {
        int apt = fuzzy::FuzzyMatch::pt_none;
        if (ptok == "tag")
          apt = fuzzy::FuzzyMatch::pt_tag;
        else if (ptok == "sep")
          apt = fuzzy::FuzzyMatch::pt_sep;
        else if (ptok == "jnr")
          apt = fuzzy::FuzzyMatch::pt_jnr;
        else if (ptok == "nbr")
          apt = fuzzy::FuzzyMatch::pt_nbr;
        else if (ptok == "cas")
          apt = fuzzy::FuzzyMatch::pt_cas;
        else if (ptok == "pct") apt = fuzzy::FuzzyMatch::pt_pct;
        if (!apt || (pt & apt))
          throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value,
                                                         "--penalty-tokens", ptok);
        pt |= apt;
      }
      if ((pt & fuzzy::FuzzyMatch::pt_sep) && (pt & fuzzy::FuzzyMatch::pt_jnr))
          throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value,
                                                         "--penalty-tokens", "sep/jnr");        
    }
  } catch (boost::program_options::error &e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }

  bool add_target = vm["add-target"].as<bool>();
  bool add_target_no_index = vm["add-target-no-index"].as<bool>();
  bool no_perfect = vm["no-perfect"].as<bool>();
  bool subseq_idf_weighting = vm["subseq-idf-weighting"].as<bool>();

  if (vm.count("help"))
  {
    std::cout << cmdLineOptions << std::endl;
    return 0;
  }

  fuzzy::EditCosts edit_cost = fuzzy::EditCosts(insert_cost, delete_cost, replace_cost);
  processor O(pt, fuzzy, nmatch, no_perfect,
              min_subseq_length, min_subseq_ratio,
              idf_penalty, subseq_idf_weighting,
              max_tokens_in_pattern, edit_cost);

  if (index_file.length()) {
    TICK("Loading index_file: "+index_file);
    import_binarized_fuzzy_matcher(index_file, O._fuzzyMatcher);
  }
  else if (corpus.length())
  {
    TICK("Importing TM: "+corpus);
    bool ok = import_tm(O._fuzzyMatcher, corpus, add_target, add_target_no_index);
    if (! ok)
    {
      std::cerr << "ERROR: " << "import_tm failed";
      return 2;
    }

    TICK("Sorting Index");
    O._fuzzyMatcher.sort();

    // work
    if (action == "index")
    {
      size_t pos = corpus.find(",");
      if (pos != std::string::npos)
        corpus.erase(pos);
      std::string fuzzyMatchFile = corpus + ".fmi";
      TICK("Dump: "+fuzzyMatchFile);
      export_binarized_fuzzy_matcher(fuzzyMatchFile, O._fuzzyMatcher);
    }
  }
  else {
    std::cerr << "ERROR: " << "index file or corpus needs to be provided";
    return 3;
  }

  if (action == "match") {
    TICK("Matching");
    std::pair<int, int> res(O.apply_stream(std::cin, std::cout, nthreads, 1000, true));
    std::cerr<<"NMATCH\t"<<res.first<<"\t/\t"<<res.second<<std::endl;
  }
  else if (action == "subseq") {
    TICK("Subsequencing");
    std::pair<int, int> res(O.apply_stream(std::cin, std::cout, nthreads, 1000, false));
    std::cerr<<"NMATCH\t"<<res.first<<"\t/\t"<<res.second<<std::endl;
  }
#ifndef NDEBUG
  else if (action == "dump") {
    TICK("Dumping");
    O._fuzzyMatcher.dump(std::cout);
  }
#endif

  TICK("Done");

  return 0;
}
