package com.prismtech.lite.compilers;

public class Idlc
{
  public static void main (String[] args)
  {
    IdlcCmdOptions opts = null;

    try
    {
      opts = new IdlcCmdOptions (args);
    }
    catch (CmdException ex)
    {
      System.exit (ex.retcode);
    }

    com.prismtech.lite.Compiler.run (opts);
  }
}
