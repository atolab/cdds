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

The Hello World example is packaged with pre-build executables
that can be found :code:`examples/helloworld/bin` directory.
These two executables are called HelloworldPublisher and
HelloworldSubscriber.

To be able to run these two executables, the vddsc library needs
to be available and can be found by the executables.
This can mean that the library search path has to be changed if
the library is not installed on a system default location.
For instance, by executing the command

:Windows: :code:`set PATH=../../../lib;%PATH%`
:Linux: :code:`export LD_LIBRARY_PATH=../../../lib:$LD_LIBRARY_PATH`

When the library is available, then the HelloworldSubscriber can
be started by going to the executables directory.

:Windows: :code:`HelloworldSubscriber.exe`
:Linux: :code:`./HelloworldSubscriber`

This will wait until it receives a message from
HelloworldPublisher, which can be started by

:Windows: :code:`HelloworldPublisher.exe`
:Linux: :code:`./HelloworldPublisher`

in a different terminal.
The HelloworldPublisher will display the following:
::

    === [Publisher] Writing : Message (1, Hello World)

While the HelloworldSubscriber will print this:
::

    === [Reader] waiting for a message ...
    === [Subscriber] Received : Message (1, Hello World)

This shows that the message was send from the publisher to the
waiting subscriber.


******************
Hello World Source
******************

The Hello World example source code can be found in the
:code:`examples/helloworld directory`. It consists out of the
following files, which will be explained in more detail later on.

+----------------------------+--------------------------------------------+
| File name                  | Description                                |
+============================+============================================+
| publisher.c                | Publishes the Helloworld message.          |
+----------------------------+--------------------------------------------+
| subscriber.c               | Waits and receives the Helloworld message. |
+----------------------------+--------------------------------------------+
| HelloWorldData.idl         | Datatype description file.                 |
+----------------------------+--------------------------------------------+
| generated/HelloWorldData.c | Generated datatype file.                   |
+----------------------------+--------------------------------------------+
| generated/HelloWorldData.h | Generated datatype file.                   |
+----------------------------+--------------------------------------------+


HelloWorldData.idl
==================

To be able to send data from a writer to a reader, DDS needs to
know the data type. For the Hello World example, this data type
is described in the HelloWorldData.idl file.

Source files are generated from this idl file that can be used
by the writer and reader of the Hello World example to
communicate the Hello World message.

An explanation of the idl content and how the source files are
generated is not really important now and will be explained in
the `Hello World DataType`_ chapter.


HelloWorldData.c & HelloWorldData.h
===================================

These are generated files. How they are generated is not really
important at this point and will be explained later in the
`Hello World DataType`_ chapter. The c source is normally of
no interest to the application developer anyway.

The HelloWorldData.h, however, does contain some information
that the application developer depends on. For one, it contains
the actual message structure that is used when writing or
reading data.
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

It also contains an extern variable that describes the data type
to the DDS middleware.
::

    HelloWorldData_Msg_desc


HelloWorld subscriber.c
=======================

This contains the source that will wait for a Hello World
message and reads it when it receives one.

.. literalinclude:: ../../examples/helloworld/subscriber.c
    :linenos:
    :language: c

We will be using the dds API and the HelloWorldData_Msg data
type to send and receive. For that, we need to include the
appropriate header files
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

We also need a topic. This basically describes the data type
that is used by the reader. Here we see the data type
description for the DDS middleware that is present in the
HelloWorldData.h. The topic also has a name. Topics with the
same data type description, but with other names, are considered
different. This means that readers/writers created with topic
with name A will not interfere with readers/writers created with
topic with name B, even though they use the same data type.
::

    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);

When we have a participant and topic, then we can create
the reader.
::

    reader = dds_create_reader (participant, topic, NULL, NULL);

We are almost able to read data. However, the read expects an
array of pointers to valid memory locations. This means the
samples array needs initialization. In this example, we have
an array of only one element: :code:`#define MAX_SAMPLES 1`.
So, we only need to initialize one element.
::

    samples[0] = HelloWorldData_Msg__alloc ();

Now everything is ready for reading data. But we don't know if
there is any data. So, we enter a polling loop that will exit
when data has been read.

Within the polling loop, we do the actual read. We provide the
initialized array of pointers (:code:`samples`), an array that
holds information about the read sample(s) (:code:`info`), the
size of the arrays and the maximum number of samples to read.
Every read sample in the samples array has related information
in the info array at the very same index.
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

When data is received and the polling loop is quit, we need to
clean up.
::

    HelloWorldData_Msg_free (samples[0], DDS_FREE_ALL);
    dds_delete (participant);

