/*
 * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
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
