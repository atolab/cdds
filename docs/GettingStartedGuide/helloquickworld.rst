.. _`HelloQuickWorld`:

.. raw:: latex

    \newpage

########################################
Hello Quick World (events & reliability)
########################################

.. contents::

************
Introduction
************

In this chapter, it is assumed that the :ref:`Hello World <HelloWorld>`
example is known by the reader.

Although the  :ref:`Hello World <HelloWorld>` example is simple, it has
some idiosyncrasies which are:

- The HelloworldSubsriber needs to be started before HelloworldPublisher.
- The HelloworldSubsriber polls for data.
- The HelloworldPublisher sleeps to let the reader/writer discovery
  finish.

The Hello Quick World example will solve these issues.

:ref:`Building <HelloWorldBuilding>` this example is done through
:ref:`CMake <CMakeIntro>` and executing it is a bit like
:ref:`running the Hello World <HelloWorldRunning>` example, except
the binaries can not be found in the pre-compiled bin directory,
but in the CMake build directory.

.. _`WaitsetIntro`:

*******
Waitset
*******

The waitset is an object which entities (like readers and writers) can be
attached to. It supplies a wait function that'll block until the status
of one (or multiple) of these entities change. There are two status
changes that are of interest for the Hello World example:

- The writer discovered a reader.
- The reader received data.

The writer event related to the discovery is :code:`publication matched`.
This means that doing the following will block the HelloquickworldPublisher
until a reader that matches the publication is detected within 30 seconds.
The number of triggered entities are returned by the wait. So, for
example, if 0 is returned, it means that the wait timed out.
::

    dds_return_t ret;
    dds_entity_t waitset;
    dds_attach_t triggered;
    ret = dds_set_enabled_status(writer, DDS_PUBLICATION_MATCHED_STATUS);
    waitset = dds_create_waitset(participant);
    ret = dds_waitset_attach(waitset, writer, NULL);
    ret = dds_waitset_wait(waitset, &triggered, 1, DDS_SECS(30));
    if (ret > 0) {
        // write
    }

By adding this code to the publisher, the Hello Quick World publisher will
wait until it detects the subscriber. This means that you don't have to
start the subscriber before the publisher, which you had to with the
:ref:`Hello World <HelloWorld>` example.

Adding the same kind of code block to the subscriber allows the user to
get rid of the polling loop. Just wait for the :code:`data available`
status change on the reader.
::

    dds_return_t ret;
    dds_entity_t waitset;
    dds_attach_t triggered;
    ret = dds_set_enabled_status(reader, DDS_DATA_AVAILABLE_STATUS);
    waitset = dds_create_waitset(participant);
    ret = dds_waitset_attach(waitset, reader, NULL);
    ret = dds_waitset_wait(waitset, &triggered, 1, DDS_SECS(30));
    if (ret > 0) {
        // read
    }

By using a waitset in the publisher and subscriber, we got rid of the
sleeps which are needed in the :ref:`Hello World <HelloWorld>` example.

.. _`ReliabilityIntro`:

***********
Reliability
***********

As we can see in the previous chapter, the Hello Quick World publisher
uses the `WaitSet`_ to wait for a reader by means of detecting the
:code:`publication matched` status change. However, the writers'
detection of the reader can be quicker than the readers' detection of the
writer. This can mean that the writer already discovered the reader and
starts writing while the reader hasn't discovered the writer yet. The
result is that the subscriber receives samples of an unknown origin and
ignores them so they don't reach the reader.

This doesn't seem like a reliable communication. Which, in fact, it isn't.
The default setting of a reader and writer is :code:`best effort`. This
means that sample delivery is not guaranteed.

We can change the reliability of the communication by changing the Quality
of Service (QoS).
::

    dds_qos_t *qos;
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));

This means that every entity created with this QoS will perform reliable
communication. The duration argument tells us how long a
:code:`dds_write()` will block when a sample can not be delivered to
discovered readers for whatever reason.

Now we have a reliable QoS, but the default of the reader and writer are
still :code:`best effort`. So, we have to use that QoS when creating them.
::

    reader = dds_create_reader (participant, topic, qos, NULL);
    writer = dds_create_writer (participant, topic, qos, NULL);

The :code:`best effort` QoS is more efficient when compared to
:code:`reliable`, especially when considering the writer:

- It doesn't have to wait/respond to ack/nacks.
- It doesn't have to maintain samples in memory for possible resends.

A :code:`best effort` reader is also more efficient then a
:code:`reliable` reader.

For some forms of communication, :code:`best effort` is good enough (f.i.
with periodic updates). For others, you'd really like to have
:code:`reliable`.
So, the choice between them is a comparison between performance and how
reliable the communication should be.


***********
Source Code
***********

In the previous chapters, :ref:`Hello World <HelloWorld>` is taken as
basis and explained how it can be improved. These improvements are shown
in this chapter with the  Hello Quick World code.

Publisher:

.. literalinclude:: ../../examples/helloquickworld/publisher.c
    :linenos:
    :language: c

Subscriber:

.. literalinclude:: ../../examples/helloquickworld/subscriber.c
    :linenos:
    :language: c

