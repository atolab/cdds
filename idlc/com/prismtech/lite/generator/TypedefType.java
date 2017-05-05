package com.prismtech.lite.generator;

import com.prismtech.lite.ScopedName;
import java.util.*;

public class TypedefType extends AbstractType implements NamedType
{
  public TypedefType (ScopedName name, Type ref)
  {
    this.ref = ref.dup ();
    this.name = new ScopedName (name);
  }

  public Type dup ()
  {
    return new TypedefType (name, ref);
  }

  public ArrayList <String> getMetaOp (String myname, String structname)
  {
    return ref.getMetaOp (myname, structname);
  }

  public String getSubOp ()
  {
    return ref.getSubOp ();
  }

  public String getOp ()
  {
    return ref.getOp ();
  }

  public String getCType ()
  {
    return ref.getCType ();
  }

  public long getKeySize ()
  {
    return ref.getKeySize ();
  }

  public int getMetaOpSize ()
  {
    return ref.getMetaOpSize ();
  }

  public Alignment getAlignment ()
  {
    return ref.getAlignment ();
  }

  public void getToplevelXML (StringBuffer str, ModuleContext mod)
  {
    mod.enter (str, name);
    str.append ("<TypeDef name=\\\"");
    str.append (name.getLeaf ());
    str.append ("\\\">");
    ref.getXML (str, mod);
    str.append ("</TypeDef>");
  }

  public void getXML (StringBuffer str, ModuleContext mod)
  {
    str.append ("<Type name=\\\"");
    str.append (mod.nameFrom (name));
    str.append ("\\\"/>");
  }

  public void populateDeps (Set <ScopedName> depset, NamedType current)
  {
    ref.populateDeps (depset, this);
    depset.add (name);
  }

  public boolean depsOK (Set <ScopedName> deps)
  {
    return TypeUtil.deptest (ref, deps, name);
  }

  public ScopedName getSN ()
  {
    return name;
  }

  public Type getRef ()
  {
    return ref;
  }

  private final Type ref;
  private final ScopedName name;
}
