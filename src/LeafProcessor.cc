#include<cstdio>
#include<ios>
#include<iostream>
#include<fstream>
#include<sstream>
#include<iterator>

#include "LeafProcessor.hh"
#include "NodeHeaderSchema.hh"
#include "NodeHeaderDecoder.hh"
#include "LeafSchema.hh"
#include "LeafDecoder.hh"

namespace
{
  static const std::string class_name = "LeafProcessor";
};

using namespace nestdaq::unpacker;

// _______________________________________________________________________
void
LeafProcessor::build_time_aligned_block( std::vector<std::tuple<DecodedHeaderData, DataPtr, DataPtr>> leafs)
{
  for( auto [header_data, body_first, body_last] : leafs ) {
    set_leaf_node( header_data, body_first, body_last );
  }

  frame_index_ = 0;

  return;
}

// _______________________________________________________________________
void
LeafProcessor::clear()
{
  n_clean_hbframe_ = 0;
  n_hbframe_ = 0;
  clear_node_block();
  clear_body();
  clear_hbd();
  clear_summary();
}

// _______________________________________________________________________
void
LeafProcessor::clear_body()
{
  data_body_.clear();
}

// _______________________________________________________________________
void
LeafProcessor::clear_hbd()
{
  data_hbd_.clear();
}

// _______________________________________________________________________
void
LeafProcessor::clear_node_block()
{
  time_aligned_block_.clear();
}

// _______________________________________________________________________
void
LeafProcessor::clear_summary()
{
  n_clean_hbframe_ = 0;
  n_hbframe_ = 0;
  hbd_summary_book_.clear();
}

// _______________________________________________________________________
auto
LeafProcessor::decode_heartbeat_delimiter()
-> std::vector<std::string>
{
  static const std::string func_name( "[" + class_name + "::" + __func__ + "()] " );

  if( time_frame_type_ == nestdaq::TimeFrame::Type::kEvent ) {
    // This is event time-frame. Do nothing //
    return {};
  }

  // This time-frame is full or meta. Leaf nodes should have hbd. //
  data_hbd_.resize( time_aligned_block_.size() );

  bool is_invalid_frame = false;
  std::vector<std::string> critical_fem;
  auto& indexed_block = time_aligned_block_[frame_index_];

  for( auto& node : indexed_block ) {
    auto& [header_data, schema_page, body_first, body_last] = node;
    std::vector<LeafDataSet> data_set;
    data_set.push_back( decode_fee_word( body_last-1, schema_page ) );
    data_set.push_back( decode_fee_word( body_last, schema_page ) );

    auto fem_id = header_data["FEM ID"];
    std::string fem_address = nestdaq::unpacker::to_ipaddress( fem_id );

    if( check_hbd_type( data_set, fem_address, body_last ) ) {
      // Heartbeat frame is broken //
      critical_fem.push_back( fem_address );
      is_invalid_frame = true;
      continue;
    }

    // Normal process //
    data_hbd_[frame_index_].insert( { fem_id, data_set } );

    // Make summary //
    if( hbd_summary_book_.find( fem_id ) == hbd_summary_book_.end() ) {
      NodeHbdSummary summary_page{0};
      hbd_summary_book_[fem_id] = summary_page;
    }

    auto& hbd_summary_page = hbd_summary_book_[fem_id];
    auto& [type_name1, data1] = data_set.at( 0 );
    auto& [type_name2, data2] = data_set.at( 1 );

    // hbd flags //
    if( data1["Flags"] == 0 ) {
      hbd_summary_page.n_clean_hbframe++;
    } else {
      auto decoded_flag = nestdaq::unpacker::decode_hbdflag( data1["Flags"] );
      if( decoded_flag["Output throttling"] )       hbd_summary_page.n_out_thrott++;
      if( decoded_flag["Input throttling type-2"] ) hbd_summary_page.n_in_thrott_t2++;
      is_invalid_frame = true;

      // Check critical flags //
      if( have_critical_flag( decoded_flag, fem_address ) ) {
        critical_fem.push_back( fem_address );
      }// Critical status
    }// HBD flag is not zero

    // Check data size //
    hbd_summary_page.gen_data_size   += data2["Gen data size"];
    hbd_summary_page.trans_data_size += data2["Trans data size"];
    //    if( data2["Gen data size"] != data2["Trans data size"] ) is_invalid_frame = true;
    if( data2["Gen data size"] != data2["Trans data size"] ) {
      if( data1["Flags"] == 0 ) {
        log_
          << func_name << "Leaf node frame (" << fem_address << ")"
          << " silently drops data" << std::endl;
        critical_fem.push_back( fem_address );
      }
    }
  } // for (node)

  if( !is_invalid_frame ) ++n_clean_hbframe_;
  ++n_hbframe_;

  if( ctrl_flags_[CtrlKeys::kImmediateFlush] ) {
      std::cout << flush_log() << std::flush;
  }

  return critical_fem;
}

