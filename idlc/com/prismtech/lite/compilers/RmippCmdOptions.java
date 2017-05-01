package com.prismtech.lite.compilers;

public class RmippCmdOptions extends CmdOptions
{
  public RmippCmdOptions (String[] args) throws CmdException
  {
    super ("rmipp", args);
  }

  public void optHelp (java.io.PrintStream io)
  {
    super.optHelp (io);
    io.println ("   -t QosFile       XML file specifying QoS");
  }

  public boolean process (String arg1, String arg2) throws CmdException
  {
    boolean result = false;
    if (arg1.equals ("-t"))
    {
      if (arg2 == null || arg2.charAt (0) == '-')
      {
        System.err.println
          (name + ": QoS filename expected following -t option");
        throw new CmdException (1);
      }
      qosfile = arg2;
      result = true;
    }
    else if (arg1.equals ("-l"))
    {
      if (arg2 == null || arg2.charAt (0) == '-')
      {
        System.err.println
          (name + ": target language expected following -l option");
        throw new CmdException (1);
      }
      if (!arg2.equals ("cpp") && !arg2.equals ("c++"))
      {
        System.err.println (name + ": LITE supports C++ only");
        throw new CmdException (1);
      }
      result = true;
    }
    else
    {
      result = super.process (arg1, arg2);
    }
    return result;
  }

  public String qosfile;
}
