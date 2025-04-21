#include<cstdio>
#include<ios>
#include<iostream>
#include<fstream>
#include<sstream>
#include<iterator>

#include "NodeUnpacker.hh"
#include "LeafSchema.hh"
#include "LeafDecoder.hh"

using namespace nestdaq::unpacker;

static int hierarchy_counter = 0;

// Constructor __________________________________________________________
NodeUnpacker::NodeUnpacker( const HeaderSchemaPage& page )
  :
  my_name_( std::get<0>( page ) ),
  flag_( std::get<1>( page ) ),
  header_def_( std::get<2>( page ) )
{
}

// _______________________________________________________________________
auto
NodeUnpacker::extract_leafnodes()
-> std::vector<std::tuple<DecodedHeaderData, DataPtr, DataPtr>>
{
  if( flag_[PageFlag::kIsLeaf] && flag_[PageFlag::kIsBuilt] ) {
    auto parent = parent_.lock();
    return { { parent->get_header(), body_first_, body_last_ } };
  } else {
    std::vector<std::tuple<DecodedHeaderData, DataPtr, DataPtr>> leafs;
    for( auto [index, child] : child_list_ ) {
      auto&& leaf_node = child->extract_leafnodes();
      leafs.insert( leafs.end(), leaf_node.begin(), leaf_node.end() );
    }
    return leafs;
  }
}

// _______________________________________________________________________
const DataPtr
NodeUnpacker::get_last_ptr() const
{
  return static_cast< const DataPtr >( block_last_ );
}

// _______________________________________________________________________
void
NodeUnpacker::set_data( const DataPtr first )
{
  block_first_ = first;
}

// _______________________________________________________________________
void
NodeUnpacker::set_parent( std::shared_ptr<NodeUnpacker> ptr )
{
  parent_ = ptr;
}

// _______________________________________________________________________
void
NodeUnpacker::set_ptr_position()
{
  if( header_data_["Length"] == header_data_["hLength"] ) {
    // Empty block //
    body_first_ = nullptr;
    body_last_  = ( block_first_ + header_data_["Length"]/sizeof( uint64_t ) -1 );
    block_last_ = ( block_first_ + header_data_["Length"]/sizeof( uint64_t ) -1 );
  } else {
    // Set body first position
    body_first_ = ( block_first_ + header_data_["hLength"]/sizeof( uint64_t ) );
    body_last_  = ( block_first_ + header_data_["Length"]/sizeof( uint64_t ) -1 );
    block_last_ = ( block_first_ + header_data_["Length"]/sizeof( uint64_t ) -1 );
  }
}

// _______________________________________________________________________
void
NodeUnpacker::show_block_structure( const std::function<void( const HeaderDef&, const DecodedHeaderData& )>& callback ) const
{
  if( parent_.expired() ) {
    hierarchy_counter = 0;
  }

  std::string indent{ "-" };
  for( int i = 0; i < hierarchy_counter; ++i ) {
    indent.insert( 0, " " );
  }
  ++hierarchy_counter;

  std::cout << indent << " " << my_name_ << ", ";
  show_header( callback );
  for( auto [index, child] : child_list_ ) {
    child->show_block_structure( callback );
  }

  --hierarchy_counter;
}

// _______________________________________________________________________
void
NodeUnpacker::show_header( const std::function<void( const HeaderDef&, const DecodedHeaderData& )>& callback ) const
{
  return callback( header_def_, header_data_ );
}

// _______________________________________________________________________
std::shared_ptr<NodeUnpacker>
NodeUnpacker::get_child( uint64_t index ) const
{
  static const std::string func_name( "[" + my_name_ + "::" + __func__ + "()] " );

  auto child = child_list_.find( index );
  if( child == child_list_.end() ) {
    std::cerr << "#E: " << func_name
              << "No such child with index of " << index << std::endl;
    return nullptr;
  }

  return child->second;
}

// ________________________________________________________________________
DecodedHeaderData
NodeUnpacker::get_header() const
{
  return header_data_;
}

// ________________________________________________________________________
uint64_t
NodeUnpacker::get_header_content( std::string name ) const
{
  return header_data_.at(name);
}

// ________________________________________________________________________
std::shared_ptr<NodeUnpacker>
NodeUnpacker::get_parent() const
{
  return parent_.lock();
}

// ________________________________________________________________________
void
NodeUnpacker::unpack()
{
  static const std::string func_name( "[" + my_name_ + "::" + __func__ + "()] " );

  nestdaq::unpacker::decode_header( header_def_, block_first_, header_data_ );
  set_ptr_position();
  //show_header();

//  if( body_first_ == nullptr ) {
//    // End of Block, exist unpack process //
//    std::cout << "#W: " << func_name << " Empty data block detected. No payload exists in this block." << std::endl;
//    nestdaq::unpacker::show_header( header_def_, header_data_ );
//
//    return;
//  }

  for( DataPtr ptr = body_first_; ptr != ( block_last_+1 ); ) {
    if( flag_[PageFlag::kIsLeaf] ) {
      // It's expected that FEE data exist in this payload //
      std::cout << "I'm leaf" << flag_[PageFlag::kIsBuilt] << std::endl;
      auto parent = this->get_parent();
      if( !parent ) {
        std::cout << "#E: " << func_name << "std::shared_ptr manged object, (parent), deleted." << std::endl;
        return;
      }

      break;
    } else {
      // Find sub-block //
      auto itr_schema_page = g_header_schema.find( *ptr );
      if( g_header_schema.end() == itr_schema_page ) {
        std::cout << "#E: Unkown data block" << std::endl;
        return;
      }

      auto unpacker = std::make_shared<NodeUnpacker>( itr_schema_page->second );
      unpacker->set_data( ptr );
      unpacker->set_parent( shared_from_this() );
      unpacker->unpack();
      child_list_[child_index_++] = unpacker;

      ptr = unpacker->get_last_ptr() + 1;
    }
  }
}


