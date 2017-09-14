.. _`HelloWorld`:

.. raw:: latex

    \newpage

################################
Hello World (building & running)
################################

.. contents::

************
Introduction
************

The Hello World example is an introduction into DDS by creating
a publisher and subscriber and sending a message from the former
to the latter.


.. _`HelloWorldRunning`:

*******************
Running Hello World
*******************

The Hello World example is packaged with pre-built executables
that can normally be found in:

:Windows: :code:`C:\Program Files\PrismTech\DDS\share\VortexDDS\examples\helloworld\bin`
:Linux: :code:`/usr/share/VortexDDS/examples/helloworld/bin`

.. note::
    The installation prefix (:code:`C:\Program Files\PrismTech\DDS` and
    :code:`/usr/`) are determined by the plaform and may be changed during :ref:`installation <Installation>` of the product.

These two executables are called HelloworldPublisher and
HelloworldSubscriber.

After going to the helloworld executables directory, the
HelloworldSubscriber can be started
:ref:`* <HelloWorldRunningNote>` by the following command.

:Windows: :code:`HelloworldSubscriber.exe`
:Linux: :code:`./HelloworldSubscriber`

This will wait until it receives a message from
HelloworldPublisher, which can be started by

:Windows: :code:`HelloworldPublisher.exe`
:Linux: :code:`./HelloworldPublisher`

in a different terminal.
The HelloworldPublisher will display the following:
::

      === [Publisher]  Waiting for a reader to be discovered ...
      === [Publisher]  Writing : Message (1, Hello World)

While the HelloworldSubscriber will print this:
::

      === [Subscriber] Waiting for a sample ...
      === [Subscriber] Received : Message (1, Hello World)


This shows that the message was sent from the publisher to the
waiting subscriber.

.. _`HelloWorldRunningNote`:


\* When the executables do not run due to lacking VortexDDS
libraries, please look at these notes for
:ref:`Windows <WindowsSetLibPath>` and
:ref:`Linux <LinuxSetLibPath>`.



*****************
Hello World Files
*****************

The Hello World example directory (:code:`examples/helloworld`)
contains the following files, which will be explained in more
detail later on.

