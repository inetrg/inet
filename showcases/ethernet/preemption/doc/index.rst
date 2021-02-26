:orphan:

Ethernet Preemption
===================

.. - Higher priority frames can interrupt transmission of lower priority frames in the ethernet MAC layer.
   - This guaranties low latency for high priority frames
   - After the high priority frame transmission is finished, the other fragment of the low priority frame is sent

   - Actually there are two mac layers (PreemptableMac and ExpressMac)
   - This needs packet streaming as well

   **TODO** two mac sublayers ?

.. - Compare three cases:
	- baseline: no preemption or priority queue
	- priority queue in the mac
	- preemption (actually, there are two different queues)

Goals
-----

Ethernet preemption enables higher priority frames to interrupt the transmission of lower priority frames in the Ethernet MAC layer. This guaranties low latency for time-critical high priority frames.

This showcase demonstrates Ethernet preemption and examines the latency reduction.

The Model
---------

.. **V1** In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. When a high priority frame becomes available during the transmission of a low priority frame, the Ethernet MAC can interrupt the transmission of the low priority frame. After sending the high priority frame, the MAC continues the transmission of the low priority frame from where it left off, eventually sending the low priority frame in two (or more) fragments.

In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. When a high priority frame becomes available during the transmission of a low priority frame, the Ethernet MAC can interrupt the transmission of the low priority frame, and start sending the high priority frame instead. The MAC can continue transmission of the low priority frame from where it left off, eventually sending the low priority frame in two (or more) fragments.


.. **V1** Preemption is a feature of the composable Ethernet model in INET. It requires packet streaming so that packet transmission is represented as a stream so that it can be interrupted. The :ned:`EthernetPreemptingMacLayer` contains two MAC sub-layers, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express mac layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

.. ------------------------------

   **V1** Preemption is a feature of the composable Ethernet model in INET. It makes use of the composable Ethernet model's support for packet streaming, so that packet transmission is represented as a stream which can be interrupted. The :ned:`EthernetPreemptingMacLayer` contains two MAC sub-layers, a preemptable (:ned:`EthernetFragmentingMacLayer`) and an express mac layer (:ned:`EthernetStreamingMacLayer`), each with its own queue for frames:

   .. **TODO** a nodeon belul streamel; macLayeren belul is streaming; phy layer is;


   .. figure:: media/mac.png
      :align: center

   .. **V2** Preemption is a feature of the composable Ethernet model in INET. It makes use of the composable Ethernet model's support for packet streaming, so that packet transmission is represented as a stream which can be interrupted.
      As such, it reqires the composable Ethernet interface (:ned:`LayeredEthernetInterface`) with the following submodules: 

   .. It requires the following modules in the composable Ethernet interface (:ned:`LayeredEthernetInterface`)

      - :ned:`EthernetPreemptingMacLayer`: The module contains two MAC sub-layers, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express mac layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

      .. figure:: media/mac.png
         :align: center

      - :ned:`EthernetPreemptingPhyLayer`: This module supports packet streaming.

   Preemption requires the :ned:`EthernetPreemptingPhyLayer`. **TODO** its implemented in/needed for packet streaming

.. ------------------------------

   **V3** Preemption is a feature of the composable Ethernet model in INET. It makes use of the composable Ethernet model's support for packet streaming, so that packet transmission is represented as a stream which can be interrupted.
   As such, it reqires the composable Ethernet interface (:ned:`LayeredEthernetInterface`) with the following submodules: an :ned:`EthernetPreemptingMacLayer` and an :ned:`EthernetPreemptingPhyLayer`.

   **TODO** 

   - It uses intra-node packet streaming; 
   - preemptable maclayerbol jon a stream;
   - preempting server mondja hogy stop es inditja el a masik macbol; 
   - ujrainditja a frame-et
   - any given moment of time csak 1 frame lehet fragmentalva.
   - a sceduleren keresztul értesul hogy van egy masik stream
   - az IFG applied in the phy layer

