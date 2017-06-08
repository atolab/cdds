package com.prismtech.vortex.generator;

public abstract class AbstractType implements Type
{
  public void makeKeyField ()
  {
    iskey = true;
  }

  public boolean isKeyField ()
  {
    return iskey;
  }

  private boolean iskey = false;
}
