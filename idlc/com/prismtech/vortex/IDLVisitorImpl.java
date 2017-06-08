package com.prismtech.vortex;

import java.io.*;

import org.antlr.v4.runtime.*;

public class IDLVisitorImpl extends com.prismtech.vortex.parser.IDLBaseVisitor
{
  public IDLVisitorImpl (String od)
  {
    outputdir = od;
  }


  private String outputdir;
}
