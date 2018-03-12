# Cyclone DDS

Cyclone DDS (cdds) is by far the most performant and robust DDS implementation available on the market. 

Beside, Cyclone DDS is developed completely in the open and is undergoing the acceptance process to become part of Eclipse IoT (see  [eclipse-cyclone-dds](https://projects.eclipse.org/proposals/eclipse-cyclone-dds)).


# Getting Started
## Building Cyclone DDS

In order to build cyclone DDS you need to have installed on your host [cmake](https://cmake.org) **v3.5.0** or higher and either the [Java 8 JDK](http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html) or simply the [Java 8 RE](http://www.oracle.com/technetwork/java/javase/downloads/server-jre8-downloads-2133154.html).

Assuming that **git** is also available on your machine then, simply do:

    $ git clone git@github.com:atolab/cdds.git
    $ cd cdds
    $ mkdir build
    $ cmake ../src
    $ make
    $ make install

At this point you are ready to use **cdds** for your next DDS project!
 

