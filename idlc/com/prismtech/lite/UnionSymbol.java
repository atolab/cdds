package com.prismtech.lite;

public class UnionSymbol extends TypeDefSymbol
{
  public UnionSymbol (ScopedName name)
  {
    super (name);
    valid = true;
  }

  public String toString ()
  {
    return "Union";
  }

  public void invalidate ()
  {
    valid = false;
  }

  public boolean isValid ()
  {
    return valid;
  }

  private boolean valid;
}
