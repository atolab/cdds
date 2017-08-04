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

It is expected that the :ref:`Hello World <HelloWorld>` example is known.

The :ref:`Hello World <HelloWorld>` example is simple indeed, but it has
some idiosyncrasies:

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


*******
Waitset
*******

The waitset is an object to which entities (like readers and writers) can
be attached. It supplies a wait function that'll block until the status
of one (or multiple) of these entities change. There are two status
changes that interests us:

- The writer discovered a reader.
- The reader received data.

The writer event related to the discovery is :code:`publication matched`.
This means that doing the following will block the HelloquickworldPublisher
until a reader that matches the publication is detected within 30 seconds.
The number of triggered entities are returned by the wait, so if 0 is
returned, you know it timed out.
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

By adding this to the publisher, the Hello Quick World publisher will wait
until it detects the subscriber. This means that you don't have to start
the subscriber before the publisher, like you had to with the
:ref:`Hello World <HelloWorld>` example.

Adding the same kind of code block to the subscriber let's us get rid of
the polling loop. Just wait for the :code:`data available` status change
on the reader.
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
sleeps that were in the :ref:`Hello World <HelloWorld>` example.


***********
Reliability
***********

The Hello Quick World publisher waits for it to detect a reader by means
of the :code:`publication matched` status change. However, the writers'
discovery can be quicker than the readers'. This can mean that the writer
already discovered the reader and starts writing while the reader hasn't
discovered the writer yet. The result is that the subscriber receives
samples of an unknown origin and ignores them so they don't reach the
reader.

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

The :code:`best effort` QoS is a bit more efficient compared to
:code:`reliable`, especially when considering the writer:

- It doesn't have to wait/respond to ack/nacks.
- It doesn't have to maintain samples in memory for possible resends.

For some forms of communication, :code:`best effort` is good enough (f.i.
with periodic updates). For others, you'd really like to have
:code:`reliable`.
So, the choice between them is a comparison between performance and how
reliable the communication should be.



***********
Source Code
***********

When taking :ref:`Hello World <HelloWorld>` as basis and add the mentioned
improvements to that, you get the Hello Quick World code.

Publisher:

.. literalinclude:: ../../examples/helloquickworld/publisher.c
    :linenos:
    :language: c

Subscriber:

.. literalinclude:: ../../examples/helloquickworld/subscriber.c
    :linenos:
    :language: c

