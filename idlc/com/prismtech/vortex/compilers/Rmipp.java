package com.prismtech.vortex.compilers;

import java.util.*;
import com.prismtech.vortex.Version;
import com.prismtech.DDS_Service.idl2smodel;

public class Rmipp
{
  public static void main (String[] args)
  {
    RmippCmdOptions opts = null;
    int status = 0;

    try
    {
      opts = new RmippCmdOptions (args);
    }
    catch (CmdException ex)
    {
      System.exit (ex.retcode);
    }

    /* TODO: Redo rmipp so that it can take 'opts' directly. Until then,
       build a command line */

    List <String> commonopts = new ArrayList <String> ();
    List <String> rmippopts = new ArrayList <String> ();
    List <String> idlppcmd1 = new ArrayList <String> ();
    List <String> idlppcmd2 = new ArrayList <String> ();

    if (opts.version)
    {
      /* TODO: Would be nice to get a version number out of rmipp */
      System.out.print ("Vortex DDS ");
      System.out.println ("RMI preprocessor driver v" + Version.version);
    }
    else
    {
      String litehome = System.getProperty ("LITE_HOME");
      String litehost = System.getProperty ("LITE_HOST");
      opts.includes.add (litehome + FS + "etc" + FS + "idl");

      IdlcCmdOptions idlcopts1 = new IdlcCmdOptions (opts);
      IdlcCmdOptions idlcopts2 = new IdlcCmdOptions (opts);
      idlcopts2.files.clear ();

      commonopts.add ("-l");
      commonopts.add ("c++");
      if (opts.dllname != null)
      {
        commonopts.add ("-P");
        if (opts.dllfile != null)
        {
          commonopts.add (opts.dllname + "," + opts.dllfile);
        }
        else
        {
          commonopts.add (opts.dllname);
        }
      }
      if (opts.outputdir != null)
      {
        commonopts.add ("-d");
        commonopts.add (opts.outputdir);
      }
      for (String s : opts.includes)
      {
        commonopts.add ("-I");
        commonopts.add (s);
      }
      for (String s : opts.macros.keySet ())
      {
        commonopts.add ("-D");
        String val = opts.macros.get (s);
        if (!val.equals (""))
        {
          commonopts.add (s + "=" + val);
        }
        else
        {
          commonopts.add (s);
        }
      }

      if (opts.qosfile != null)
      {
        rmippopts.add ("-t");
        rmippopts.add (opts.qosfile);
      }
      rmippopts.addAll (commonopts);
      rmippopts.addAll (opts.files);

      idlppcmd1.add (litehome + FS + "bin" + FS + litehost + FS + "idlpp");
      idlppcmd1.add ("-S");
      idlppcmd1.add ("-a");
      idlppcmd1.add (litehome + FS + "etc" + FS + "idlpp");
      idlppcmd1.add ("-x");
      idlppcmd1.add ("lite");

      idlppcmd1.addAll (commonopts);
      idlppcmd1.addAll (opts.files);

      idlppcmd2.add ("idlpp");
      idlppcmd2.add ("-S");
      idlppcmd2.add ("-a");
      idlppcmd2.add (litehome + FS + "etc" + FS + "idlpp");
      idlppcmd2.add ("-x");
      idlppcmd2.add ("lite");

      for (String s : opts.files)
      {
        int i = s.lastIndexOf (FS);
        if (i == -1)
        {
          idlppcmd2.add ("-I.");
          idlcopts2.includes.add (".");
        }
        else
        {
          idlppcmd2.add ("-I" + s.substring (0, i));
          idlcopts2.includes.add (s.substring (0, i));
        }
      }

      idlppcmd2.addAll (commonopts);

      String f2prefix = (opts.outputdir == null) ? "" : opts.outputdir + FS;
      for (String s : opts.files)
      {
        String f2name =
          f2prefix +
          stripIdl (s.substring (s.lastIndexOf (FS) + 1)) +
          "_Topics.idl";
        idlppcmd2.add (f2name);
        idlcopts2.files.add (f2name);
      }

      idl2smodel.main (rmippopts.toArray (new String[0]));
      com.prismtech.vortex.Compiler.run (idlcopts1);
      com.prismtech.vortex.Compiler.run (idlcopts2);
      status = runcmd (idlppcmd1);
      if (status == 0)
      {
        status = runcmd (idlppcmd2);
      }
    }
    System.exit (status);
  }

  private static String stripIdl (String in)
  {
    if (in.endsWith (".idl"))
    {
      return in.substring (0, in.length () - 4);
    }
    else
    {
      return in;
    }
  }

  private static int runcmd (List<String> cmdline)
  {
    int result;
    try
    {
      result = new ProcessBuilder (cmdline).start (). waitFor ();
      if (result != 0)
      {
        System.err.print ("rmipp: nonzero return from");
        for (String s : cmdline)
        {
          System.err.print (" " + s);
        }
        System.err.println ();
      }
    }
    catch (Exception ex)
    {
      System.err.print ("rmipp: exception when running");
      for (String s : cmdline)
      {
        System.err.print (" " + s);
      }
      System.err.println ();
      System.err.println (ex);
      result = 1;
    }
    return result;
  }

  private static String FS = System.getProperty ("file.separator");
}

