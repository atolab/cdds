.. _`GettingStarted`:

#####################
Getting Started Guide
#####################

This is the getting started guide.

It starts with a short :ref:`introduction <Introduction>` into DDS and
then continues with the :ref:`installation <Installation>` of VortexDDS.

The following chapters are examples of increasing complexity.

The :ref:`Hello World <HelloWorld>` example is the simplest one and mainly
concerns with writing and reading data and the data type. However, it also
explains how examples and other applications can be build and run.

The :ref:`Hello Quick World <HelloQuickWorld>` handles events (like
detecting new data) and the difference between reliable and best effort
communication.

The :ref:`Hello Instance World <HelloInstanceWorld>` deals with keys to
identify data, which means samples and instances in DDS terms. It also
deals with the way states of data can change and how this is related to
these samples and instances.

.. toctree::
    :maxdepth: 3

    introduction.rst
    installation.rst
    helloworld.rst
    helloquickworld.rst
    helloinstanceworld.rst


