package com.prismtech.lite.compilers;

public class CmdException extends Exception
{
  public CmdException (int retcode)
  {
    this.retcode = retcode;
  }

  public final int retcode;
}
