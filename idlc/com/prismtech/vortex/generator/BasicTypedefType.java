package com.prismtech.vortex.generator;

import com.prismtech.vortex.ScopedName;
import java.util.*;

public class BasicTypedefType extends BasicType implements NamedType
{
  public BasicTypedefType (ScopedName name, BasicType ref)
  {
    super (ref.type);
    this.name = new ScopedName (name);
  }

  public Type dup ()
  {
    return new TypedefType (name, new BasicType (type));
  }

  public void getToplevelXML (StringBuffer str, ModuleContext mod)
  {
    mod.enter (str, name);
    str.append ("<TypeDef name=\\\"");
    str.append (name.getLeaf ());
    str.append ("\\\">");
    super.getXML (str, mod);
    str.append ("</TypeDef>");
  }

  public void getXML (StringBuffer str, ModuleContext mod)
  {
    str.append ("<Type name=\\\"");
    str.append (mod.nameFrom (name));
    str.append ("\\\"/>");
  }

  public void populateDeps (Set <ScopedName> depset, NamedType current)
  {
    depset.add (name);
  }

  public boolean depsOK (Set <ScopedName> deps)
  {
    return true;
  }

  public ScopedName getSN ()
  {
    return name;
  }

  public boolean isInline ()
  {
    return false;
  }

  private final ScopedName name;
}
