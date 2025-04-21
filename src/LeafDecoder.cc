#include <iostream>
#include <sstream>
#include <string>
#include "LeafDecoder.hh"

namespace
{
  auto
    decode(const uint64_t& word, const std::vector<nestdaq::LeafDecodeDef> word_def)
    -> nestdaq::unpacker::LeafData
  {
    nestdaq::unpacker::LeafData decoded_data;
    for( auto data_def : word_def ) {
      decoded_data[data_def.name] = static_cast< uint32_t >( ( word >> data_def.shift ) & data_def.mask );
    }

    return decoded_data;
  };
};

// _______________________________________________________________________
auto
nestdaq::unpacker::decode_fee_word( const uint64_t* buffer, const LeafSchemaPage& page )
-> LeafDataSet
{
  auto& [fee_name, type_decode, fee_def] = page;
  auto data_type = ( ( buffer[0] >> type_decode.shift ) & type_decode.mask );

  auto itr = fee_def.find( data_type );
  if( itr == fee_def.end() ) {
    LeafData empty_data;
    return { "null", empty_data };
  }

  auto [type_name, word_def] = itr->second;
  LeafData&& decoded_data = decode( buffer[0], word_def );

  return { type_name, decoded_data };
};

// _______________________________________________________________________
auto
nestdaq::unpacker::decode_hbdflag( const uint32_t& flags )
-> LeafData
{
  return decode( flags, nestdaq::amaneq::g_hbd_flag_def );
}

// _______________________________________________________________________
void
nestdaq::unpacker::show_decoded_feedata( const LeafDataSet& decoded_dataset )
{

  auto& [type_name, decoded_data] = decoded_dataset;

  std::cout << type_name << ": ";

  for( auto data : decoded_data ) {
    std::cout << data.first << ": " << data.second << ", ";
  }
  std::cout << std::endl;
}

// _______________________________________________________________________
void
nestdaq::unpacker::show_hbdflag( const LeafData& flags )
{
  for( auto& [name, value] : flags ) {
    // OPTIMIZE: Rewrite here using std::format if use of c++20 is allowed.
    printf( "%-20s (%u)\n", name.c_str(), value );
  }
}