.. **V4** Preemption is a feature of INET's composable Ethernet model. It uses packet streaming, so that packet transmission is represented as a stream which can be interrupted.

Preemption is a feature of INET's composable Ethernet model. It uses packet streaming, so that packet transmission is represented as a stream which can be interrupted. Preemption requires the :ned:`LayeredEthernetInterface` (containing MAC and PHY layers, displayed below).

.. figure:: media/LayeredEthernetInterface2.png
   :align: center

To enable preemption, the default submodules :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` need to be replaced with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`.

.. It requires the following modules in the composable Ethernet interface (:ned:`LayeredEthernetInterface`)

The :ned:`EthernetPreemptingMacLayer` contains two sub-MAC-layer submodules, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express MAC layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

.. figure:: media/mac.png
   :align: center

:ned:`EthernetPreemptingMacLayer` uses intra-node packet streaming. Discrete packets come in the MAC module from the higher layers, but the sub-MAC-layers (express and preemptable) output packet streams. Packets continue as streams out of the MAC layer, through the PHY layer and out the Ethernet connection.

**V1** During preemption, packets initially stream from the preemptable sub-mac-layer. The scheduler notifies the preempting server when a packet arrives at the express mac. The server stops the preemptable stream, sends the express stream, and then it restarts the preemptable steam.

**V2** Here is an example for the preemption process:

- packets initially stream from the preemptable sub-mac-layer
- The scheduler notifies the preempting server when a packet arrives at the express mac
- The server stops the preemptable stream, sends the express stream, and then it restarts the preemptable steam

Interframe gaps between the packets are applied/inserted at the PHY layer. Note that only one frame can be fragmented at any given moment.

The :ned:`EthernetPreemptingPhyLayer` supports packet streaming and fragmenting (sending packets in multiple fragments).

.. **TODO** supports streaming and fragmenting (which is tobb darabban kuld el csomagokat)

.. ------------------------------

.. **TODO** `not here?` Preemption requires the :ned:`EthernetPreemptingPhyLayer`. **TODO** its implemented in/needed for packet streaming

.. **TODO** we use LayeredEthernetInterface everywhere

.. Preemption requires packet streaming so that packet transmission is represented as a stream so that it can be interrupted.

The simulation uses the following network:

.. figure:: media/network.png
   :align: center

It contains two :ned:`StandardHost`'s connected with 100Mbps Ethernet, and also a :ned:`PcapRecorder` to record PCAP traces.

There are three configurations in omnetpp.ini, for the following three cases:

- **Default**: The baseline configuration (not using preemption or priority queues)/not using any latency reduction techniques
- **PriorityQueue**: Uses priority queue in the Ethernet MAC for lower delay of high priority frames
- **Preemption**: Uses preemption for high priority frames for even lower delay than the priority queue case, because the ongoing transmission of low priority frames doens't need to finish before sending the high priority frame

In the ``General`` configuration, the hosts are configured to use the layered ethernet model instead of the default, which must be disabled:

.. literalinclude:: ../omnetpp.ini
   :start-at: encap.typename
   :end-at: LayeredEthernetInterface
   :language: ini

Also, we want to record a PCAP trace, so we can examine the traffic in Wireshark. We enable PCAP recording, and set the PCAP recorder to dump Ethernet PHY frames:

.. literalinclude:: ../omnetpp.ini
   :start-at: recordPcap
   :end-at: fcsMode
   :language: ini

.. **V1** One of the hosts, ``host1`` is configured to send UDP packets to ``host2``:

.. **V2** For traffic generation, ``host1`` is configured to send UDP packets to ``host2``:

To generate traffic, ``host1`` is configured to send UDP packets to ``host2``:

.. literalinclude:: ../omnetpp.ini
   :start-at: numApps
   :end-at: app[1].io.destPort
   :language: ini