// _______________________________________________________________________
void
LeafProcessor::decode_node_body()
{
  data_body_.resize( time_aligned_block_.size() );

  std::vector<std::string> critical_fem;
  auto& indexed_block = time_aligned_block_[frame_index_];

  for( auto& node : indexed_block ) {
    auto& [header_data, schema_page, body_first, body_last] = node;
    if( !body_first ) continue;

    auto fem_id = header_data["FEM ID"];
    //    std::string fem_address = nestdaq::unpacker::to_ipaddress( fem_id );
    std::vector<LeafDataSet> data_set;
    auto body_end = body_last +1;
    for( auto ptr = body_first; ptr != body_end; ++ptr ) {
      auto [type_name, data] = decode_fee_word( ptr, schema_page );
      if( !( type_name == "HBD-1" || type_name == "HBD-2" ) ) {
        data_set.push_back( { type_name, data } );
      }
    }

    data_body_[frame_index_].insert( { fem_id, data_set } );

  }
}

// _______________________________________________________________________
auto
LeafProcessor::extract_leaf_nodes( const std::function<std::tuple<LeafNodeData, LeafNodeData>( LeafNodeData&, LeafNodeData& )>& callback)
-> std::tuple<LeafNodeData, LeafNodeData>
{
  return callback( data_body_[frame_index_], data_hbd_[frame_index_] );
}

// _______________________________________________________________________
auto
LeafProcessor::flush_log() -> std::string
{
  std::string log_msg = log_.str();
  log_.str( "" );
  return log_msg;
}

// _______________________________________________________________________
auto
LeafProcessor::get_leafnode_data( uint64_t fem_id ) const
-> const std::tuple<std::vector<LeafDataSet>, std::vector<LeafDataSet>>
{
  auto itr_body = data_body_[frame_index_].find( fem_id );
  auto itr_hbd = data_hbd_[frame_index_].find( fem_id );

  if( true
    && itr_body != data_body_[frame_index_].end()
    && itr_hbd != data_hbd_[frame_index_].end() ) {
    return { itr_body->second, itr_hbd->second };
  } else {
    return { {}, {} };
  }
}

// _______________________________________________________________________
auto
LeafProcessor::get_num_frame() const
-> uint32_t
{
  return time_aligned_block_.size();
}

// _______________________________________________________________________
auto
LeafProcessor::get_node_effs() const
-> std::vector<std::tuple<std::string, double>>
{
  std::vector<std::tuple<std::string, double>> ret{0};
  for( auto& [fem_id, hbd_summary_page] : hbd_summary_book_ ) {
    double eff = 1.0;
    //if( hbd_summary_page.gen_data_size != 0 ) eff = static_cast< double >( hbd_summary_page.trans_data_size )/hbd_summary_page.gen_data_size;
    if(n_hbframe_ != 0) eff = static_cast< double >( hbd_summary_page.n_clean_hbframe )/n_hbframe_;
    auto address = nestdaq::unpacker::to_ipaddress( fem_id );
    ret.push_back( { address, eff } );
  }

  return ret;
}

// _______________________________________________________________________
auto
LeafProcessor::get_node_ids() const
-> std::vector<uint64_t>
{
  std::vector<uint64_t> ret;
  auto& indexed_frame = time_aligned_block_.at( 0 );
  for( auto& node : indexed_frame) {
    auto& header_data = std::get<0>( node );
    ret.push_back( header_data.at( "FEM ID" ) );
  }

  return ret;
}

// _______________________________________________________________________
auto
LeafProcessor::get_system_eff() const
-> double
{
  return  n_hbframe_ != 0 ? static_cast< double >( n_clean_hbframe_ )/n_hbframe_ : 1.0;
}

// _______________________________________________________________________
void
LeafProcessor::set_ctrl(size_t index)
{
  ctrl_flags_.set( index );
}

// _______________________________________________________________________
void
LeafProcessor::set_frame_index( uint32_t index )
{
  frame_index_ = index;
}

// _______________________________________________________________________
void
LeafProcessor::set_leaf_node( DecodedHeaderData& header_data_in, DataPtr body_first_in, DataPtr body_last_in)
{
  static const std::string func_name( "[" + class_name + "::" + __func__ + "()] " );

  auto itr = g_leaf_schema.find( header_data_in["FEM type"] );
  if( itr == g_leaf_schema.end() ) {
    log_ << func_name << "No such FEM type is defined in LeafSchema" << std::endl;
    log_ << std::hex << " - Magic: 0x" << header_data_in.at( "Magic" ) << std::endl;
    if( ctrl_flags_[LeafProcessor::CtrlKeys::kImmediateFlush] ) {
      std::cout << flush_log() << std::flush;
    }
    return;
  }

  const LeafSchemaPage& schema_page = itr->second;

  for( auto& indexed_block : time_aligned_block_ ) {
    if( !find_leaf( header_data_in, indexed_block ) ) {
      indexed_block.push_back( { header_data_in, schema_page, body_first_in, body_last_in } );
      return;
    }
  }

  // New index block //
  time_aligned_block_.push_back( { { header_data_in, schema_page, body_first_in, body_last_in } } );

  return;
}

