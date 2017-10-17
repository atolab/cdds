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

Vortex DDS has various configuration parameters and comes with a default built-in configuration.
To run an example, or any application that uses Vortex DDS for its data exchange, this default
configuration is usually fine and no further action is required.

Configuration parameters for VortexDDS are expressed in XML and grouped together in a single XML file.
To use a custom XML configuration in an application, the ``VORTEXDDS_URI`` environment variable needs
to be set prior to starting the application and pointed to the location of the configuration file to
be used.

| *Example*
| **Windows:** ``set VORTEXDDS_URI=file://%USERPROFILE%/VortexDDS/my-config.xml``
| **Linux:** ``export VORTEXDDS_URI="file://$HOME/VortexDDS/my-config.xml"``

The VortexDDS installation comes with a set of standard configuration files for common use cases.
You update existing configuration files or create your own by using the VortexDDS Configurator tool,
which provides context-sensitive help on available configuration parameters and their applicability.

You can start the VortexDDS Configuration tool through the VortexDDS Launcher, or from your command-prompt
by entering the tools directory and running ``java -jar vortexddsconf.jar``. The default location of the tools
directory is ``/usr/share/VortexDDS/tools`` on Linux or ``C:\Program Files\ADLINK\Vortex DDS\share\VortexDDS\tools``
on Windows.
