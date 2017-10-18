package com.prismtech.vortex;

import java.util.List;

import com.prismtech.vortex.parser.IDLParser;

public class TypeDeclSymbol extends TypeDefSymbol
{
  public TypeDeclSymbol
  (
    ScopedName name,
    IDLParser.Type_specContext definition,
    List<IDLParser.Fixed_array_sizeContext> dimensions,
    boolean isint,
    boolean isnonint
  )
  {
    super (name);
    def = definition.getText ();
    this.isint = isint;
    this.isnonint = isnonint;
  }

  public String toString ()
  {
    return "Typedef = " + def;
  }

  public boolean isInteger ()
  {
    return isint;
  }
  public boolean isNonintConst ()
  {
    return isnonint;
  }

  private String def;
  private boolean isint;
  private boolean isnonint;
}
