package com.prismtech.lite.generator;

import java.util.*;
import com.prismtech.lite.ScopedName;

public class BasicType extends AbstractType
{
  public enum BT
  {
    BOOLEAN ("bool", "DDS_OP_TYPE_BOO", "DDS_OP_SUBTYPE_BOO", Alignment.BOOL, "Boolean"),
    OCTET ("uint8_t", "DDS_OP_TYPE_1BY", "DDS_OP_SUBTYPE_1BY", Alignment.ONE, "Octet"),
    CHAR ("char", "DDS_OP_TYPE_1BY", "DDS_OP_SUBTYPE_1BY", Alignment.ONE, "Char"),
    SHORT ("int16_t", "DDS_OP_TYPE_2BY", "DDS_OP_SUBTYPE_2BY", Alignment.TWO, "Short"),
    USHORT ("uint16_t", "DDS_OP_TYPE_2BY", "DDS_OP_SUBTYPE_2BY", Alignment.TWO, "UShort"),
    LONG ("int32_t", "DDS_OP_TYPE_4BY", "DDS_OP_SUBTYPE_4BY", Alignment.FOUR, "Long"),
    ULONG ("uint32_t", "DDS_OP_TYPE_4BY", "DDS_OP_SUBTYPE_4BY", Alignment.FOUR, "ULong"),
    LONGLONG ("int64_t", "DDS_OP_TYPE_8BY", "DDS_OP_SUBTYPE_8BY", Alignment.EIGHT, "LongLong"),
    ULONGLONG ("uint64_t", "DDS_OP_TYPE_8BY", "DDS_OP_SUBTYPE_8BY", Alignment.EIGHT, "ULongLong"),
    FLOAT ("float", "DDS_OP_TYPE_4BY", "DDS_OP_SUBTYPE_4BY", Alignment.FOUR, "Float"),
    DOUBLE ("double", "DDS_OP_TYPE_8BY", "DDS_OP_SUBTYPE_8BY", Alignment.EIGHT, "Double"),
    STRING ("char *", "DDS_OP_TYPE_STR", "DDS_OP_SUBTYPE_STR", Alignment.PTR, "String");

    public final String cType;
    public final String op;
    public final String subop;
    public final Alignment align;
    public final String XML;

    BT (String cType, String op, String subop, Alignment align, String XML)
    {
      this.cType = cType;
      this.op = op;
      this.subop = subop;
      this.align = align;
      this.XML = XML;
    }
  }

  public BasicType (BT type)
  {
    this.type = type;
  }

  public Type dup ()
  {
    return new BasicType (type);
  }

  public ArrayList <String> getMetaOp (String myname, String structname)
  {
    ArrayList <String> result = new ArrayList <String> (1);
    result.add (new String
    (
      "DDS_OP_ADR | " + type.op + (isKeyField () ? " | DDS_OP_FLAG_KEY" : "") +
      ", offsetof (" + structname + ", " + myname + ")"
    ));
    return result;
  }

  public String getSubOp ()
  {
    return type.subop;
  }

  public String getOp ()
  {
    return type.op;
  }

  public String getCType ()
  {
    return type.cType;
  }

  public int getMetaOpSize ()
  {
    return 2;
  }

  public Alignment getAlignment ()
  {
    return type.align;
  }

  public long getKeySize ()
  {
    switch (type)
    {
      case BOOLEAN:
        return 1;
      case STRING:
        return -1;
      default:
        return type.align.getValue ();
    }
  }

  public void getXML (StringBuffer str, ModuleContext mod)
  {
    str.append ("<");
    str.append (type.XML);
    str.append ("/>");
  }

  public void populateDeps (Set <ScopedName> depset, NamedType current)
  {
  }

  public boolean depsOK (Set <ScopedName> deps)
  {
    return true;
  }

  final BT type;
}