// _______________________________________________________________________
void
LeafProcessor::set_tf_type( uint64_t type )
{
  time_frame_type_ = type;
}

// _______________________________________________________________________
void
LeafProcessor::show_built_block() const
{
  static const std::string func_name( "[" + class_name + "::" + __func__ + "()] " );

  std::cout << func_name << std::endl;
  std::cout << " Number of HB blocks: " << time_aligned_block_.size() << std::endl;

  int32_t index = 0;
  for( auto& indexed_block : time_aligned_block_ ) {
    std::cout << " - Block Index: " << index++ << std::endl;

    for( auto& leaf_node : indexed_block ) {
      auto& [header_data, schema_page, body_first, body_last] = leaf_node;
      auto [type_name1, data1] = decode_fee_word( body_last-1, schema_page );

      std::cout
        << "  -ID (" << nestdaq::unpacker::to_ipaddress( header_data.at( "FEM ID" ) ) << "), "
        << "HB frame number: " << std::dec << data1.at("Frame number") << std::endl;
    }
  }
}

// _______________________________________________________________________
void
LeafProcessor::show_body( uint64_t fem_id ) const
{
  show_node_data_impl( fem_id, data_body_ );
}

// _______________________________________________________________________
void
LeafProcessor::show_hbd( uint64_t fem_id ) const
{
  show_node_data_impl( fem_id, data_hbd_ );
}

// _______________________________________________________________________
void
LeafProcessor::reset_ctrl(size_t index)
{
  ctrl_flags_.reset( index );
}

// Private methods -------------------------------------------------------
// _______________________________________________________________________
bool
LeafProcessor::check_hbd_type( std::vector<LeafDataSet>& data_set, std::string& fem_address, DataPtr body_last )
{
  static const std::string func_name( "[" + class_name + "::" + __func__ + "()] " );

  auto& [type_name1, data1] = data_set.at( 0 );
  auto& [type_name2, data2] = data_set.at( 1 );

  if( type_name1 != "HBD-1" || type_name2 != "HBD-2" ) {
    // Heartbeat frame is broken //
    log_
      << func_name << "Leaf node frame (" << fem_address << ")"
      << " is not ended with heartbeat delimiter" << std::endl;
    log_
      << " 2nd last: 0x" << std::hex << *( body_last-1 ) << "\n"
      << "     last: 0x" << std::hex << *( body_last ) << std::endl;

    return true;
  } else {
    return false;
  }
}

// _______________________________________________________________________
bool
LeafProcessor::find_leaf( const DecodedHeaderData& header_data_in, const std::vector<LeafNodeSet>& block) const
{
  for( const auto& leaf_node : block ) {
    const auto& header_data = std::get<0>( leaf_node );
    if( header_data.at("FEM ID") == header_data_in.at("FEM ID") ) {
      return true;
    }
  }

  return false;
}

// _______________________________________________________________________
bool
LeafProcessor::have_critical_flag( LeafData& decoded_flag, std::string& fem_address )
{
  static const std::string func_name( "[" + std::string( __func__ ) + "()] " );

  if( false
    || decoded_flag["Local hbf num mismatch"]
    || decoded_flag["Overflow"]
    || decoded_flag["MIKUMARI error"]
    || decoded_flag["Radiation URE"]
    ) {

    log_
      << func_name << "Leaf node frame (" << fem_address << ")"
      << " is under critical status" << std::endl;
    for( auto& [name, value] : decoded_flag ) {
      log_ << name << ": " << std::dec << value << std::endl;
    }
    return true;
  } else {
    return false;
  }
}

// _______________________________________________________________________
void
LeafProcessor::show_node_data_impl( const uint64_t fem_id, const LeafDataArray& array ) const
{
  static const std::string func_name( "[" + class_name + "::" + __func__ + "()] " );

  if( array.size() == 0 ) {
    std::cerr << "#E : " << func_name << "Decoded data is empty. Call decode method first." << std::endl;
    return;
  }

  std::string address = nestdaq::unpacker::to_ipaddress( fem_id );

  auto itr = array[frame_index_].find( fem_id );
  if( itr == array[frame_index_].end() ) {
    std::cerr << "#E : " << func_name << "No such FEM ID device: " << address << std::endl;
    return;
  }

  auto leaf_node_data = itr->second;
  std::for_each(
    leaf_node_data.begin(), leaf_node_data.end(),
    [&] ( auto& data_set )
    {
      std::cout << address << ", " << std::flush;
      nestdaq::unpacker::show_decoded_feedata( data_set );
    }
  );

  return;
}