.. ------------------------------

   **V1** There are two :ned:`UdpApp`'s in ``host1``, one is generating background traffic and the other time-sensitive traffic. (The :ned:`UdpApp` is similar to `UdpBasicApp` in function, but it's built using generic protocol components for more flexibility/modularity.) **TODO** what is the outbound module? We configure the app's ``outbound`` module to be a PacketTagger, so we can tag packets with a VLAN ID requests, to put them in different priority categories.
   **TODO** why

   ------------------------------

There are two :ned:`UdpApp`'s in ``host1``, one is generating background traffic and the other time-sensitive traffic. The UDP apps put VLAN tags on the packets, and the Ethernet MAC uses these tags to classify traffic into high and low priorities.


.. .. note:: The :ned:`UdpApp` is similar to `UdpBasicApp` in function, but it's modular, built using generic protocol components for more flexibility.) **TODO** not sure its needed at all

------------------------------

.. **TODO** not so detailed

We set up high background traffic (96 Mbps) and lower time-sensitive traffic (9.6 Mbps); both send 1200B packets:

.. literalinclude:: ../omnetpp.ini
   :start-at: app[0].source.packetLength
   :end-at: app[1].source.productionInterval
   :language: ini

In the **Default** configuration, no preemption or priority queue is used; the configuration just limits the :ned:`EthernetMac`'s queue length to not include queueing time in the measured end-to-end delay **TODO** false:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-at: typename
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-before: Config
   :language: ini

In the **PriorityQueue** configuration, we change the queue type in the Mac layer from the default :ned:`PacketQueue` to :ned:`PriorityQueue`:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-at: classifierClass
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-before: Config
   :language: ini

The priority queue needs two internal queues, for the two traffic categories; to not include queueing delay in the measured end-to-end delay, we also limit the internal queues, disable the shared buffer, and configure a packet dropper TODO function. We configure the priority queue's classifier to classify packets based on the VLAN ID request, which indicates the traffic category.

.. **TODO** we'll use VLAN tags to indicate the traffic categories. The UDP apps put VLAN tags to the packets they create, 

In the **Preemption** configuration, we replace the :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` modules default in :ned:`LayeredEthernetInterface` with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, which support preemption:

TODO there is no priority queue; there are queues in the two submacs; lehet a merged macben is shared queue; 

.. **V2** In the **Preemption** configuration, we replace the default MAC and PHY layer submodules of :ned:`LayeredEthernetInterface` (:ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer`) with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, which support preemption.

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :end-at: dropperClass
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :language: ini

.. note:: There is no priority queue in this configuration, but the sub-mac-layer modules each have their own queue. It is also possible to disable these internal queues and have just one shared queue in the EthernetPreemptableMac module.

We also limit the queue, and configure a packet dropper function. TODO

Results
-------

In the case of the `default` configuration, the MAC stores packets in a FIFO queue.
Thus higher priority packets wait in line with the lower priority packets, before getting sent eventually.

In the case of the `priority queue` configuration, higher priority frames wait in their own sub-queue in the PriorityQueue module in the MAC. If there are high priority frames present in the queue, the MAC will send them first, or after finishing the transmission of the current low priority frame. High priority frames can be delayed, as the transmission of the current frame needs to finish before sending the high priority frame.

In the `preemption` configuration, in addition to the higher priority frames having their own queue, the MAC immediatelly stops transmitting the current low priority frame, and sends the high priority frame instead.
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

.. The transmission of ``background-3`` starts before the high priority frame arrives at the MAC.

The Ethernet MAC in ``host1`` starts transmitting ``background-3``. During the transmission, a time-sensitive frame (``ts-1``) arrives at the MAC. The MAC interrupts the transmission of  ``background-3``; in the animation, ``background-3`` is first displayed as a whole frame, and changes to ``background-3 frag-0 progress`` when the high priority frame is available. Then there is the high priority frame transmission and then the remaining fragment of background-3 (frag1)./After transmitting the high priority frame, the remaining fragment of background-3 (frag1) is transmitted.**TODO**

