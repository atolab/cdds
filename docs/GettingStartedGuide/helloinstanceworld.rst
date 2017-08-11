.. _`HelloInstanceWorld`:

.. raw:: latex

    \newpage

############################################
Hello Instance World (data states & history)
############################################

.. contents::

************
Introduction
************

In this chapter, it is assumed that the reader has knowledge about the
:ref:`Hello World <HelloWorld>`and :ref:`Hello Quick World <HelloQuickWorld>`
examples.

In the previous examples, only one message is sent and received. In the
Hello Instance World example, more than one messages are sent and
different states of data are explored.

:ref:`Building <HelloWorldBuilding>` this example is done through
:ref:`CMake <CMakeIntro>` and executing it is a bit like
:ref:`running the Hello World <HelloWorldRunning>` example, except
the binaries can not be found in the pre-compiled bin directory,
but in the CMake build directory.


**************
Data Instances
**************

The Hello Intance World uses the HelloWorldData type, which is described
in the HelloWorldData.idl file:

.. literalinclude:: ../../examples/helloinstanceworld/HelloWorldData.idl
    :linenos:
    :language: idl

There's one line in that IDL file that determines the key:
::

    #pragma keylist Msg userID

The key for the :code:`Msg` struct is the :code:`userId`
variable. A key can consist out of multiple struct variables.
However, in this example, only one is used.

Samples with identical keys will be grouped together into one
instance. This means that samples with the same :code:`userId` will
be considered to be part of the same instance.

Multiple topics can be created with the same data type by using different
names per topic. Samples with the same keys but different topics will be
considered different instances. So, an instance is related to the data
type key and the topic. Therefore, it is also called topic instances.

Topic instances are runtime entities for which DDS keeps track of whether

- there are any live writers,
- the instance has appeared in the system for the first time, and
- the instance has been disposed (explicitly removed from the system).

Topic instances impact the organization of data on the reader side as well
as the memory usage. Furthermore, there are some QoSs that apply at the
instance level. The History QoS policy is one of them.


**************
Reader History
**************

Every reader has a history in which samples can be available. The default
setting of a reader will keep the last sample of every instance. This
means that a new sample of a particular instance will replace any
existing older sample in the reader history.
See the 3rd read in `States Examples`_ where this takes place.

It is possible to change the number of samples per instance that the
reader keeps in its history.
::

    dds_qos_t *qos;
    qos = dds_qos_create ();
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 2);

If a reader is created with this QoS, then it will keep the last 2 samples
of every instance within its history.

    \newpage

**************
States Summary
**************

Every sample has mainly three states related to it. Two of these states
are related to the instance, which is shown in the table below.

+-----------------+------------------------------+-----------------------------------------------+
| State           | Value                        | Description                                   |
+=================+==============================+===============================================+
| sample_state    |                 DDS_SST_READ | This sample has never been read before read   |
|                 |                              | by the related reader.                        |
|                 +------------------------------+-----------------------------------------------+
|                 |             DDS_SST_NOT_READ | This sample has been read in a previous read  |
|                 |                              | of the related reader.                        |
+-----------------+------------------------------+-----------------------------------------------+
| view_state      |                  DDS_VST_NEW | The related instance of this sample has never |
|                 |                              | been seen before by the related reader.       |
|                 +------------------------------+-----------------------------------------------+
|                 |                  DDS_VST_OLD | The related instance of this sample has been  |
|                 |                              | seen before by the related reader.            |
+-----------------+------------------------------+-----------------------------------------------+
| instance_state  |                DDS_IST_ALIVE | The related instance of this sample has not   |
|                 |                              | been disposed nor unregistered.               |
|                 +------------------------------+-----------------------------------------------+
|                 |   DDS_IST_NOT_ALIVE_DISPOSED | | The related instance of this sample has     |
|                 |                              |   been disposed.                              |
|                 |                              | | Disposed means that the instance is not     |
|                 |                              |   valid anymore.                              |
|                 +------------------------------+-----------------------------------------------+
|                 | DDS_IST_NOT_ALIVE_NO_WRITERS | | The related instance of this sample has     |
|                 |                              |   been unregistered.                          |
|                 |                              | | Unregistered means that the instance is     |
|                 |                              |   still valid, but the writer will not update |
|                 |                              |   it anymore.                                 |
+-----------------+------------------------------+-----------------------------------------------+


***************
States Examples
***************

To provide an illustration of the different states, the output of the
Hello Instance World example will be used.

The HelloinstanceworldPublisher will write three samples in two instances.

+--------------------------------+----------+
| Sample                         | Instance |
+================================+==========+
| userID=1, message="{id1|msg1}" |     1    |
+--------------------------------+----------+
| userID=2, message="{id2|msg1}" |     2    |
+--------------------------------+----------+
| userID=1, message="{id1|msg2}" |     1    |
+--------------------------------+----------+

