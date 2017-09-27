Examples
========

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   helloworld/readme
   roundtrip/readme
   throughput/readme

Configuration
*************

To run an example with the default configuration of VortexDDS, no further action is required.
Instructions for running an example can be found using the links above.

To use a different configuration file, you can set the ``VORTEXDDS_URI`` environment variable.

| *Example*
| **Windows:** ``set VORTEXDDS_URI=file://%APPDATA%/VortexDDS/my-config.xml``
| **Linux:** ``export VORTEXDDS_URI="file:///$HOME/my-config.xml"``

Some standard configuration files for common usecases are distributed with VortexDDS installation packages.
You can also create your own configuration files using the VortexDDS Configurator tool.