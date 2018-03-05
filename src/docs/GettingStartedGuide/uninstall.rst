..
   Copyright(c) 2006 to 2018 ADLINK Technology Limited and others

   This program and the accompanying materials are made available under the
   terms of the Eclipse Public License v. 2.0 which is available at
   http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
   v. 1.0 which is available at
   http://www.eclipse.org/org/documents/edl-v10.php.

   SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause

.. _`Uninstall`:

.. raw:: latex

    \newpage

######################
Uninstalling VortexDDS
######################

*****
Linux
*****

Uninstalling VortexDDS on Linux can be established by invoking
the following two commands (of which the first is optional):
::

    sudo dpkg --remove vortex-dds-dev
    sudo dpkg --remove vortex-dds

.. note::
    Mind the order in which these commands are run. The development
    package (:code:`vortex-dds-dev`) need to be removed first since
    it depends on the library version (:code:`vortex-dds`).

*******
Windows
*******

There are two ways to uninstall VortexDDS from Windows

1. By using the original VortexDDS :ref:`MSI <WindowsInstallMSI>` file
2. By using Windows "Apps & features"

Original MSI
============

Locate the original VortexDDS MSI file on your system and start it.
After clicking :code:`Next`, an overview of options appears, amongst which
is the remove option. By clicking :code:`Remove`, all files and folders are
removed, except the VortexDDS examples (if installed).

Apps & features
===============

Go to :code:`Windows Settings` by clicking the :code:`Settings`-icon ( |settings_icon| )
in the Windows Start Menu. Choose :code:`Apps` in the
:code:`Windows Settings` screen. A list of all installed apps
and programs pops up. Select :code:`VortexDDS` and choose :code:`Uninstall`.
All installed files and folders will be removed, except the
VortexDDS examples (if installed).

.. |settings_icon| image:: ../_static/pictures/settings-icon.png
  :height: 9
  :width: 9
