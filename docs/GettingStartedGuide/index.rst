.. _`GettingStarted`:

#####################
Getting Started Guide
#####################

This is the getting started guide.

It'll first start with a short :ref:`introduction <Introduction>` into
DDS. Then it'll mention how VortexDDS can be
:ref:`installed <Installation>`.

The following chapters are examples of increasing complexity.

The :ref:`Hello World <HelloWorld>` example is the simplest and mainly
concerns writing and reading data and the data type. However, it also
explains how examples and other applications can be build and run.

The :ref:`Hello Quick World <HelloQuickWorld>` handles events (like
detecting new data) and the difference between reliable and best effort
communication.

The :ref:`Hello Instance World <HelloInstanceWorld>` talks about keys to
identify data, which means samples and instances in DDS terms. It also
talks about how states of data can change and how this is related to
these samples and instances.

.. toctree::
    :maxdepth: 3

    introduction.rst
    installation.rst
    helloworld.rst
    helloquickworld.rst
    helloinstanceworld.rst


