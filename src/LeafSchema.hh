#ifndef LEAF_SCHEMA_HH
#define LEAF_SCHEMA_HH

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <bitset>
#include <stdint.h>

namespace nestdaq {

  struct LeafDecodeDef
  {
    const std::string name;
    const uint64_t    shift;
    const uint64_t    mask;
  };

  using LeafCategoryDef = std::vector<std::vector<LeafDecodeDef>>;

  namespace amaneq
  {
    static const LeafDecodeDef g_decode_type = { "Type", 58, 0x3f };
    struct Type
    {
      static constexpr uint32_t kLtdc  = 0b001011;
      static constexpr uint32_t kTtdc  = 0b001101;
      static constexpr uint32_t kItt1s = 0b110011;
      static constexpr uint32_t kItt1e = 0b010001;
      static constexpr uint32_t kItt2s = 0b011010;
      static constexpr uint32_t kItt2e = 0b010010;
      static constexpr uint32_t kHbd1  = 0b011100;
      static constexpr uint32_t kHbd2  = 0b011110;
    };


    static const LeafCategoryDef g_hbd =
    {
      {{"Flags",    40, 0xffff}, {"LACCP offset", 24, 0xffff}, {"Frame number", 0, 0xff'ffff}},    // HBD 1st
      {{"User reg", 40, 0xffff}, {"Gen data size", 20, 0xf'ffff}, {"Trans data size", 0, 0xf'ffff}} // HBD 2nd
    };

    static const LeafCategoryDef g_strlrtdc_cate =
    {
      {{"Ch", 50, 0xff}, {"TOT", 34, 0xffff}, {"TDC", 15, 0x7'ffff}}, // TDC data
      {amaneq::g_hbd[0]}, // HBD 1st
      {amaneq::g_hbd[1]}  // HBD 2nd
    };

    static const LeafCategoryDef g_strhrtdc_cate =
    {
      {{"Ch", 51, 0x7f}, {"TOT", 29, 0x3f'ffff}, {"TDC", 0, 0x1fff'ffff}}, // TDC data
      {amaneq::g_hbd[0]}, // HBD 1st
      {amaneq::g_hbd[1]}  // HBD 2nd
    };

    // Heartbeat delimiter flags //
    struct HbdFlag {
      static constexpr size_t k_size_hbdflag = 16;
    };

    static const std::vector<LeafDecodeDef> g_hbd_flag_def =
    {
      {"HBD flag-1",               0, 0x1},
      {"HBD flag-2",               1, 0x1},
      {"HB frame throttling",      4, 0x1},
      {"Output throttling",        5, 0x1},
      {"Input throttling type-2",  6, 0x1},
      {"Input throttling type-1",  7, 0x1},
      {"Local hbf num mismatch",   9, 0x1},
      {"Global hbf num mismatch", 10, 0x1},
      {"Overflow",                11, 0x1},
      {"MIKUMARI error",          13, 0x1},
      {"Radiation URE",           14, 0x1}
    };
  };

  struct LeafWordDef
  {
    const std::string type_name;
    const std::vector<LeafDecodeDef> word_def;
  };

  using LeafDef = std::unordered_map<uint32_t, LeafWordDef>;

  static const LeafDef g_strlrtdc =
  {
    {amaneq::Type::kLtdc,   {"L-TDC",  amaneq::g_strlrtdc_cate[0]}}, // Leading TDC
    {amaneq::Type::kTtdc,   {"T-TDC",  amaneq::g_strlrtdc_cate[0]}}, // Trailing TDC
    {amaneq::Type::kItt1s,  {"ITT-1-S",amaneq::g_strlrtdc_cate[0]}}, // Input throttling type-1 start
    {amaneq::Type::kItt1e,  {"ITT-1-E",amaneq::g_strlrtdc_cate[0]}}, // Input throttling type-1 end
    {amaneq::Type::kItt2s,  {"ITT-2-S",amaneq::g_strlrtdc_cate[0]}}, // Input throttling type-2 start
    {amaneq::Type::kItt2e,  {"ITT-2-E",amaneq::g_strlrtdc_cate[0]}}, // Input throttling type-2 start
    {amaneq::Type::kHbd1,   {"HBD-1",  amaneq::g_strlrtdc_cate[1]}}, // Heartbeat delimiter 1st
    {amaneq::Type::kHbd2,   {"HBD-2",  amaneq::g_strlrtdc_cate[2]}}  // Heartbeat delimiter 2nd
  };

  static const LeafDef g_strhrtdc =
  {
    {amaneq::Type::kLtdc,   {"L-TDC",  amaneq::g_strhrtdc_cate[0]}}, // Leading TDC
    {amaneq::Type::kTtdc,   {"T-TDC",  amaneq::g_strhrtdc_cate[0]}}, // Trailing TDC
    {amaneq::Type::kItt1s,  {"ITT-1-S",amaneq::g_strhrtdc_cate[0]}}, // Input throttling type-1 start
    {amaneq::Type::kItt1e,  {"ITT-1-E",amaneq::g_strhrtdc_cate[0]}}, // Input throttling type-1 end
    {amaneq::Type::kItt2s,  {"ITT-2-S",amaneq::g_strhrtdc_cate[0]}}, // Input throttling type-2 start
    {amaneq::Type::kItt2e,  {"ITT-2-E",amaneq::g_strhrtdc_cate[0]}}, // Input throttling type-2 start
    {amaneq::Type::kHbd1,   {"HBD-1",  amaneq::g_strhrtdc_cate[1]}}, // Heartbeat delimiter 1st
    {amaneq::Type::kHbd2,   {"HBD-2",  amaneq::g_strhrtdc_cate[2]}}  // Heartbeat delimiter 2nd
  };

  using LeafSchemaPage    = std::tuple<std::string, LeafDecodeDef, LeafDef>;
  using LeafSchema        = std::map<uint32_t, LeafSchemaPage>;

  static const LeafSchema g_leaf_schema =
      {
        {5, {{"Str-HRTDC"}, {amaneq::g_decode_type}, {g_strhrtdc}}},
        {6, {{"Str-LRTDC"}, {amaneq::g_decode_type}, {g_strlrtdc}}}
      };

//  static const LeafSchemaPage g_strlrtdc_page =
//  {
//    {"Str-LRTDC"}, {amaneq::g_decode_type}, {g_strlrtdc}
//  };
//
//  static const LeafSchemaPage g_strhrtdc_page =
//  {
//    {"Str-HRTDC"}, {amaneq::g_decode_type}, {g_strhrtdc}
//  };



} // namespace nestdaq

#endif
