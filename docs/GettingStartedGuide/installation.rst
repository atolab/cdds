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

Debian
======

| TODO: (CHAM-322)
| The contents of the Debian packages are not correct yet. So, it's a bit
  hard to write this chapter, but it would probably involve
| sudo apt-get install
| and such.


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

Extract one of these at a location of your liking and VortexDDS can be
used.

The tarball contains these directories of importance:

+-----------------------+----------------------------------------------------------+
| Directory             | Contents                                                 |
+=======================+==========================================================+
| ./bin                 | Tools, like :ref:`dds_idlc <IdlCompiler>`.               |
+-----------------------+----------------------------------------------------------+
| ./examples            | DDS usage examples, like :ref:`helloworld <HelloWorld>`. |
+-----------------------+----------------------------------------------------------+
| ./include             | Development header files, like dds.h.                    |
+-----------------------+----------------------------------------------------------+
| ./lib                 | VortexDDS libraries, like libvddsc.so                    |
+-----------------------+----------------------------------------------------------+
| ./lib/cmake/VortexDDS | :ref:`CMake <CMakeIntro>` files related to the VortexDDS |
|                       | CMake package.                                           |
+-----------------------+----------------------------------------------------------+


*******
Windows
*******

TODO: (this ticket)


*******
License
*******

TODO: CHAM-325

