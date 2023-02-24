#include <stdexcept>

namespace fuzzy
{
  inline unsigned
  BM25::get_sentence_length(size_t s_id) const
  {
    if (s_id + 1 == _sentence_pos.size())
      return _sentence_buffer.size() - _sentence_pos[s_id] -  2;
    return _sentence_pos[s_id + 1] - _sentence_pos[s_id] - 2;
  }
  template<class Archive>
  void BM25::save(Archive& archive, unsigned int) const
  {
    std::cerr << "saving...";
    unsigned num_sentences = _bm25->shape()[1];
    unsigned vocab_size = _bm25->shape()[0];
    auto _bm25_arr = _bm25->data();
    archive
    & num_sentences
    & vocab_size
    & boost::serialization::make_array(_bm25_arr, _bm25->num_elements())
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
    std::cerr << " done" << std::endl;
    // std::cerr << "saved elements:" << std::endl; 
    // for (unsigned i = 0; i < _bm25->num_elements(); i++)
    //   std::cerr << _bm25_arr[i] << "\t";
    // std::cerr << std::endl;
  }

  template<class Archive>
  void BM25::load(Archive& archive, unsigned int version)
  {
    std::cerr << "loading...";
    unsigned num_sentences;
    unsigned vocab_size;
    archive
    & num_sentences
    & vocab_size;
    _bm25 = new boost::multi_array<float, 2>(boost::extents[vocab_size][num_sentences]);
    auto _bm25_arr = _bm25->data();
    archive
    & boost::serialization::make_array(_bm25_arr, _bm25->num_elements());
    archive
    & _sentence_buffer
    & _sentence_pos
    & _quickVocabAccess;
    _sorted = true;
    std::cerr << " done" << std::endl;
    // for (unsigned i = 0; i < _bm25->num_elements(); i++)
    //   std::cerr << _bm25_arr[i] << "\t";
    // std::cerr << std::endl;
    // std::cerr << "dim = " << _bm25->num_dimensions() << std::endl;
    // std::cerr << "shape 1 = " << _bm25->shape()[0] << std::endl;
    // std::cerr << "shape 2 = " << _bm25->shape()[1] << std::endl;
    for (unsigned s_id = 0; s_id < num_sentences; s_id++)
      for (unsigned term = 0; term < vocab_size; term++)
        std::cerr << (*_bm25)[term][s_id] << "\t";
    std::cerr << std::endl;
  }
}
