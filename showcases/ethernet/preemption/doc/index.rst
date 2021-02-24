:orphan:

Ethernet Preemption
===================

- Higher priority frames can interrupt transmission of lower priority frames in the ethernet MAC layer.
- This guaranties low latency for high priority frames
- After the high priority frame transmission is finished, the other fragment of the low priority frame is sent

- Actually there are two mac layers (PreemptableMac and ExpressMac)
- This needs packet streaming as well

- Compare three cases:
	- baseline: no preemption or priority queue
	- priority queue in the mac
	- preemption (actually, there are two different queues)

Goals
-----

Ethernet preemption enables higher priority frames to interrupt the transmission of lower priority frames in the Ethernet MAC layer. This guaranties low latency for time-critical high priority frames.

This showcase demonstrates Ethernet preemption and examines the latency reduction.

The Model
---------

In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. When a high priority frame becomes available during the transmission of a low priority frame, the Ethernet MAC can interrupt the transmission of the low priority frame. After sending the high priority frame, the MAC continues the transmission of the low priority frame from where it left off, eventually sending the low priority frame in two (or more) fragments.

**V1** Preemption is a feature of the composable Ethernet model in INET. It requires packet streaming so that packet transmission is represented as a stream so that it can be interrupted. The :ned:`EthernetPreemptingMacLayer` contains two MAC sub-layers, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express mac layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

**V2** Preemption is a feature of the composable Ethernet model in INET. It makes use of the composable Ethernet model's support for packet streaming, so that packet transmission is represented as a stream which can be interrupted. The :ned:`EthernetPreemptingMacLayer` contains two MAC sub-layers, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express mac layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

.. figure:: media/mac.png
   :align: center

Preemption requires the :ned:`EthernetPreemptingPhyLayer`. **TODO** its implemented in/needed for packet streaming

**TODO** we use LayeredEthernetInterface everywhere

.. Preemption requires packet streaming so that packet transmission is represented as a stream so that it can be interrupted.

The simulation uses the following network:

.. figure:: media/network.png
   :align: center

It contains two :ned:`StandardHost`'s connected with 100Mbps Ethernet, and also a :ned:`PcapRecorder` to record Pcap traces.

There are three configurations in omnetpp.ini, for the following three cases:

- **Default**: The baseline configuration (not using preemption or priority queues)/not using any latency reduction techniques
- **PriorityQueue**: Uses priority queue in the Ethernet MAC; should have better delay for high priority frames/for lower delay for high priority frames
- **Preemption**: Uses preemption for high priority frames; for even better delay/should have better delay than the priority queue case, because ongoing low priority frames can be interrupted/'s transmission doenst need to finish before sending the high priority frame

In the ``General`` configuration, the hosts are configured to use the layered/composable ethernet model:

.. literalinclude:: ../omnetpp.ini
   :start-at: encap.typename
   :end-at: LayeredEthernetInterface
   :language: ini

Also, we want to record a PCAP trace, so we can examine the traffic in Wireshark. We enable PCAP recording, and set the PCAP recorder to dump the Ethernet PHY frames; **TODO** CRC/FCS?

.. literalinclude:: ../omnetpp.ini
   :start-at: recordPcap
   :end-at: dumpProtocols
   :language: ini

**V1** One of the hosts, ``host1`` is configured to send UDP packets to ``host2``:

**V2** For traffic generation, ``host1`` is configured to send UDP packets to ``host2``:

**V3** To generate traffic, ``host1`` is configured to send UDP packets to ``host2``:

.. literalinclude:: ../omnetpp.ini
   :start-at: numApps
   :end-at: app[1].io.destPort
   :language: ini

There are two :ned:`UdpApp`'s in ``host1``, one generating background traffic and the other time-sensitive traffic. (The :ned:`UdpApp` is similar to `UdpBasicApp` in function, but it's built using generic protocol components for more flexibility/modularity.) **TODO** what is the outbound module? We configure the app's ``outbound`` module to be a PacketTagger, so we can tag packets with a VLAN ID requests, to put them in different priority categories.
**TODO** why

**TODO** not so detailed

We set up high background traffic (96 Mbps) and lower time-sensitive traffic (9.6 Mbps); both send 1200B packets:

.. literalinclude:: ../omnetpp.ini
   :start-at: app[0].source.packetLength
   :end-at: app[1].source.productionInterval
   :language: ini

In the **Default** configuration, no preemption or priority queue is used; the configuration just limits the EthernetMac's queue to decrease the measured end-to-end delay/to not include queueing time in the measured end-to-end delay:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-at: typename
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-before: Config
   :language: ini

In the **PriorityQueue** configuration, we change the queue type in the Mac layer from the default PacketQueue to PriorityQueue:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-at: classifierClass
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-before: Config
   :language: ini

The priority queue needs two internal queues, for the two traffic categories; to not include queueing delay in the measured end-to-end delay, we also limit the internal queues, disable the buffer, and configure a packet dropper function. We configure the priority queue's classifier to classify packets based on the VLAN ID request. **TODO** why

