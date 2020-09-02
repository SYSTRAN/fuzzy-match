#include <fuzzy/fuzzy_matcher_binarization.hh>

#include <fstream>

#include <boost/iostreams/filtering_streambuf.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/current_function.hpp>

#include <stdexcept>

namespace fuzzy
{
  void
  export_binarized_fuzzy_matcher(const std::string& binarized_tm_filename, const FuzzyMatch& fuzzy_matcher)
  {
    std::ofstream ofs;
    ofs.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    ofs.open(binarized_tm_filename.c_str(), std::ios_base::binary);

    ofs.write("FMI", 3).write(&fuzzy_matcher.version, 1);

    boost::iostreams::filtering_ostreambuf fos;
    fos.push(ofs);

    boost::archive::binary_oarchive archive(fos);
    archive << fuzzy_matcher;
  }

  void
  import_binarized_fuzzy_matcher(const std::string& binarized_tm_filename, FuzzyMatch& fuzzy_matcher)
  {
    std::ifstream ifs;
    ifs.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    ifs.open(binarized_tm_filename.c_str());

    char buffer[4];
    ifs.read(buffer, 4);
    if (!strcmp(buffer, "FMI")) {
      throw std::runtime_error(std::string(BOOST_CURRENT_FUNCTION) + "invalid FMI file format");
    }

    boost::iostreams::filtering_istreambuf fis;
    fis.push(ifs);

    boost::archive::binary_iarchive archive(fis);
    archive >> fuzzy_matcher;
    fuzzy_matcher._update_tokenizer();
  }
}
