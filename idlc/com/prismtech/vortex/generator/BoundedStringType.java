package com.prismtech.vortex.generator;

import java.util.*;
import com.prismtech.vortex.ScopedName;

public class BoundedStringType extends AbstractType
{
  public BoundedStringType (long size)
  {
    this.size = size;
  }

  public Type dup ()
  {
    return new BoundedStringType (size);
  }

  public long getKeySize ()
  {
    return size + 5;
  }

  public long getBound ()
  {
    return size;
  }

  public ArrayList <String> getMetaOp (String myname, String structname)
  {
    ArrayList <String> result = new ArrayList <String> ();
    String offset;
    if (myname != null)
    {
      offset = "offsetof (" + structname + ", " + myname + ")";
    }
    else
    {
      offset = "0u";
    }

    result.add (new String
    (
      "DDS_OP_ADR | DDS_OP_TYPE_BST" +
      (isKeyField () ? " | DDS_OP_FLAG_KEY" : "") + ", " +
      offset + ", " + Long.toString (size + 1)
    ));

    return result;
  }

  public String getSubOp ()
  {
    return "DDS_OP_SUBTYPE_BST";
  }

  public String getOp ()
  {
    return "DDS_OP_TYPE_BST";
  }

  public String getCType ()
  {
    return "char";
  }

  public int getMetaOpSize ()
  {
    return 3;
  }

  public Alignment getAlignment ()
  {
    return Alignment.ONE;
  }

  public void getXML (StringBuffer str, ModuleContext mod)
  {
    str.append ("<String length=\\\"");
    str.append (Long.toString (size));
    str.append ("\\\"/>");
  }

  public void populateDeps (Set <ScopedName> depset, NamedType current)
  {
  }

  public boolean depsOK (Set <ScopedName> deps)
  {
    return true;
  }

  private final long size;
}