+--------------------------------+--------------------------------------------+
| File name                      | Description                                |
+================================+============================================+
| publisher.c                    | Publishes the Helloworld message.          |
+--------------------------------+--------------------------------------------+
| subscriber.c                   | Waits and receives the Helloworld message. |
+--------------------------------+--------------------------------------------+
| HelloWorldData.idl             | Datatype description file.                 |
+--------------------------------+--------------------------------------------+
| Makefile                       | Linux native build file.                   |
+--------------------------------+--------------------------------------------+
| HelloWorld.sln                 | Windows Visual Studio solutions file.      |
+--------------------------------+--------------------------------------------+
| CMakeLists.txt                 | CMake build file.                          |
+--------------------------------+--------------------------------------------+
| bin/HelloworldPublisher<.exe>  | Hello World Publisher executable.          |
+--------------------------------+--------------------------------------------+
| bin/HelloworldSubscriber<.exe> | Hello World Subscriber executable.         |
+--------------------------------+--------------------------------------------+
| generated/HelloWorldData.c     | Generated datatype source file.            |
+--------------------------------+--------------------------------------------+
| generated/HelloWorldData.h     | Generated datatype header file.            |
+--------------------------------+--------------------------------------------+
| vs/*                           | Windows Visual Studio support files.       |
+--------------------------------+--------------------------------------------+


Publisher and Subscriber
========================

The publisher.c and subscriber.c contain the source for the
Hello World example :ref:`Publisher <HelloWorldPublisherSource>`
and :ref:`Subscriber <HelloWorldSubscriberSource>` respectively.

The Publisher writes one message that is received by the
Subscriber.


Build Files
===========

Three files are available Hello World root directory to support
building the example. Both
:ref:`Windows native <WindowsNativeBuild>` (HelloWorld.sln) and
:ref:`Linux native <LinuxNativeBuild>` (Makefile) build files
will only be available for this Hello World example. All the
other examples make use of the :ref:`CMake <CMakeIntro>` build
system and thus only have the CMakeLists.txt build related file.


HelloWorldData.idl
==================

To be able to sent data from a writer to a reader, DDS needs to
know the data type. For the Hello World example, this data type
is described in the HelloWorldData.idl file.

Source files are generated from this IDL file that can be used
by the writer and reader of the Hello World example to
communicate the Hello World message.

An explanation of the IDL content and how the source files are
generated is not needed at this point and will be explained in
the `Hello World DataType`_ chapter.

.. _`HelloWorldDataFiles`:

HelloWorldData.c & HelloWorldData.h
===================================

These are the generated files related to the data type of the
messages that are sent and received. How they are generated will
be explained in the further chapter called
`Hello World DataType`_.

While the c source has no interest for the application
developers, HelloWorldData.h contains some information that they
depend on. For example, it contains the actual message structure
that is used when writing or reading data.
::

    typedef struct HelloWorldData_Msg
    {
      int32_t userID;
      char * message;
    } HelloWorldData_Msg;

It also contains convenience macros to allocate and free memory
space for the specific data types.
::

    HelloWorldData_Msg__alloc()
    HelloWorldData_Msg_free(d,o)

It contains an extern variable that describes the data type to
the DDS middleware as well.
::

    HelloWorldData_Msg_desc



******************
Hello World Source
******************

Apart from the
:ref:`HelloWorldData data type files <HelloWorldDataFiles>` that
the Hello World example uses to send messages, it also contains
two source files (:ref:`subscriber.c <HelloWorldPublisherSource>`
and :ref:`publisher.c <HelloWorldSubscriberSource>`) with the
business logic, which is explained in the following chapters.

.. _`HelloWorldSubscriberSource`:

HelloWorld subscriber.c
=======================

This contains the source that will wait for a Hello World
message and reads it when it receives one.

.. literalinclude:: ../../examples/helloworld/subscriber.c
    :linenos:
    :language: c

We will be using the DDS API and the
:ref:`HelloWorldData_Msg <HelloWorldDataFiles>` type
to receive data. For that, we need to include the
appropriate header files.
::

    #include "dds.h"
    #include "HelloWorldData.h"

The main starts with defining a few variables that will be used for
reading the Hello World message.
The entities are needed to create a reader.
::

    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t reader;

Then there are some buffers that are needed to actually read the
data.
::

    HelloWorldData_Msg *msg;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t info[MAX_SAMPLES];

To be able to create a reader, we first need a participant. This
participant is part of a specific communication domain. In the
Hello World example case, it is part of the default domain.
::

    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);

The another requisite is the topic which basically describes the
data type that is used by the reader. When creating the topic,
the :ref:`data description <HelloWorldDataFiles>` for the DDS
middleware that is present in the
:ref:`HelloWorldData.h <HelloWorldDataFiles>` is used.
The topic also has a name. Topics with the same data type
description, but with different names, are considered
different topics. This means that readers/writers created with a
topic named "A" will not interfere with readers/writers created
with a topic named "B".
::

    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc,
                              "HelloWorldData_Msg", NULL, NULL);

When we have a participant and topic, then we can create
the reader. Since the order in which the Hello World Publisher and
Hello World Subscriber are started shouldn't matter, we need to create
a so called 'reliable' reader. Without going into details, the reader
will be created like this
::

    dds_qos_t *qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    reader = dds_create_reader (participant, topic, qos, NULL);
    dds_qos_delete(qos);

We are almost able to read data. However, the read expects an
array of pointers to valid memory locations. This means the
samples array needs initialization. In this example, we have
an array of only one element: :code:`#define MAX_SAMPLES 1`.
So, we only need to initialize one element.
::

    samples[0] = HelloWorldData_Msg__alloc ();

Now everything is ready for reading data. But we don't know if
there is any data. To simplify things, we enter a polling loop
that will exit when data has been read.

Within the polling loop, we do the actual read. We provide the
initialized array of pointers (:code:`samples`), an array that
holds information about the read sample(s) (:code:`info`), the
size of the arrays and the maximum number of samples to read.
Every read sample in the samples array has related information
in the info array at the same index.
::

    ret = dds_read (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);

The :code:`dds_read` function returns the number of samples it
did read. We can use that to determine if the function actually
read some data. When it has, then it is still possible that the
data part of the sample is not valid. This has some use cases
when there is no real data, but still the state of the related
sample has changed (for instance it was deleted). This will
normally not happen in the Hello World example. But we check
for it anyway.
::

    if ((ret > 0) && (info[0].valid_data))

If data has been read, then we can cast the void pointer to the
actual message data type and display the contents. The polling
loop is quit as well in this case.
::

    msg = (HelloWorldData_Msg*) samples[0];
    printf ("=== [Subscriber] Received : ");
    printf ("Message (%d, %s)\n", msg->userID, msg->message);
    break;

When data is received and the polling loop is stopped, we need to
clean up.
::

    HelloWorldData_Msg_free (samples[0], DDS_FREE_ALL);
    dds_delete (participant);

All the entities that are created using the participant are also
deleted. This means that deleting the participant will
automatically the topic and reader as well.


.. _`HelloWorldPublisherSource`:

HelloWorld publisher.c
======================

This contains the source that will write an Hello World message
on which the subscriber is waiting.

.. literalinclude:: ../../examples/helloworld/publisher.c
    :linenos:
    :language: c

We will be using the DDS API and the
:ref:`HelloWorldData_Msg <HelloWorldDataFiles>` type
to sent data. For that, we need to include the
appropriate header files.
::

    #include "dds.h"
    #include "HelloWorldData.h"

Just like with the
:ref:`reader in subscriber.c <HelloWorldSubscriberSource>`,
we need a participant and topic to be able to create a writer.
We use the same topic name as in subscriber.c. Otherwise the
reader and writer are not considered related and data will not
be sent between them.
::

    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t writer;

    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc,
                              "HelloWorldData_Msg", NULL, NULL);
    writer = dds_create_writer (participant, topic, NULL, NULL);

The DDS middleware is a pub/sub implementation. This means that
it will discover related readers and writers and connect them so
that written data can be received by readers, without the
application having to worry about it. There is a catch though:
this discovery and coupling takes a small amount of
time. There are various ways to work around this problem. For
instance by making the readers and writers reliable or wait for
publication/subscription matched events or just don't care if the
reader misses a few samples (f.i. when the publishing frequency is
high enough) or just to sleep (not recommended). In this example we
use a simplified way of waiting for a matching reader to be started.
This is done by polling whether a publication matched status has been
reached. First we need to enable this status on the writer, like
::

    dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);

Now the polling starts:
::

    while(true)
    {
      uint32_t status;
      ret = dds_get_status_changes (writer, &status);
      if (status == DDS_PUBLICATION_MATCHED_STATUS) {
        break;
      }
      /* Polling sleep. */
      dds_sleepfor (DDS_MSECS (20));
    }

