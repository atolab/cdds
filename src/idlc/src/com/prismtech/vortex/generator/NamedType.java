package com.prismtech.vortex.generator;

import com.prismtech.vortex.ScopedName;

public interface NamedType extends Type
{
  public ScopedName getSN ();
  public void getToplevelXML (StringBuffer str, ModuleContext mod);
}
