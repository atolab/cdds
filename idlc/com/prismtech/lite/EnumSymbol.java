package com.prismtech.lite;

public class EnumSymbol extends TypeDefSymbol
{
  public EnumSymbol (ScopedName name)
  {
    super (name);
  }

  public String toString ()
  {
    return "Enum";
  }
}