After this loop, we are sure that a matching reader has been started.
Now, we commence to writing the data. First the data must be initialized
::

    HelloWorldData_Msg msg;

    msg.userID = 1;
    msg.message = "Hello World";

Then we can actually sent the message to be received by the
subscriber.
::

    ret = dds_write (writer, &msg);

After the sample is written, we need to clean up.
::

    ret = dds_delete (participant);

All the entities that are created using the participant are also
deleted. This means that deleting the participant will
automatically delete the topic and writer as well.



********************
Building Hello World
********************

We recommend using `CMake`_, which is explained in one of the
following chapters. Other examples will make use of that.

However, to start things off, a native way of building
the Hello World example is provided.


.. _`LinuxNativeBuild`:

Linux Native Build
==================

A Linux native :code:`Makefile` is provided in the
:code:`examples/helloworld` directory. In a terminal, go to that
directory and type
::

    make

The build process should have access to the include files and
the vddsc library. The Makefile expects them to be present at
system default locations so that it can find them automatically.
If this isn't the case on your machine, then please
update the :code:`CFLAGS` and :code:`LDFLAGS` within the
Makefile to point to the proper locations.

This will build the HelloworldSubscriber and HelloworldPublisher
executables in the helloworld source directory (not the bin
directory that contains the pre-build binaries).

`Running Hello World`_ example can now be done with the binaries
that were just build. Be sure to use the right directories.


.. _`WindowsNativeBuild`:

Windows Native Build
====================

For the Windows Native Build, a Visual Studio solution file is
available in the :code:`examples/helloworld` directory. Use a
file explorer to navigate to that directory and double click on
the :code:`HelloWorld.sln` file. Visual Studio should now start
with the HelloWorld solution that contains three projects.

+----------------------+-------------------------------------------------+
| Project              | Description                                     |
+======================+=================================================+
| HelloWorldPublisher  | Information to build the example publisher.     |
+----------------------+-------------------------------------------------+
| HelloWorldSubscriber | Information to build the example subcriber.     |
+----------------------+-------------------------------------------------+
| HelloWorldType       | Information to (re)generate                     |
|                      | :ref:`HelloWorldData_Msg <HelloWorldDataFiles>` |
|                      | data type.                                      |
+----------------------+-------------------------------------------------+

Creating the Hello World example executables is as simple as
selecting the required configuration and building the solution.

