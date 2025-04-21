#ifndef NODE_UNPACKER_HH
#define NODE_UNPACKER_HH

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <stdint.h>
#include <memory>
#include <functional>
#include "NodeHeaderDecoder.hh"

namespace nestdaq::unpacker
{

  using DataPtr     = uint64_t*;

  class NodeUnpacker final : public std::enable_shared_from_this<NodeUnpacker>
  {
    public:
      NodeUnpacker() = delete;
      NodeUnpacker( const NodeUnpacker& ) = delete;
      explicit NodeUnpacker( const HeaderSchemaPage& page );
      ~NodeUnpacker() = default;

      const NodeUnpacker& operator=( const NodeUnpacker& ) = delete;
      auto extract_leafnodes() -> std::vector<std::tuple<DecodedHeaderData, DataPtr, DataPtr>>;
      auto get_child( uint64_t index ) const -> std::shared_ptr<NodeUnpacker>;
      auto get_child_size();
      auto get_header() const -> DecodedHeaderData;
      auto get_header_content( std::string name ) const -> uint64_t;
      auto get_parent() const -> std::shared_ptr<NodeUnpacker>;
      void set_data( const DataPtr first );
      void set_parent( std::shared_ptr<NodeUnpacker> ptr );
      void set_ptr_position();
      void show_block_structure( const std::function<void( const HeaderDef&, const DecodedHeaderData& )>& ) const;
      void show_header( const std::function<void( const HeaderDef&, const DecodedHeaderData& )>& ) const;
      void unpack();

    private:
      const std::string my_name_;
      const std::bitset<8> flag_;
      const HeaderDef header_def_;

      std::weak_ptr<NodeUnpacker> parent_;
      std::map<uint32_t, std::shared_ptr<NodeUnpacker>> child_list_;

      int32_t child_index_ {0};

      DecodedHeaderData header_data_     {0};
      DataPtr block_first_        {nullptr};
      DataPtr body_first_         {nullptr};
      DataPtr body_last_          {nullptr};
      DataPtr block_last_         {nullptr};

      const DataPtr get_last_ptr() const;
  };

}// nestdaq::unpacker

#endif
