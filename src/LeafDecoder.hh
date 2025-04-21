#ifndef LEAF_DECODER_HH
#define LEAF_DECODER_HH

//#include <unordered_map>
#include "LeafSchema.hh"

namespace nestdaq::unpacker {

  using LeafData     = std::unordered_map<std::string, uint32_t>;
  using LeafDataSet  = std::tuple<std::string, LeafData>;

  auto decode_fee_word( const uint64_t* buffer, const LeafSchemaPage& page ) -> LeafDataSet;
  auto decode_hbdflag(const uint32_t& flags) -> LeafData;
  void show_decoded_feedata( const LeafDataSet& decoded_dataset );
  void show_hbdflag( const LeafData& decoded_flag );

};// namespace nestdaq::unpacker

#endif