.. note::
    It is expected that the various Vortex and Hello World
    example files are installed at system default locations. If
    that is not the case, then the file
    :code:`helloworld\vs\directories.props` contains some best
    guesses of where those files could be. This should mean that
    building the HelloWorld solution works out-of-the-box.
    However, if you find that Visual Studio complains about
    header files or libraries that it can not find, then please
    update the information in :code:`helloworld\vs\directories.props`
    to point to the right locations.

To run the example, Visual Studio should run both the publisher
and subscriber simultaneously. It is capable of doing so, but
it's not its default setting. To change it, open the HelloWorld
solution property page by right clicking the solution and
selecting :code:`Properties`. Then go to :code:`Common Properties`
-> :code:`Startup Project`, select :code:`Multiple startup project`
and set :code:`Action "Start"` for HelloWorldPublisher and
HelloWorldSubscriber. Finish the change by selecting :code:`OK`.

Visual Studio is now ready to actually run the Hello World
example, which can be done by selecting :code:`Debug` ->
:code:`Start without debugging`.
Both the subscriber and the publisher will be started and the
publisher will write a message that is received by the subscriber.

Building the solution also (re)generates the
:ref:`HelloWorldData_Msg <HelloWorldDataFiles>` data type files
automatically. Changing that data type is explained in the
following chapter.



********************
Hello World DataType
********************

So far, we haven't touched the actual data type that is sent
from the writer to the reader.


Data-Centric Architecture
=========================

By creating a Data-centric architecture, you get a loosely
coupled information-driven systems. It emphasizes a data layer
that is common for all the distributed applications within the
system. Because there is no direct coupling among the
applications in the DDS model, they can be added and removed
easily in a modular and scalable manner. This makes that the
complexity of a data-centric architecture doesn't really
increase when more and more publishers/subscribers are added.

The HelloWorld example has a very simple 'data layer' of only
one data type :code:`HelloWorldData_Msg`. The subscriber and
publisher are not aware of each other. The former just waits
until somebody provides the data it requires, while the latter
just publishes the data without considering the number of
interested parties. In other words, it doesn't matter for the
publisher if there are none or multiple subscribers. It just
writes the data. The DDS middleware takes care of delivering
the data when needed.


Hello World IDL
===============

There are a few ways to describe the structures that make up the
data layer. The HelloWorld uses the IDL language to describe the
data type in HelloWorldData.idl:

.. literalinclude:: ../../examples/helloworld/HelloWorldData.idl
    :linenos:
    :language: idl

An extensive explanation of IDL lies outside the scope of this
example. Nevertheless, a quick overview of this example is given
anyway.

First, there's the :code:`module HelloWorldData`. This is a kind
of namespace or scope or similar.
Within that module, there's the :code:`struct Msg`. This is the
actual data structure that is used for the communication. In
this case, it contains a :code:`userID` and :code:`message`.

The combination of this module and struct translates to the
following when using the c language.
::

    typedef struct HelloWorldData_Msg
    {
      int32_t userID;
      char * message;
    } HelloWorldData_Msg;

When it is translated to a different language, it will look
different and more tailored towards that language. This is the
advantage of using a data oriented language, like IDL, to
describe the data layer. It can be translated into different
languages after which the resulting applications can communicate
without concerns about the (possible different) programming
languages these application are written in.


.. _`IdlCompiler`:

Generate DataTypes
==================

Like already mentioned in the `Hello World IDL`_ chapter, an IDL
file contains the description of data type(s). This needs to be
translated into programming languages to be useful in the
creation of DDS applications.

To be able to do that, there's a pre-compile step that actually
compiles the IDL file into the desired programming language.

A java application :code:`com.prismtech.vortex.compilers.Idlc`
is supplied to support this pre-compile step. This is available
in :code:`idlc-jar-with-dependencies.jar`

The compilation from IDL into c source code is as simple as
starting that java application with an IDL file. In the case of
the Hello World example, that IDL file is HelloWorldData.idl.
::

    java -classpath "<install_dir>/share/VortexDDS/idlc/idlc-jar-with-dependencies.jar" com.prismtech.vortex.compilers.Idlc HelloWorldData.idl

This seems a bit elaborate, but this step will be done
automatically if you use the :ref:`idlc_generate <IdlcGenerate>`
function in the :ref:`VortexDDS CMake package <VortexDdsPackage>`.
But we get ahead of ourselfs with `CMake`_. Native build targets
have been provided for the Hello World example for your
convenience.

:Windows: The :code:`HelloWorldType` project within the HelloWorld solution.
:Linux: The :code:`make datatype` command.