After that, it will dispose instance 2. Because the dispose is related to
instances only, only the key variable (thus :code:`userID`) is of
importance and the rest of the sample will be ignored (in the helloworld
case that'll be :code:`message`).

This results in the following output from the publisher
::

    === [Publisher] Waiting for a reader ...
    === [Publisher] Writing : Message (1, {id1|msg1})
    === [Publisher] Writing : Message (2, {id2|msg1})
    === [Publisher] Writing : Message (1, {id1|msg2})
    === [Publisher] Dispose : Message (2, don't care)

These samples are received by HelloinstanceworldSubscriber. It is able to
perform a read between every write from the publisher. This is the result
(the last part of every line is related to the :code:`sample state`,
:code:`view state` and :code:`instanc state` of that particular sample):
::

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (1, {id1|msg1}) [ sample not read, instance new,     instance alive ]

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (1, {id1|msg1}) [     sample read, instance old,     instance alive ]
    === [Subscriber] Received : Message (2, {id2|msg1}) [ sample not read, instance new,     instance alive ]

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (1, {id1|msg2}) [ sample not read, instance old,     instance alive ]
    === [Subscriber] Received : Message (2, {id2|msg1}) [     sample read, instance old,     instance alive ]

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (1, {id1|msg2}) [     sample read, instance old,     instance alive ]
    === [Subscriber] Received : Message (2, {id2|msg1}) [     sample read, instance old,  instance disposed ]
    === [Subscriber] Received : Not alive message -> terminate

1st read

    | The sample is not read, the instance has never been seen before and
      the writer did just write it (it didn't dispose nor unregister it).

2nd read

    | Now two samples are acquired by a single read.
    | The first sample is the same sample as previously read. The data is
      still the same, but the states have changed because the sample has
      been read before and the instance has been seen before. The instance
      hasn't been disposed nor unregistered though, so the instance state
      itself didn't change.
    | The second sample is never seen before and is of a new instance. So,
      it has the same states as when the first sample was read for the
      first time.

3rd read

    | Again, two samples are acquired by a single read.
    | This time, the first sample is the new sample that hasn't been read
      before. However, this new sample is related to an instance that is
      already seen (and thus old). Because of the default history of 1
      sample per instance, this new sample has replaced the old sample of
      this instance.
    | The second sample is just the same as it was. It's just old now.

4th read

    | This time, both samples have been read before. It's just that
      instance 2 has been disposed by the writer. The dispose did not
      change the data of that sample. Only the instance state has changed.

HelloinstanceworldSubscriber has been implemented to terminate when it
receives a sample that is not alive (disposed or unregistered).


*******************
Take & Invalid Data
*******************

In the example code of the previous chapters, only :code:`dds_read()` is
used. This will leave the samples in the reader history. On the other
hand, :code:`dds_take()` can also be used to read. In contrast to
:code:`dds_read(), :code:`dds_take()` will remove the sample(s) from the
reader history, which means it can't be read afterwards anymore.

When changing the line
::

    ret = dds_read (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);

in the Hello Instance World example subscriber into
::

    ret = dds_take (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);

you'll see the following output:
::

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (1, {id1|msg1}) [ sample not read, instance new,     instance alive ]

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (2, {id2|msg1}) [ sample not read, instance new,     instance alive ]

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (1, {id1|msg2}) [ sample not read, instance old,     instance alive ]

    === [Subscriber] Waiting for message(s) ...
    === [Subscriber] Received : Message (2, <invalid data>) [ sample not read, instance old,  instance disposed ]
    === [Subscriber] Received : Not alive message -> terminate

1st take

    The sample is not read, the instance has never been seen before and
    the writer did just write it (it didn't dispose nor unregister it).

2nd take

    The second sample is read. Because the first sample was taken from the
    reader history, it is not read with this 2nd take.

3rd take

    Only the new sample of the first instance is read. The instance has
    been seen before, so that's old.

4th take

    The publisher disposed the second instance. The reader doesn't have
    any related sample in its history of which the instance state can be
    changed. To be able to still notify the reader of a disposed
    instance, it will be able to read a :code:`dummy` sample of that
    instance. This means that only the key part of the sample is set. The
    rest of the data is invalid. To indicate that, the :code:`valid_data`
    boolean, related to that sample, has been set to :code:`false`.


***********
Source Code
***********

In the previous chapters, :ref:`Hello Quick World <HelloQuickWorld>` is
taken as basis and some changes that can be applied to it are explained.

Applying these changes results in the following Hello Instance World code.

Publisher:

.. literalinclude:: ../../examples/helloinstanceworld/publisher.c
    :linenos:
    :language: c

Subscriber:

.. literalinclude:: ../../examples/helloinstanceworld/subscriber.c
    :linenos:
    :language: c