.. **V2** In the animation, background 3 is displayed as a whole frame (as opposed to a fragment) at first, because the MAC plans to transmit it in one go. Then the ts-0 frame becomes available, and the transmission is interrupted; the frame in the animation changes to background3 frag0. After an interframe gap period, the ts-0 frame is sent. Then the remaining fragment.of background 3.

The same frame sequence as in the video is displayed in the Qtenv packet log:

.. figure:: media/packetlog5.png
   :align: center
   :width: 100%

.. **V1** In the packet log, background-3 is present as a whole frame (1254B), as the MAC was about to send it uninterrupted. When ts-0 becomes available, background-3 frag0 (x B) is logged.

Entries are added to the packet log as they happen in the simulation. At first, ``background-3`` is logged as an uninterrupted frame. When the high priority frame becomes available, the frame name changes to `background-3 frag-0`, and it's logged. Actually, only ``background-3 frag-0`` was sent before ``ts-1``.

a logbol ugy tunik hogy egy 1200Bos csomag atment de nem mert rajott h nem lesz
annyi

.. **V3** Entries are added to the packet log as they happen in the simulation. At first, background-3 is present as a whole frame (1254B), as the MAC was about to send it uninterrupted. When ts-0 becomes available, background-3 frag0 (583B) is logged.

.. so

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

The same frame sequence is displayed on a sequence chart on the following images, with a different frame selected and highlighted on each image. Note that the timeline is non-linear:

.. figure:: media/seqchart4.png
   :align: center
   :width: 100%

Just as in the packet log, the sequence chart contains the whole uninterrupted `background-3` frame, as it's logged when its transmission is started. This can be confusing, but it's actually the proper operation of the sequence chart tool. 

.. note:: There are actually two time dimensions present on the sequence chart: the events and messages as they happen at a moment, and what the modules "think" about the future, i.e. how long will a transmission take. In reality, the transmission might be interrupted and so both the original (background3) and the "updated" (background3 frag0) is present on the chart.

------------------------------

**V1** Here is the frame sequence on a sequence chart on a linear timeline, with the background-3 frag-0 frame highlighted:

.. figure:: media/linear.png
   :align: center
   :width: 100%

   ------------------------------

**V2!** Here is the frame sequence on a sequence chart on a linear timeline:

.. figure:: media/linear2.png
   :align: center
   :width: 100%

------------------------------

**TODO** the background3 frag0 frame is just a minimum ethernet frame long

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

.. .. figure:: media/packetinspector2.png
   :align: center
   :width: 100%

.. .. figure:: media/packetinspector4.png
   :align: center
   :width: 100%

Here is background3-frag1 displayed in Qtenv's packet inspector:

.. figure:: media/packetinspector5.png
   :align: center
   :width: 100%

**TODO** -> you can see the phy header details -> elso header-t is!
ezert nincs benne byteocuntchunk mert nincs benne ethernet mac header 

.. **TODO** without highlight

.. At the start of the simulation/at simulation time 0, both the high and the low priority app sends an UDP packet to lower layers. The Ethernet MAC starts transmitting the lower priority frame **TODO** why

The paths the high and low priority (express and preemptable) packets take in the :ned:`EthernetPreemptingMayLayer` is illustrated below:

.. figure:: media/preemptible2.png
   :align: center

.. figure:: media/express2.png
   :align: center

We plot the mean end-to-end delay of the UDP packets for the three cases on the following chart:

.. figure:: media/delay.png
   :align: center
   :width: 100%

As expected, in the default case, the delay for the two priority categories are the same. The priority queue decreases the delay of the high priority frames and increases that of the low priority frames. Using preemption has a similar effect but its more effective/Using preemption produces a similar effect but more effective/Using preemption is even more effective in reducing delay of high priority frames.

.. .. figure:: media/preemptive.png
   :align: center
   :width: 90%

.. .. figure:: media/express.png
   :align: center
   :width: 90%