This will result in new HelloWorldData.c and HelloWorldData.h
files that can be used in the Hello World publisher and
subscriber applications.

The application has to be rebuild when the data type source
files were re-generated.

Again, this is all for the native builds. When using CMake, all
this is done automatically.



*******************
Building With CMake
*******************

In the earlier chapters, building the Hello World example is done
natively. However, the Hello World can also be build using the
CMake tool. This is what is recommended. In fact, all the other
examples don't provide native makefiles, only CMake files.


.. _`CMakeIntro`:

CMake
=====

CMake is an open-source, cross-platform family of tools designed
to build, test and package software. CMake is used to control
the software compilation process using simple platform and
compiler independent configuration files, and generate native
makefiles and workspaces that can be used in the compiler
environment of your choice.

In other words, CMake's main strength is build portability.
CMake uses the native tools, and other than requiring itself,
does not require any additional tools to be installed. The same
CMake input files will build with GNU make, Visual studio 6,7,8
IDEs, borland make, nmake, and XCode.

An other advantage of CMake is building out-of-source. It simply
works out-of-the-box. There are two important reasons to choose
this:

1. Easy cleanup (no cluttering the source tree). Simply remove
   the build directory if you want to start from scratch.
2. Multiple build targets. It's possible to have up-to-date
   Debug and Release targets, without having to recompile the
   entire tree. For systems that do cross-platform compilation,
   it is easy to have up-to-date builds for the host and target
   platform.

There are a few other benefits to CMake, but that is out of the
scope of this document.

.. _`VortexDdsPackage`:

Hello World CMake (VortexDDS Package)
=====================================

After the CMake digression, we're back with the Hello World
example. Apart from the native build files, CMake build files
are provided as well. See
:code:`examples/helloworld/CMakeLists.txt`

.. literalinclude:: ../../examples/helloworld/CMakeLists.export
    :linenos:
    :language: cmake

It will try to find the :code:`VortexDDS` CMake package. When it
has found it, every path and dependencies are automatically set.
After that, an application can use it without fuss. CMake will
look in the default locations for the code:`VortexDDS` package.
If it can not find it, you need to add the location of
:code:`VortexDDSConfig.cmake` to the :code:`CMAKE_PREFIX_PATH`
environment variable.

.. _`IdlcGenerate`:

The :code:`VortexDDS` package provides the :code:`vddsc` library
that contains the DDS API that the application needs. But apart
from that, it also contains helper functionality
(:code:`idlc_generate`) to generate library targets from IDL
files. These library targets can be easily used when compiling
an application that depends on a data type described
in an IDL file.

Two applications will be created, :code:`HelloworldPublisher`
and :code:`HelloworldSubscriber`. Both consist only out of one
source file.

Both applications need to be linked to the :code:`vddsc` library
in the :code:`VortexDDS` package and :code:`HelloWorldData_lib`
that was generated by the call to :code:`idlc_generate`.


.. _`HelloWorldBuilding`:

Hello World Configuration
=========================

The Hello World is prepared to be built by CMake through the use
of its :code:`CMakeLists.txt` file. The first step is letting
CMake configure the build environment.

It's good practice to build examples or applications
out-of-source. In order to do that, create a :code:`build`
directory in the :code:`examples/helloworld` directory and go
there, making our location :code:`examples/helloworld/build`.

Here, we can let CMake configure the build environment for
us by typing:
::

    cmake ../

.. note::
    It is possible that you have to supply a specific generator for
    the configuration, like:
    ::

        cmake -G "Visual Studio 14 2015 Win64" ..

.. note::
    CMake generators can also create IDE environments. For instance,
    the "Visual Studio 14 2015 Win64" will generate a Visual Studio
    solution file. Other IDE's are also possible, like Eclipse.

CMake will use the CMakeLists.txt in the helloworld directory
to create makefiles that fit the native platform.

Since everything is prepared, we can actually build the
applications (HelloworldPublisher and HelloworldSubscriber in
this case).


Hello World Build
=================

After the configuration step, building the example is as easy
as typing:
::

    cmake --build .

.. note::
    On Windows, it is likely that you have to supply the config
    of Visual Studio:
    ::

        cmake --build . --config "Release"

while being in the build directory created during the
configuration step: :code:`examples/helloworld/build`.

The resulting Publisher and Subscriber applications can be found
in:

:Windows:  :code:`examples\helloworld\build\Release`.
:Linux: :code:`examples/helloworld/build`.

`Running Hello World`_ example can now be done with the
binaries that were just build. Be sure to use the right
directories.

