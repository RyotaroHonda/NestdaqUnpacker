# NestdaqUnpacker

C++ software to decode the data blocks of the NestDAQ data and related front-end electronics.

It requires C++17

## Sources

### Schema and decoders for leaf (fee) nodes

The minimal source sets of this software. These are independent from the NestDAQ components.

- LeafSchema.hh
- LeafDecoder.hh/cc
- LeafDecoder.

### Schema and decoders for NestDAQ data frames

Sources to parse the NestDAQ data frame headers.

- NodeHeaderSchema.hh
- NodeHeaderDecoder.hh/cc

### Leaf node processor

This decodes the payload of the leaf node on a data frame basis and stores the decoded results.
It also parses hardbeat delimiters and obtains statistical information.

- LeafProcessor.hh/cc

### Node unpacker

This parses the time-frame and extracts the lower data blocks.
It will be used only in the offline data analysis.

- NodeUnpacker.hh/cc
