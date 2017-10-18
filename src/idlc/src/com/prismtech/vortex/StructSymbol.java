package com.prismtech.vortex;

import java.util.*;

public class StructSymbol extends TypeDefSymbol
{
  public StructSymbol (ScopedName name)
  {
    super (name);
    members = new ArrayList <String> ();
    valid = true;
  }

  public String toString ()
  {
    StringBuffer result = new StringBuffer ("Struct, members are");
    for (String s : members)
    {
      result.append (' ');
      result.append (s);
    }
    return result.toString ();
  }

  public void addMember (String membername)
  {
    members.add (membername);
  }

  public void addStructMember (String membername, StructSymbol membertype)
  {
    for (String s : membertype.members)
    {
      members.add (membername + "." + s);
    }
  }

  public boolean hasMember (String membername)
  {
    return members.contains (membername);
  }

  public void invalidate ()
  {
    valid = false;
  }

  public boolean isValid ()
  {
    return valid;
  }

  private List <String> members;
  private boolean valid;
}
