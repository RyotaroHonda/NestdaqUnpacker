#ifndef LEAF_PROCESSOR_HH
#define LEAF_PROCESSOR_HH

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <stdint.h>
#include <memory>
#include <functional>
#include "LeafSchema.hh"
#include "LeafDecoder.hh"
#include "NodeHeaderDecoder.hh"

namespace nestdaq::unpacker
{

  using DataPtr       = uint64_t*;
  using LeafNodeSet   = std::tuple<DecodedHeaderData, const LeafSchemaPage, DataPtr, DataPtr>;
  using LeafNodeData  = std::unordered_map<uint64_t, std::vector<LeafDataSet>>;
  using LeafDataArray = std::vector<LeafNodeData>;

  class LeafProcessor final
  {
  public:
    struct CtrlKeys {
      static constexpr size_t kImmediateFlush = 0;
      static constexpr size_t kSizeFlags      = 8;
    };

    LeafProcessor() = default;
    LeafProcessor( const LeafProcessor& ) = delete;
    ~LeafProcessor() = default;

    const LeafProcessor& operator=( const LeafProcessor& ) = delete;

    void build_time_aligned_block( std::vector<std::tuple<DecodedHeaderData, DataPtr, DataPtr>> );
    void clear();
    void clear_node_block();
    void clear_body();
    void clear_hbd();
    void clear_summary();
    auto decode_heartbeat_delimiter() -> std::vector<std::string>;
    void decode_node_body();
    auto extract_leaf_nodes( const std::function<std::tuple<LeafNodeData, LeafNodeData>( LeafNodeData&, LeafNodeData& )>& ) -> std::tuple<LeafNodeData, LeafNodeData>;
    auto flush_log() -> std::string;
    auto get_leafnode_data( uint64_t fem_id ) const -> const std::tuple<std::vector<LeafDataSet>, std::vector<LeafDataSet>>;
    auto get_num_frame() const -> uint32_t;
    auto get_node_effs() const -> std::vector<std::tuple<std::string, double>>;
    auto get_node_ids() const -> std::vector<uint64_t>;
    auto get_system_eff() const -> double;
    void reset_ctrl( size_t );
    void set_ctrl( size_t );
    void set_frame_index( uint32_t index );
    void set_leaf_node( DecodedHeaderData&, DataPtr, DataPtr );
    void set_tf_type( uint64_t type );
    void show_built_block() const;
    void show_body( uint64_t fem_id ) const;
    void show_hbd( uint64_t fem_id ) const;

  private:
    // Class control //
    std::bitset<CtrlKeys::kSizeFlags> ctrl_flags_{ 0 };
    std::ostringstream log_;
    uint64_t time_frame_type_{ 0 };

    // These private variables are indexed with frame_index_ //
    uint32_t frame_index_{ 0 };
    std::vector<std::vector<LeafNodeSet>> time_aligned_block_;

    // Decoded data //
    LeafDataArray data_hbd_;
    LeafDataArray data_body_;
    // These private variables are indexed with frame_index_ //

    // Summary of heartbeat delimiter //
    struct NodeHbdSummary {
      uint32_t n_clean_hbframe{ 0 };
      uint32_t n_in_thrott_t2{ 0 };
      uint32_t n_out_thrott{ 0 };
      uint32_t gen_data_size{ 0 };
      uint32_t trans_data_size{ 0 };
    };

    std::unordered_map<uint64_t, NodeHbdSummary> hbd_summary_book_;
    uint32_t n_hbframe_{ 0 };
    uint32_t n_clean_hbframe_{ 0 };

    // Method //
    bool check_hbd_type( std::vector<LeafDataSet>& , std::string&, DataPtr);
    bool find_leaf( const DecodedHeaderData&, const std::vector<LeafNodeSet>& ) const;
    bool have_critical_flag( LeafData& decoded_flag, std::string& fem_address );
    void show_node_data_impl( const uint64_t fem_id, const LeafDataArray& array ) const;


  };
} // nestdaq::unpacker

#endif
