package com.prismtech.lite.generator;

import com.prismtech.lite.ScopedName;
import java.util.Set;

public class TypeUtil
{
  public static boolean deptest
    (Type t, Set <ScopedName> deps, ScopedName parent)

  /* t - the Type we're testing for
   * deps - the types that have been written
   * parent - the containing type, if any
   */
  {
    if (!t.depsOK (deps))
    {
      return false;
    }

    if (t instanceof NamedType)
    {
      NamedType nt = (NamedType)t;
      ScopedName sn;
      if (parent != null)
      {
        sn = new ScopedName (nt.getSN ());
        sn.popComponent ();
        if (sn.equals (parent)) // It's inline
        {
          return true;
        }
      }
      sn = new ScopedName (nt.getSN ());

      // deps only has toplevel containing types, so we may need to go up
      do
      {
        if (deps.contains (sn))
        {
          return true;
        }
      } while (!sn.popComponent ().equals (""));
      return false;
    }
    else
    {
      return true;
    }
  }
}
