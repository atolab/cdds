HelloWorld
==========

Description
***********

The basic HelloWorld example is used to illustrate the necessary steps to setup DCPS entities.
Note it is also used in the Getting Started Guide to explain the usage of VortexDDS.

Design
******

It consists of 2 units:

- HelloworldPublisher: implements the publisher's main
- HelloworldSubscriber: implements the subscriber's main

Scenario
********

The publisher sends a single HelloWorld sample. The sample contains two fields:

- a userID field (long type)
- a message field (string type)

When it receives the sample, the subscriber displays the userID and the message field.

Running the example
*******************

It is recommended that you run the subscriber and publisher in separate terminals to avoid mixing the output.

- Open 2 terminals.
- In the first terminal start the subscriber by running HelloWorldSubscriber
- In the second terminal start the publisher by running HelloWorldPublisher
