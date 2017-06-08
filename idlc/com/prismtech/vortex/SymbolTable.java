package com.prismtech.vortex;

import java.util.*;

public class SymbolTable
{
  public SymbolTable()
  {
    map = new HashMap <ScopedName, Symbol> ();
  }

  public void add (Symbol newsym)
  {
    Symbol oldval;
    oldval = map.put (newsym.name (), newsym);
    if (oldval != null)
    {
      System.err.println ("Internal inconsistency, multiple definition for " + newsym.name ());
      System.err.println ("Old value was " + oldval.toString ());
      System.err.println ("New value is " + newsym.toString ());
    }
  }

  public Symbol resolve (ScopedName current, ScopedName request)
  {
    Symbol result = null;
    if (current != null)
    {
      ScopedName searchscope = new ScopedName (current);
      do
      {
        result = map.get (searchscope.catenate (request));
      }
      while (result == null && ! searchscope.popComponent().equals (""));
    }
    if (result == null)
    {
      result = map.get (request);
    }
    return result;
  }

  public Symbol getSymbol (ScopedName request)
  {
    return map.get (request);
  }

  public void dump ()
  {
    for (Map.Entry <ScopedName, Symbol> sym : map.entrySet ())
    {
      System.out.println (sym.getKey () + " is a " + sym.getValue ());
    }
  }

  Map <ScopedName, Symbol> map;
}

