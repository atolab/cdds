package com.prismtech.lite;

public abstract class Symbol
{
  Symbol (ScopedName name)
  {
    this.name = name;
  }

  public ScopedName name ()
  {
    return new ScopedName (name);
  }

  private ScopedName name;
}
