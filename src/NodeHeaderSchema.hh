#ifndef NODE_HEADER_SCHEMA_HH
#define NODE_HEADER_SCHEMA_HH

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <bitset>
#include <stdint.h>

namespace nestdaq {

  struct HeaderDecodeDef
  {
    const std::string name;
    const uint64_t    shift;
    const uint64_t    mask;
  };

  namespace PageFlag{
    static const uint32_t kIsLeaf  = 0;
    static const uint32_t kIsBuilt = 1;
    static const uint32_t kSize    = 8;
  }

  template <class T>
  constexpr std::size_t to_bit(T &a)
  {
    return static_cast<std::size_t>(1 << a);
  }

  using HeaderDef        = std::vector<std::vector<HeaderDecodeDef>>;
  using HeaderSchemaPage = std::tuple<std::string, std::bitset<PageFlag::kSize>, HeaderDef>;
  using HeaderSchema     = std::map<uint64_t, HeaderSchemaPage>;

  static const HeaderSchema g_header_schema =
  {
      {
        0x004b4e53454c4946,
        {
          {"FileSinkHeader-v1"}, {0},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}},
            {{"Device Type",     0,  0xffff'ffff'ffff'ffff}},
            {{"Rnu number",      0,  0xffff'ffff'ffff'ffff}},
            {{"Start UNIX time", 0,  0xffff'ffff'ffff'ffff}},
            {{"Stop UNIX time",  0,  0xffff'ffff'ffff'ffff}}
          }
        }
      },
      {
        0x004c5254454c4946,
        {
          {"FileSinkTrailer-v1"}, {0},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}},
            {{"Device Type",     0,  0xffff'ffff'ffff'ffff}},
            {{"Rnu number",      0,  0xffff'ffff'ffff'ffff}},
            {{"Start UNIX time", 0,  0xffff'ffff'ffff'ffff}},
            {{"Stop UNIX time",  0,  0xffff'ffff'ffff'ffff}}
          }
        }
      },
      {
        0x004d5246454d4954,
        {
          {"TimeFrame-v1"}, {0},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}},
            {{"Num source",      32, 0xffff'ffff}, {"TimeFrame ID",  0, 0xffff'ffff}}
          }
        }
      },
      {
        0x00454d4954425553,
        {
          {"SubTimeFrame-v1"}, {0},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}},
            {{"FEM type",        32, 0xffff'ffff}, {"TimeFrame ID",  0, 0xffff'ffff}},
            {{"Num messages",    32, 0xffff'ffff}, {"FEM ID",  0, 0xffff'ffff}},
            {{"Time in sec",     0,  0xffff'ffff'ffff'ffff}},
            {{"Time in usec",    0,  0xffff'ffff'ffff'ffff}}
          }
        }
      },
      {
        0x0049474f'4c544c46,
        {
          {"LogicFilter-v1"}, {0},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}},
            {{"Num trigs",       32, 0xffff'ffff}, {"TimeFrame ID",  0, 0xffff'ffff}},
            {{"Num messages",    32, 0xffff'ffff}, {"Worker ID",  0, 0xffff'ffff}},
            {{"Time in sec",     0,  0xffff'ffff'ffff'ffff}},
            {{"Time in usec",    0,  0xffff'ffff'ffff'ffff}}
        }
        }
      },
      {
        0x0054414542545248,
        {
          {"HeartbeatFrame"}, {to_bit(PageFlag::kIsLeaf) | to_bit(PageFlag::kIsBuilt)},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}}
          }
        }
      },
      {
        0x00454d49'54475254,
        {
          {"TrgTime-v1"}, {to_bit(PageFlag::kIsLeaf)},
          { {{"Version",         56, 0xff}, {"Magic", 0, 0xff'ffff'ffff'ffff}},
            {{"Type",            48, 0xffff}, {"hLength", 32, 0xffff}, {"Length", 0, 0xffff'ffff}}
          }
        }
      }
  };

  namespace TimeFrame
  {
    struct Type {
      static constexpr uint64_t kFull  = 0;
      static constexpr uint64_t kMeta  = 1;
      static constexpr uint64_t kEvent = 2;
    };
  }

} // namespace nestdaq

#endif