**V1** In the **Preemption** configuration, we replace the :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` modules default in :ned:`LayeredEthernetInterface` with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`; the latter support Ethernet preemption:

**V2** In the **Preemption** configuration, we replace the default MAC and PHY layer submodules of :ned:`LayeredEthernetInterface` (:ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer`) with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, which support preemption.

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :end-at: dropperClass
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :language: ini

We also limit the queue, and configure a packet dropper function.

Results
-------

In the case of the default configuration, the MAC stores packets in a FIFO queue.
Thus higher priority packets wait in line with the lower priority packets, before getting sent eventually.

In the case of the priority queue configuration, higher priority frames wait in their own queue in the PriorityQueue module in the MAC. If there are high priority frames present in the queue, the MAC will send them next, after finishing the transmission of the current frame. High priority frames can be delayed, as the transmission of the current frame needs to finish before sending the high priority frame.

In the case of the preemption configuration, in addition to the higher priority frames having their own queue, the MAC immediatelly stops transmitting the current low priority frame, and sends the high priority frame.
After the high priority frame transmission is complete, it sends the remaining fragment of the low priority frame.

**TODO** remove duplicates

Here is a video of the preemption behavior:

.. .. video:: media/preemption.mp4
   :width: 100%
	 :align: center

.. .. video:: media/Preemption1.mp4
   :width: 100%
	 :align: center

.. .. video:: media/preemption2.mp4
   :width: 100%
	 :align: center

.. video:: media/preemption3.mp4
   :width: 100%
	 :align: center

.. **TODO** packet log

.. .. figure:: media/packetlog.png
   :align: center

.. .. figure:: media/packetlog2.png
   :align: center

.. .. figure:: media/packetlog3.png
   :align: center

.. .. figure:: media/packetlog4.png
   :align: center

.. figure:: media/packetlog5.png
   :align: center
   :width: 100%

.. The transmission of ``background-3`` starts before the high priority frame arrives at the MAC.

The Ethernet MAC in ``host1`` starts transmitting ``background-3``. While it is transmitting/the transmission is ongoing/during the transmission, a time-sensitive frame (``ts-1``) arrives at the MAC. The MAC interrupts the transmission of  ``background-3``; in the animation, ``background-3`` is first displayed as a whole frame, and changes to ``background-3 frag-0 progress`` when the high priority frame is available. Then there is the high priority frame transmission and then the remaining fragment of background-3 (frag1). **TODO**

In the animation, background 3 is displayed as a whole frame at first, because the MAC plans to transmit it in one go. Then the ts-0 frame becomes available, and the transmission is interrupted; the frame in the animation changes to background3 frag0. After an interframe gap period, the ts-0 frame is sent. Then the remaining fragment.of background 3.

In the packet log, background-3 is present as a whole frame (1254B), as the MAC was about to send it uninterrupted. When ts-0 becomes available, background-3 frag0 (x B) is logged.

so

- the log gets entries as they happen
- at first background3 uninterrupted is in the log
- then background3 frag0
- when in fact in reality only background3 frag0 was sent/the bytes of background3 frag0 were sent before ts0

so

- when ts-1 is available, the transmission animation of background-3 changes to frag-0
- it is interrupted
- ts-0 is sent
- frag0 is sent
- in the packet log, first there is the whole background-3 (cos the MAC through at first that it'll send it uninterrupted)
- then there is the frag0 (which is about halfway into the transmission of background 3
- then there is ts-1
- then there frag1

.. .. |1| image:: media/1.png

.. .. |2| image:: media/2.png

.. .. |3| image:: media/3.png

.. .. |4| image:: media/4.png

.. +-----+-----+
   + |1| + |2| +
   +-----+-----+
   + |3| + |4| +
   +-----+-----+

.. figure:: media/seqchart4.png
   :align: center
   :width: 100%

**V1**

.. figure:: media/linear.png
   :align: center
   :width: 100%

**V2**

.. figure:: media/linear2.png
   :align: center
   :width: 100%

.. figure:: media/wireshark.png
   :align: center
   :width: 100%

- wireshark displays fragmented ethernet frames weirdly
- the two fragments are 5 and 7
- the reassembled frame is 6
- the sum of the sizes of 5 + 7 (583 + 683) is 1266
- its more than the unfragmented frame size (1254) because there are two ethernet headers/more headers

.. .. figure:: media/inspector.png
   :align: center

.. .. figure:: media/packetinspector.png
   :align: center

.. figure:: media/packetinspector2.png
   :align: center
   :width: 100%

At the start of the simulation/at simulation time 0, both the high and the low priority app sends an UDP packet to lower layers. The Ethernet MAC starts transmitting the lower priority frame **TODO** why

.. figure:: media/delay.png
   :align: center
   :width: 100%

.. .. figure:: media/preemptive.png
   :align: center
   :width: 90%

.. .. figure:: media/express.png
   :align: center
   :width: 90%

.. figure:: media/preemptible2.png
   :align: center

.. figure:: media/express2.png
   :align: center
