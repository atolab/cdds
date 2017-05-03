package com.prismtech.lite.generator;

import com.prismtech.lite.ScopedName;

public interface NamedType extends Type
{
  public ScopedName getSN ();
  public void getToplevelXML (StringBuffer str, ModuleContext mod);
}
