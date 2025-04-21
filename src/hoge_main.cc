
#include<cstdio>
#include<ios>
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<cstring>
#include<stdint.h>

#include "NodeUnpacker.hh"
#include "LeafProcessor.hh"

using namespace nestdaq;

int main(int argc, char** argv)
{
  std::string path = argv[1];
  std::ifstream&& ifs{};

  ifs = std::ifstream(path.c_str(), std::ios::binary);
  if(!ifs.is_open()) return -1;

  std::vector<uint64_t> buffer(1);
  ifs.read(reinterpret_cast<char*>(&buffer[0]), sizeof(uint64_t));

  auto itr_schema_page = g_header_schema.find(buffer[0]);
  if(g_header_schema.end() == itr_schema_page){
    std::cout << "#E: Unkown data block" << std::endl;
    return -1;
  }

  auto [name, flags, hdef] = itr_schema_page->second;

  buffer.resize(hdef.size());
  ifs.read(reinterpret_cast<char*>(&buffer[1]), sizeof(uint64_t)*(hdef.size()-1));

  // Skip comment
  buffer.resize(32);
  ifs.read(reinterpret_cast<char*>(&buffer[0]), sizeof(uint64_t)*32);

  // End of FileSinkHeader //

  unpacker::LeafProcessor leaf_processor;
  for( int i = 0; i < 1; ++i ) {
    buffer.resize(32);
    ifs.read(reinterpret_cast<char*>(&buffer[0]), sizeof(uint64_t));
    itr_schema_page = g_header_schema.find(buffer[0]);
    if( g_header_schema.end() == itr_schema_page ) {
      std::cout << "#E: Unkown data block" << std::endl;
      std::cout << "#E: " << std::hex << buffer[0] << std::endl;
      return -1;
    }

    const auto [name, flags, hdef] = itr_schema_page->second;
    ifs.read(reinterpret_cast< char* >( &buffer[1] ), sizeof(uint64_t)*( hdef.size()-1 ));

    unpacker::DecodedHeaderData header;
    unpacker::decode_header(hdef, &buffer[0], header);
    uint64_t& length_in_byte  = header["Length"];
    uint64_t& hlength_in_byte = header["hLength"];
    buffer.resize(length_in_byte / sizeof(uint64_t));

    ifs.read( reinterpret_cast< char* >( &buffer[hlength_in_byte/sizeof( uint64_t )] ), length_in_byte-hlength_in_byte );

    auto unpacker = std::make_shared<unpacker::NodeUnpacker>(itr_schema_page->second);
    unpacker->set_data(&buffer[0]);
    unpacker->unpack();
//    unpacker->show_header( nestdaq::unpacker::show_header );
//    unpacker->show_block_structure( nestdaq::unpacker::show_header_oneline );

    leaf_processor.set_ctrl( unpacker::LeafProcessor::CtrlKeys::kImmediateFlush );
    leaf_processor.set_tf_type( header["Type"] );
    //    unpacker->extract_leafblocks();
    leaf_processor.build_time_aligned_block( unpacker->extract_leafnodes() );
    //    leaf_processor.show_built_block();

    std::cout << "num_frame " << leaf_processor.get_num_frame() << std::endl;

    for( uint32_t i = 0; i < leaf_processor.get_num_frame(); ++i ) {
      std::cout << "hoge" << std::endl;
      leaf_processor.set_frame_index( i );
      std::cout << "hoge1" << std::endl;
      leaf_processor.decode_heartbeat_delimiter();
      std::cout << "hoge2" << std::endl;
      leaf_processor.decode_node_body();
      std::cout << "hoge3" << std::endl;
    }
    std::cout << "Eff sys: " << leaf_processor.get_system_eff() << std::endl;
    auto node_effs = leaf_processor.get_node_effs();
    for( auto& [address, eff] : node_effs ) {
      std::cout << " - Leaf node(" << address << ") eff: " << eff << std::endl;
    }

    auto [body, hbd] = leaf_processor.extract_leaf_nodes(
      [] ( nestdaq::unpacker::LeafNodeData& body, nestdaq::unpacker::LeafNodeData& hbd )
      {
        nestdaq::unpacker::LeafNodeData new_body;
        nestdaq::unpacker::LeafNodeData new_hbd;

        std::cout << "lambda" << body.size() << std::endl;

        for( auto& node : body ) {
          if( 0xc0a80a0a == node.first ) {
            new_body[node.first] = node.second;
          }
        }

        for( auto& node : hbd ) {
          if( 0xc0a80a0a == node.first ) {
            new_hbd[node.first] = node.second;
          }
        }

        return std::tuple<nestdaq::unpacker::LeafNodeData, nestdaq::unpacker::LeafNodeData> { new_body, new_hbd };
      } );

    for( auto& [fem_id, data_set] : body ) {
      for( auto& data : data_set ) {
        nestdaq::unpacker::show_decoded_feedata( data );
      }
    }

    switch( header["Type"] )
    {
    case nestdaq::TimeFrame::Type::kFull:
      leaf_processor.clear();
      break;

    case nestdaq::TimeFrame::Type::kMeta:
      leaf_processor.clear_node_block();
      leaf_processor.clear_hbd();
      leaf_processor.clear_body();
      break;

    case nestdaq::TimeFrame::Type::kEvent :
      leaf_processor.clear_node_block();
      leaf_processor.clear_body();
      break;

    default:
      break;
    }

  }


  return 0;
}
