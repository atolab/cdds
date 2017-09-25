.. _`Installation`:

.. raw:: latex

    \newpage

#################
Install VortexDDS
#################

.. .. contents::


.. _`SystemRequirements`:

*******************
System requirements
*******************

Currently Prismtech VortexDDS is supported on the following platforms:

+-------------------+--------------+--------------------+
| Operating systems | Architecture | Compiler           |
+===================+==============+====================+
| Ubuntu 16.04 LTS  | 64-bit       | gcc 5.4 or later   |
+-------------------+--------------+--------------------+
| Windows 10        | 64 -bit      | VS2015             |
+-------------------+--------------+--------------------+



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
content) are provided.

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
| ./share/VortexDDS/examples | Examples (like :ref:`Hello World! <HelloWorld>`).        |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/idlc     | :ref:`IDLC Compiler <IdlCompiler>` related files.        |
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

.. _`WindowsInstallMSI`:

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
| ./share/VortexDDS/examples | Examples (like :ref:`Hello World! <HelloWorld>`).        |
+----------------------------+----------------------------------------------------------+
| ./share/VortexDDS/idlc     | :ref:`IDLC Compiler <IdlCompiler>` related files.        |
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


.. _`TestYourInstallation`:

**********************
Test your installation
**********************

The installation provides a simple prebuilt :ref:`Hello World! <HelloWorld>` application which
can be run in order to test your installation. The *Hello World!* application consists of two
executables: a so called HelloworldPublisher and a HelloworldSubscriber, typically located in
:code:`/usr/share/VortexDDS/examples/helloworld/bin` on Linux and in
:code:`C:\Program Files\PrismTech\DDS\share\VortexDDS\examples\helloworld\bin` on Windows.

To run the example application, please open two console windows and navigate to the appropriate
directory in both console windows. Run the HelloworldSubscriber in one of the console windows by the
typing following command:

  :Windows: :code:`HelloworldSubscriber.exe`
  :Linux: :code:`./HelloworldSubscriber`

and the HelloworldPublisher in the other console window by typing:

  :Windows: :code:`HelloworldPublisher.exe`
  :Linux: :code:`./HelloworldPublisher`


The output HelloworldPublisher should look like

.. image:: ../_static/pictures/HelloworldPublisherWindows.png

while the HelloworldSubscriber will be looking like this

.. image:: ../_static/pictures/HelloworldSubscriberWindows.png

For more information on how to build this application your own and the code which has
been used, please have a look at the :ref:`Hello World! <HelloWorld>` chapter.

When the executables do not run due to lacking VortexDDS
libraries, please look at these notes for
:ref:`Windows <WindowsSetLibPath>` and
:ref:`Linux <LinuxSetLibPath>`.

*******
License
*******

TODO: CHAM-325