Deleting the participant meant that the reader (which is its
child) is deleted as well.


HelloWorld publisher.c
======================

This contains the source that will write an Hello World message
on which the subscriber was waiting.

.. literalinclude:: ../../examples/helloworld/publisher.c
    :linenos:
    :language: c

We will be using the dds API and the HelloWorldData_Msg data
type to send and receive. For that, we need to include the
appropriate header files
::

    #include "dds.h"
    #include "HelloWorldData.h"

Just like with the reader in subscriber.c, we need a participant
and topic to be able to create a writer. We use the same topic
name as in subscriber.c. Otherwise the reader and writer are not
considered related and data will not be send between them.
::

    dds_entity_t participant;
    dds_entity_t topic;
    dds_entity_t writer;

    participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
    topic = dds_create_topic (participant, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
    writer = dds_create_writer (participant, topic, NULL, NULL);

The DDS middleware is a pub/sub implementation. This means that
it will discover related readers and writers and connect them so
that written data can be received by readers, without the
application to worry about it. There is a catch though and that
is that this discovery and coupling takes a small amount of
time. There are various ways to work around this problem. For
instance by making the readers and writers reliable or wait for
publication/subscription matched events or just don't care if
the reader misses a few samples (f.i. when the publishing
frequency is high enough). However, that is out of the scope of
this example and to keep things simple, we just do a sleep.
::

    /* Sleep to allow discovery of reader by writer and vice versa. */
    dds_sleepfor (DDS_SECS (1));

.. note::
    As mentioned in the previous paragraph, sleeping isn't really
    recommended. See :ref:`Hello Quick World <HelloQuickWorld>`
    example for an alternative.

Now, we need to decide what data to send.
::

    HelloWorldData_Msg msg;

    msg.userID = 1;
    msg.message = "Hello World";

Then we can actually send the message for the subscriber to receive.
::

    ret = dds_write (writer, &msg);

After the sample is written, we need to clean up.
::

    ret = dds_delete (participant);

Deleting the participant meant that the writer (which is its
child) is deleted as well.



********************
Building Hello World
********************

We recommend using `CMake`_, which is explained in a
following chapter. Other examples will make use of that.

However, to kick things off, a native way of building
the Hello World example is provided.


Linux Native Build
==================

A Linux native Makefile is provided in the 
:code:`examples/helloworld` directory. Just go to that
directory and type 
::

    make

Be sure, however, that the include files and vddsc library can
be found by the build process. The Makefile expects them to be
present at system default locations or at :code:`../../lib` and
:code:`../../include` relative to the :code:`examples/helloworld`
directory. If this isn't the case on your machine, then please
update the :code:`CFLAGS` and :code:`LDFLAGS` within the
Makefile to point to the proper locations.

This will build the HelloworldSubscriber and HelloworldPublisher
executables in the helloworld source directory (not the bin
directory that contains the pre-build binaries).

`Running Hello World`_ example can now be done with the
binaries that were just build. Be sure to use the right
directories though.


Windows Native Build
====================

TODO



********************
Hello World DataType
********************

So far, we haven't touched the actual data type that is send
from the writer to the reader.


Data-Centric Architecture
=========================

By creating a Data-centric architecture, you get a loosely
coupled information-driven systems. It emphasizes a data layer
that is common for all the distributed applications within the
system. Because there is no direct coupling among the
applications in the DDS model, they can be added and removed
easily in a modular and scalable manner. This makes that a
data-centric architecture doesn't really increase in complexity
when the amount of data is increased.

The HelloWorld has a very simple 'data layer' of only one data
type :code:`HelloWorldData_Msg`. The subscriber and publisher
don't really know of each other. The former just waits until
somebody provides the data it requires, while the latter just
publishes the data without worrying if there are interested
parties or how many.


Hello World IDL
===============

There are a few ways to describe the structures that make up the
data layer. The HelloWorld uses the idl language to describe the
data type in HelloWorldData.idl:

.. literalinclude:: ../../examples/helloworld/HelloWorldData.idl
    :linenos:
    :language: idl

An extensive explanation of idl is outside the scope of this
example. A quick overview of the example is given anyway.

First, there's the :code:`module HelloWorldData`. This is kind
of a namespace or scope or similar.
Within that module, there's the :code:`struct Msg`. This is the
actual data structure that used for the communication. In this
case, it contains a :code:`userID` and :code:`message`.

The combination of this module and struct translates to the
following when using the c language.
::

    typedef struct HelloWorldData_Msg
    {
      int32_t userID;
      char * message;
    } HelloWorldData_Msg;

When it is translate to a different language, it will look
different and more tailored towards that language. This is the
advantage of using a data oriented language, like idl, to
describe the data layer. It can be translated into different
languages after which the resulting applications can communicate
without concerns about said programming languages.

A bit more information about the :code:`#pragma keylist` can be
found :ref:`here <HelloInstanceWorld>`.

.. _`IdlCompiler`:

IDL Precompiler
===============

Like already said, an idl file needs to be translated to another
programming language in which the communication application will
be written.

To be able to do that, there's a pre-compile step that actually
compiles the idl file into the desired programming language.

The tool :code:`dds_idlc` (idl compiler) is supplied for this.

The compilation from idl into c source code is as simple as
calling dds_idlc with the idl file.
::

    dds_idlc HelloWorldData.idl

This will result in new HelloWorldData.c and HelloWorldData.h
files that can be used in the Hello World publisher and
subscriber applications.


Build DataTypes
===============

If the idl file is updated, the dds_idlc needs to be called to
create the related source files.

For the Hello World example, the native build processes are
extended to be able to generate the c source code.

:Windows: :code:`TODO`
:Linux: :code:`make datatype`

This will result in new HelloWorldData.c and HelloWorldData.h in
the generated directory.

The applications are not automatically build after new data type
files are generated. To get the new data types to take effect,
the applications themselves need to be rebuild.

After that, the Hello World example will use the new data types.



*******************
Building With CMake
*******************

So far, we've been talking about building the Hello World example
natively. However, the Hello World can also be build using the
cmake tool. This is what is recommended. In fact, all the other
examples don't provide native makefiles, only cmake files.


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

An other strength is that CMake is building out-of-source. It
simply works out-of-the-box. There are two important reasons why
you would want this.

1. Easy cleanup (no cluttering the source tree). Simply remove
   the build directory if you want to start from scratch.
2. Multiple build targets. It's possible to have up-to-date
   Debug and Release targets, without having to recompile the
   entire tree. For systems that do cross-platform compilation,
   it is easy to have up-to-date builds for the host and target
   platform.

There are a few other benefits to CMake, but that is out of the
scope of this document.

Hello World CMake
=================

After the CMake digression, we're back with the Hello World
example. Apart from the native build files, CMake build files
are provided as well. See
:code:`examples/helloworld/CMakeLists.txt`

.. literalinclude:: ../../examples/helloworld/CMakeLists.export
    :linenos:
    :language: cmake

It will try to find the :code:`VortexDDS` CMake package. When it
has found it, every path and dependencies are automatically set
so that an application can use it without fuss. If it can not
find it, please add the location of :code:`VortexDDSConfig.cmake`
to the :code:`CMAKE_PREFIX_PATH` environment variable.

The :code:`VortexDDS` package provides the :code:`vddsc` library
that contains the DDS API that the application needs. But apart
from that, it also contains helper functionality to generate
library targets from idl files that can be easily used when
compiling an application that depends on a data type described
in such an idl file. To be able to do this, the :code:`VortexDDS`
package tries to access the :code:`dds_idlc` tool. If it can't
find it, a warning will be issued during the CMake configuration
step. An error will be triggered when the build process tries to
use :code:`dds_idlc` while it wasn't found before. If that
happens, please add the :code:`dds_idlc` location to the
:code:`CMAKE_PREFIX_PATH` environment variable.

Two applications will be created, both of which only consists
out of 1 source file.

Both applications need to be linked to the vddsc library in the
VortexDDS package and the just generated HelloWorldData_lib.


.. _`HelloWorldBuilding`:

Hello World Configuration
=========================

The Hello World is prepared to be build by CMake through the use
of its :code:`CMakeLists.txt` file. The first step is letting
CMake configure the build environment.

The location where the configure step is executed, will be the
root for the build directory. It's good practice to build
examples (or anything for that matter) out-of-source. To do
that, create a :code:`build` directory in the 
:code:`examples/helloworld` directory and go there, making
our location :code:`examples/helloworld/build`.

Here, we can let CMake configure the build environment for
us by typing
::

    cmake ../

CMake will use the CMakeLists.txt in the helloworld directory
to create makefiles that fit the native platform.

Now that everything is prepared, we can actually build the
applications (HelloworldPublisher and HelloworldSubscriber in
this case).


Hello World Build
=================

After the configuration step, building the example is as easy
as typing
::

    cmake --build .

while being in the the build directory created during the
configuration step. In this example, that directory would be
:code:`examples/helloworld/build`.

After the build finished, both the Hello World publisher and
subscriber applications are present within the build directory.

`Running Hello World`_ example can now be done with the
binaries that were just build. Be sure to use the right
directories though.

