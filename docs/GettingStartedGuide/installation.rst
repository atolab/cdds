.. _`Installation`:

.. raw:: latex

    \newpage

############
Installation
############

.. contents::


*****
Linux
*****

Ubuntu
======

On Ubuntu and other debian-derived platforms, the product can be installed using a native package.

::

    sudo dpkg -i VortexDDS-<version>-Linux-lib.deb
    sudo dpkg -i VortexDDS-<version>-Linux-dev.deb


Red Hat
=======

Not supported yet (CHAM-326).


Tarball
=======

For more generic Linux installations, different tar-balls (with the same
content) can be provided.

+----------------------------------+---------------------------------------+
| Tarball                          | Description                           |
+==================================+=======================================+
| VortexDDS-<version>-Linux.tar.Z  | Tar Compress compression.             |
+----------------------------------+---------------------------------------+
| VortexDDS-<version>-Linux.tar.gz | Tar GZip compression.                 |
+----------------------------------+---------------------------------------+
| VortexDDS-<version>-Linux.tar.sh | Self extracting Tar GZip compression. |
+----------------------------------+---------------------------------------+

By extracting one of them at any preferred location, VortexDDS can be used.

The following directories are available after the tarball is extracted:

+----------------------------+----------------------------------------------------------+
| Directory                  | Contents                                                 |
+============================+==========================================================+
| ./include                  | Development header files, like dds.h.                    |
+----------------------------+----------------------------------------------------------+
| ./lib                      | VortexDDS libraries, like libvddsc.so                    |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS          | :ref:`CMake <CMakeIntro>` files related to the VortexDDS |
|                            | CMake package                                            |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/docs     | Documentation.                                           |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/examples | Examples (like :ref:`helloworld <HelloWorld>`).          |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/idlc     | :ref:`IDLC Compilder <IdlCompiler>` related files.       |
+----------------------------+----------------------------------------------------------+


.. _`LinuxSetLibPath`:

Paths
=====

To be able to run VortexDDS executables, the required libraries (like
libvddsc.so) need to be available to the executables.
Normally, these are installed in system default locations and it works
out-of-the-box. However, if they are not installed on those locations,
it is possible that the library search path has to be changed.
This can be achieved by executing the command:
::

    export LD_LIBRARY_PATH=<install_dir>/lib:$LD_LIBRARY_PATH


*******
Windows
*******

MSI
===

The default deployment method on Windows is to install the product using the MSI installer.

The installation process is self-explanatory. Two components are available: one for the runtime libraries,
and a seperate component containing files to support development (headers, IDL preprocessor, examples etc.).


ZIP
===

The Windows installation is also provided as a ZIP file. By extracting it
at any preferred location, VortexDDS can be used.

The following directories are available after the zip file is extracted:

+----------------------------+----------------------------------------------------------+
| Directory                  | Contents                                                 |
+============================+==========================================================+
| ./bin                      | VortexDDS dynamic-link libraries, like vddsc.dll.        |
+----------------------------+----------------------------------------------------------+
| ./include                  | Development header files, like dds.h.                    |
+----------------------------+----------------------------------------------------------+
| ./lib                      | VortexDDS libraries, like vddsc.lib                      |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS          | :ref:`CMake <CMakeIntro>` files related to the VortexDDS |
|                            | CMake package                                            |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/docs     | Documentation.                                           |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/examples | Examples (like :ref:`helloworld <HelloWorld>`).          |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/idlc     | :ref:`IDLC Compilder <IdlCompiler>` related files.       |
+----------------------------+----------------------------------------------------------+


.. _`WindowsSetLibPath`:

Paths
=====

To be able to run VortexDDS executables, the required libraries (like
vddsc.dll) need to be available to the executables.
Normally, these are installed in system default locations and it works
out-of-the-box. However, if they are not installed on those locations,
it is possible that the library search path has to be changed.
This can be achieved by executing the command:
::

    set PATH=<install_dir>/bin;%PATH%


*******
License
*******

TODO: CHAM-325

