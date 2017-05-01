package com.prismtech.lite;

public class OtherConstSymbol extends Symbol
{
  public OtherConstSymbol (ScopedName name)
  {
    super (name);
  }

  public String toString ()
  {
    return "Non-integer constant";
  }
}
