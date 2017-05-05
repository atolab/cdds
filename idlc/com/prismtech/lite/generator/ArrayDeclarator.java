package com.prismtech.lite.generator;

import java.util.*;

public class ArrayDeclarator
{
  public ArrayDeclarator (String name)
  {
    this.name = name;
    this.dims = new ArrayList <Long> ();
    size = 1;
  }

  public void addDimension (long dim)
  {
    dims.add (new Long (dim));
    size *= dim;
  }

  public String getName ()
  {
    return name;
  }

  public long getSize ()
  {
    return size;
  }

  public String getDimString ()
  {
    StringBuffer result = new StringBuffer ();
    for (Long dim : dims)
    {
      result.append ("[");
      result.append (dim.toString ());
      result.append ("]");
    }
    return result.toString ();
  }

  public Collection <Long> getDims ()
  {
    return dims;
  }

  private final String name;
  private List<Long> dims;
  private long size;
}
