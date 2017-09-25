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
is the remove option. By clicking :code:`Remove`, everything the installer
installed will be removed. The installer will leave the user-installed
files untouched.

Apps & features
===============

Go to :code:`Windows Settings` by clicking the :code:`Settings`-icon ( |settings_icon| )
in the Windows Start Menu. Choose :code:`Apps` in the
:code:`Windows Settings` screen. A list of all installed apps
and programs pops up. Select :code:`VortexDDS` and choose :code:`Uninstall`.


.. |settings_icon| image:: ../_static/pictures/settings-icon.png
  :height: 9
  :width: 9
