package com.prismtech.lite;

public class IntConstSymbol extends Symbol
{
  public IntConstSymbol (ScopedName name)
  {
    super (name);
  }

  public String toString ()
  {
    return "Integer constant";
  }
}
