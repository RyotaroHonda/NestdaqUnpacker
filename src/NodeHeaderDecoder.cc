#include <sstream>
#include "NodeHeaderDecoder.hh"

std::string
nestdaq::unpacker::to_ipaddress( const uint64_t& address )
{
  std::ostringstream ss;
  for( int i = 3; i >= 0; --i ) {
    uint32_t val = ( address >> 8*i ) & 0xff;
    ss << std::dec << val << ".";
  }

  return ss.str();
}

void
nestdaq::unpacker::decode_header( const HeaderDef& header_def, const uint64_t* buffer, DecodedHeaderData& hdata )
{
  int index = 0;

  for( auto& word_def : header_def ) {
    for( auto& dd : word_def ) {
      hdata[dd.name] = ( ( buffer[index] >> dd.shift ) & dd.mask );
    }
    index++;
  }
};

void
nestdaq::unpacker::show_header( const HeaderDef& def, const DecodedHeaderData& data )
{
  for( auto& word_def : def ) {
    for( auto& dd : word_def ) {
      auto [name, value] = *data.find( dd.name );

      // OPTIMIZE: Rewrite here using std::format if the use of c++20 is allowed.
      printf( "%-20s 0x%016lx (%32lu)\n", name.c_str(), value, value );
    }
  }
}

void
nestdaq::unpacker::show_header_oneline( const HeaderDef& def, const DecodedHeaderData& data )
{
  auto type    = ( data.find( "Type" ) )->second;
  auto length  = ( data.find( "Length" ) )->second;
  auto itr     = data.find( "FEM ID" );

  std::string&& ipaddr{};
  if( itr == data.end() ) {
    ipaddr = "-";
  } else {
    ipaddr = to_ipaddress( itr->second );
  }

  // OPTIMIZE: Rewrite here using std::format if the use of c++20 is allowed.
  printf( "Type: %3lu, ID: %20s, Length: %lu (bytes)\n", type, ipaddr.c_str(), length );
}
