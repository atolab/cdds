package com.prismtech.vortex.compilers;

import java.util.*;
import com.prismtech.vortex.Version;
//import com.prismtech.DDS_Service.idl2smodel;

public class Idlcpp
{
  public static void main (String[] args)
  {
    IdlcppCmdOptions opts = null;
    int status = 0;

    try
    {
      opts = new IdlcppCmdOptions (args);
    }
    catch (CmdException ex)
    {
      System.exit (ex.retcode);
    }

    List <String> idlppcmd = new ArrayList <String> ();

    if (opts.version)
    {
      System.out.print ("Vortex DDS ");
      System.out.println ("IDL to C++ compiler v" + Version.version);
    }
    else
    {
      String FS = System.getProperty ("file.separator");
      String litehome = System.getProperty ("LITE_HOME");
      String litehost = System.getProperty ("LITE_HOST");

      opts.includes.add (litehome + FS + "etc" + FS + "idl");

      IdlcCmdOptions idlcopts = new IdlcCmdOptions (opts);

      if (!opts.pponly)
      {
         idlppcmd.add (litehome + FS + "bin" + FS + litehost + FS + "idlpp");
         idlppcmd.add ("-S");
         idlppcmd.add ("-a");
         idlppcmd.add (litehome + FS + "etc" + FS + "idlpp");
         idlppcmd.add ("-x");
         idlppcmd.add ("lite");

         idlppcmd.add ("-l");
         if (opts.language == null)
         {
           idlppcmd.add ("isoc++");
         }
         else
         {
           idlppcmd.add (opts.language);
         }

         if (opts.dllname != null)
         {
           idlppcmd.add ("-P");
           if (opts.dllfile != null)
           {
             idlppcmd.add (opts.dllname + "," + opts.dllfile);
           }
           else
           {
             idlppcmd.add (opts.dllname);
           }
         }
         if (opts.outputdir != null)
         {
           idlppcmd.add ("-d");
           idlppcmd.add (opts.outputdir);
         }
         for (String s : opts.includes)
         {
           idlppcmd.add ("-I");
           idlppcmd.add (s);
         }
         for (String s : opts.macros.keySet ())
         {
           idlppcmd.add ("-D");
           String val = opts.macros.get (s);
           if (!val.equals (""))
           {
             idlppcmd.add (s + "=" + val);
           }
           else
           {
             idlppcmd.add (s);
           }
         }

         if (opts.testmethods)
         {
           idlppcmd.add ("-T");
         }

         idlppcmd.addAll (opts.files);

         status = runcmd (idlppcmd);
      }
      com.prismtech.vortex.Compiler.run (idlcopts);
    }
    System.exit (status);
  }

  private static int runcmd (List<String> cmdline)
  {
    int result;
    try
    {
      result = new ProcessBuilder (cmdline).inheritIO ().start (). waitFor ();
      if (result != 0)
      {
        System.err.print ("dds_idlcpp: nonzero return from");
        for (String s : cmdline)
        {
          System.err.print (" " + s);
        }
        System.err.println ();
      }
    }
    catch (Exception ex)
    {
      System.err.print ("dds_idlcpp: exception when running");
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
}

