/****************************************************************

  Generated by Vortex Lite IDL to C Translator
  File name: Throughput.c
  Source: Throughput.idl
  Generated: Fri Mar 18 16:10:12 CET 2016
  Vortex Lite: V2.1.0

*****************************************************************/
#include "Throughput.h"


static const uint32_t ThroughputModule_DataType_ops [] =
{
  DDS_OP_ADR | DDS_OP_TYPE_8BY, offsetof (ThroughputModule_DataType, count),
  DDS_OP_ADR | DDS_OP_TYPE_SEQ | DDS_OP_SUBTYPE_1BY, offsetof (ThroughputModule_DataType, payload),
  DDS_OP_RTS
};

const dds_topic_descriptor_t ThroughputModule_DataType_desc =
{
  sizeof (ThroughputModule_DataType),
  8u,
  DDS_TOPIC_NO_OPTIMIZE,
  0u,
  "ThroughputModule::DataType",
  NULL,
  3u,
  ThroughputModule_DataType_ops,
  "<MetaData version=\"1.0.0\"><Module name=\"ThroughputModule\"><Struct name=\"DataType\"><Member name=\"count\"><ULongLong/></Member><Member name=\"payload\"><Sequence><Octet/></Sequence></Member></Struct></Module></MetaData>"
};