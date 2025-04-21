#ifndef HEADER_DECODER_HH
#define HEADER_DECODER_HH

#include <unordered_map>
#include "NodeHeaderSchema.hh"

namespace nestdaq::unpacker {

  using DecodedHeaderData = std::unordered_map<std::string, uint64_t>;

  auto to_ipaddress( const uint64_t& address ) -> std::string;

  void decode_header( const HeaderDef& header_def, const uint64_t* buffer, DecodedHeaderData& hdata );
  void show_header( const HeaderDef& def, const DecodedHeaderData& data );
  void show_header_oneline( const DecodedHeaderData& data );

};// namespace nestdaq::unpacker

#endif
