:orphan:

Ethernet Preemption
===================

Goals
-----

Ethernet preemption (802.1Qbu) enables higher priority frames to interrupt the transmission of lower priority frames in the Ethernet MAC layer. This facilitates creating low latency for time-critical high priority frames.

This showcase demonstrates Ethernet preemption and examines the latency reduction.

The Model
---------

Overview
~~~~~~~~

In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. When a high priority frame becomes available for transmission during the transmission of a low priority frame, the Ethernet MAC can interrupt the transmission of the low priority frame, and start sending the high priority frame immediately. When the high priority frame finishes, the MAC can continue transmission of the low priority frame from where it left off, eventually sending the low priority frame in two (or more) fragments. 

Preemption is a feature of INET's composable Ethernet model. It uses INET's packet streaming API, so that packet transmission is represented as an interruptable stream. Preemption requires the :ned:`LayeredEthernetInterface`, which contains a MAC and a PHY layer, displayed below:

.. figure:: media/LayeredEthernetInterface2.png
   :align: center

To enable preemption, the default submodules :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` need to be replaced with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`.

The :ned:`EthernetPreemptingMacLayer` contains two submodules which themselves represent Ethernet MAC layers, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express MAC layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

.. figure:: media/mac.png
   :align: center

The :ned:`EthernetPreemptingMacLayer` uses intra-node packet streaming. Discrete packets enter the MAC module from the higher layers, but leave the sub-MAC-layers (express and preemptable) as packet streams. Packets exit the MAC layer as a stream, and are represented as such through the PHY layer and the link.

In the case of preemption, packets initially stream from the preemptable sub-MAC-layer. The ``scheduler`` notifies the ``preemptingServer`` when a packet arrives at the express MAC. The ``preemptingServer`` stops the preemptable stream, sends the express stream in full, and then eventually it resumes the preemptable stream.

Interframe gaps are inserted by the PHY layer. 

**TODO** `somewhere else?` Note that only one frame can be fragmented by preemption at any given moment.

The :ned:`EthernetPreemptingPhyLayer` supports both packet streaming and fragmenting (sending packets in multiple fragments).

Configuration
~~~~~~~~~~~~~

The simulation uses the following network:

.. figure:: media/network.png
   :align: center

It contains two :ned:`StandardHost`'s connected with 100Mbps Ethernet, and also a :ned:`PcapRecorder` to record PCAP traces; ``host1`` periodically generates packets for ``host2``.

Primarily, we want to compare the end-to-end delay, so we run simulations with the same packet length for the low and high priority traffic in the following three configurations in omnetpp.ini: 

- ``Default``: The baseline configuration; doesn't use any latency reduction techniques
- ``PriorityQueue``: Uses priority queue in the Ethernet MAC to lower the delay of high priority frames
- ``Preemption``: Uses preemption for high priority frames for a very low delay with a guaranteed upper bound

Additionally, we demonstrate the above delay reduction techniques with more realistic traffic: longer and more frequent low priority frames and shorter, less frequent high priority frames. These simulations are the extension of the three configurations mentioned above, and are defined in the ini file as the configurations with the ``Realistic`` prefix.

In the ``General`` configuration, the hosts are configured to use the layered Ethernet model instead of the default, which must be disabled:

.. literalinclude:: ../omnetpp.ini
   :start-at: encap.typename
   :end-at: LayeredEthernetInterface
   :language: ini

We also want to record a PCAP trace, so we can examine the traffic in Wireshark. We enable PCAP recording, and set the PCAP recorder to dump Ethernet PHY frames, because preemption is visible in the PHY header:

.. literalinclude:: ../omnetpp.ini
   :start-at: recordPcap
   :end-at: fcsMode
   :language: ini

Here is the configuration of traffic generation in ``host1``:

.. literalinclude:: ../omnetpp.ini
   :start-at: numApps
   :end-at: app[1].io.destPort
   :language: ini

There are two :ned:`UdpApp`'s in ``host1``, one is generating background traffic (low priority) and the other, high-priority traffic. The UDP apps put VLAN tags on the packets, and the Ethernet MAC uses the VLAN ID contained in the tags to classify the traffic into high and low priorities.

We set up a high-bitrate background traffic (96 Mbps) and a lower-bitrate high-priority traffic (9.6 Mbps); both with 1200B packets. Their sum is intentionally higher than the 100 Mbps link capacity (we want non-empty queues); excess packets will be dropped.

.. literalinclude:: ../omnetpp.ini
   :start-at: app[0].source.packetLength
   :end-at: app[1].source.productionInterval
   :language: ini

The ``Default`` configuration uses no preemption or priority queue, the configuration just limits the :ned:`EthernetMac`'s queue length to 4. 

In all three cases, the queues need to be short to decrease the queueing time's effect on the measured delay. However, if they are too short, they might be empty too often, which renders the priority queue useless (it cannot prioritize if it contains just one packet, for example). The queue length of 4 is an arbitrary choice. The queue type is set to :ned:`DropTailQueue` so that it can drop packets if the queue is full:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-before: Config
   :language: ini

In the ``PriorityQueue`` configuration, we change the queue type in the Mac layer from the default :ned:`PacketQueue` to :ned:`PriorityQueue`:

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-before: Config
   :language: ini

The priority queue utilizes two internal queues, for the two traffic categories. To limit the queueing time's effect on the measured end-to-end delay, we also limit the length of internal queues to 4. We also disable the shared buffer, and set the queue type to :ned:`DropTailQueue`. We use the priority queue's classifier to put packets into the two traffic categories.

In the ``Preemption`` configuration, we replace the :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` modules default in :ned:`LayeredEthernetInterface` with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, which support preemption.

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :end-at: DropTailQueue
   :language: ini

There is no priority queue in this configuration, the two MAC submodules both have their own queues.
We also limit the queue length to 4, and configure the queue type to be :ned:`DropTailQueue`.

.. note:: We could also have just one shared priority queue in the EthernetPreemptableMac module, but this is not covered here.

We use the following traffic for the ``RealisticDefault``, ``RealisticPriorityQueue`` and ``RealisticPreemption`` configurations:

.. literalinclude:: ../omnetpp.ini
   :start-after: Config RealisticBase
   :end-before: Config RealisticDefault
   :language: ini

In this traffic configuration, high-priority packets are 100 times less frequent, and are 1/10th the size of low-priority packets.

Transmission on the Wire
~~~~~~~~~~~~~~~~~~~~~~~~

In order to make sense of how frame preemptions are represented in the OMNeT++ GUI (in Qtenv's animation and packet log, and in the Sequence Chart in the IDE), it is necessary to understand how packet transmissions are modeled in OMNeT++.

Traditionally, transmitting a frame on a link is represented in OMNeT++ by sending a "packet".  The "packet" is a C++ object (i.e. data structure) which is of, or is subclassed from, the OMNeT++ class ``cPacket``. The sending time corresponds to the start of the transmission. The packet data structure contains the length of the frame in bytes, and also the (more or less abstracted) frame content. The end of the transmission is implicit: it is computed as *start time* + *duration*, where *duration* is either explicit, or derived from the frame size and the link bitrate. This approach in vanilla form is of course not suitable for Ethernet frame preemption, because it is not known in advance whether or not a frame transmission will be preempted, and at which point.

Instead, in OMNeT++ 6.0 the above approach was modified to accommodate new use cases. In the new approach, the original packet sending remains, but its interpretation changes slightly. It now represents a *prediction*: "this is a frame whose transmission will go through, unless we say otherwise". Namely, while the transmission is ongoing, it is possible to send *transmission updates*, which modifies the prediction about the remaining part of the transmission. A *transmission update* packet essentially says "ignore what I said previously about the total frame size/content and transmission time, here's how much time the remaining transmission is going to take according to the current state of affairs, and here's the updated frame length/content". A transmission update may truncate, shorten or extend a transmission (and the frame). For technical reasons, the transmission update packet carries the full frame size and content (not just the remaining part), but it must be crafted by the sender in a way that it is consistent with what has already been transmitted (it cannot alter the past). For example, truncation is done by indicating zero remaining time, and setting the frame content to what has been transmitted up to that point. An updated transmission may be further modified by subsequent transmission updates. The end of the transmission is still implicit (it finishes according to the last transmission update), but it is also possible to make the ending explicit by sending a zero-remaining-time transmission update at exactly the time the transmission would otherwise end. After the transmission's end time has passed, it is naturally not possible to send any more transmission updates for it (we cannot modify the past).

In the light of the above, it is easy to see why a preempted Ethernet frame appears in e.g. Qtenv's event log multiple times: the original transmission and the subsequent transmission update(s) are all packets.

- The first one is the original packet, which contains the full frame size/content and carries the prediction that the frame transmission will go through uninterrupted.
- The second one is sent at the time the decision is made inside the node that the frame is going to be preempted. At that time, the node computes the truncated frame and the remaining transmission time, taking into account that at least the current octet and FCS need to be transmitted, and there is a minimum frame size requirement as well. The packet represents the size/content of the truncated frame, including FCS.
- In the current implementation, the Ethernet model also sends an explicit end-transmission update, with zero remaining transmission duration and identical frame size/content as the previous one. This would not be strictly necessary, and may change in future INET releases.

The above packets are distinguished using name suffixes: ``:progress`` and ``:end`` are appended to the original packet name for transmission updates and for the explicit end-transmission, respectively. In addition, the packet itself is also renamed by adding ``-frag0``, ``-frag1``, etc. to its name, to make frame fragments distunguishable from each other. For example, a frame called ``background3`` may be followed by ``background3-frag0:progress`` and ``background3-frag0:end``. After the intervening express frame has also completed transmission, ``background3-frag1`` will follow (see video in the next section).



Results
-------

.. In the animation, the packet log, and the event log file/sequence chart tool, frames are displayed or recorded as soon as they processed in the simulation. 

Preemption Behavior
~~~~~~~~~~~~~~~~~~~

Here is a video of the preemption behavior:

.. video:: media/preemption3.mp4
   :width: 100%
	 :align: center

The Ethernet MAC in ``host1`` starts transmitting ``background-3``. During the transmission, a high-priority frame (``ts-1``) arrives at the MAC. The MAC interrupts the transmission of  ``background-3``; in the animation, ``background-3`` is first displayed as a whole frame, then changes to ``background-3-frag0:progress`` when the high priority frame is available. After transmitting the high priority frame, the remaining fragment of ``background-3-frag1`` is transmitted.

.. In general, the any preempted frame appears three times:

.. a frame eleteben 3 kulonbozo esemenyt jelez a 3 sor; meselosebb

   1. At the start of transmission, when the MAC still intended to send the frame unfragmented (``background-3``)
   2. When the time sensitive frame arrives at the Ethernet MAC; ``background-3`` becomes ``background-3-frag0:progess`` (the MAC will actually stop transmitting the frame after sending an FCS, but this is still in the future at the time)
   3. The end of first fragment (``background-3-frag0:end``)

   **TODO** generic (example frame); for any preemted frame (not just the first fragment);

The frame sequence is displayed in the Qtenv packet log:

.. figure:: media/packetlog5.png
   :align: center
   :width: 100%

As mentioned in the previous section, a preempted frame appears multiple times in the packet log, as updates to the frame are logged. At first, ``background-3`` is logged as an uninterrupted frame. When the high priority frame becomes available, the frame name changes to ``background-3-frag0``, and it's logged separately. Actually, only  one frame named ``background-3-frag0`` was sent before ``ts-1``, but with three separate packet updates.

The same frame sequence is displayed on a sequence chart on the following images, with a different frame selected and highlighted on each image. Note that the timeline is non-linear:

.. figure:: media/seqchart4.png
   :align: center
   :width: 100%

Just as in the packet log, the sequence chart contains the whole uninterrupted ``background-3`` frame, as it's logged when its transmission is started. 

.. .. note:: From the packet log and the sequence chart it might seem that a 1200B packet was sent (as it was logged that way). This can be confusing, but it's actually the proper operation of the packet log and the sequence chart tool.

.. note:: You can think of it as there are actually two time dimensions present on the sequence chart: the events and messages as they happen at the moment, and what the modules "think" about the future, i.e. how long will a transmission take. In reality, the transmission might be interrupted and so both the original (``background-3``) and the "updated" (``background-3-frag0``) is present on the chart. **TODO** this might be redundant

Here is the frame sequence on a sequence chart on a linear timeline, with the ``background-3-frag0`` frame highlighted:

.. figure:: media/linear.png
   :align: center
   :width: 100%

Note that ``background-3-frag0:progess`` is very short (it basically contains just an updated packet with an FCS, as a remaining data part of the first fragment). Transmission of ``ts-1`` starts after a short interframe gap.

Here is the same frame sequence displayed in Wireshark:

.. figure:: media/wireshark.png
   :align: center
   :width: 100%

The frames are recorded in the PCAP file at the end of the transmission of each frame or fragment, so the 1200B frame is not present there, only the two fragments.

In the Wireshark log, ``frame 5`` and ``frame 7`` are the two fragments of ``background-3``. Note that FPP refers to `Frame Preemption Protocol`; ``frame 6`` is ``ts-1``, sent between the two fragments.

Here is ``background-3-frag1`` displayed in Qtenv's packet inspector:

.. figure:: media/packetinspector5.png
   :align: center
   :width: 100%

This fragment doesn't contain a MAC header, because it's the second part of hte original Ethernet frame.

.. **TODO** without highlight

The paths that the high and low priority (express and preemptable) packets take in the :ned:`EthernetPreemptingMayLayer` is illustrated below by the red lines:

.. figure:: media/preemptible2.png
   :align: center

.. figure:: media/express2.png
   :align: center

Analyzing End-to-end Delay
~~~~~~~~~~~~~~~~~~~~~~~~~~

Comparing the Delay for the Three Cases
+++++++++++++++++++++++++++++++++++++++

To analyze the comparable packet length results, we plot the mean end-to-end delay of the UDP packets for the three cases on the following chart. Note that the configuration is indicated with line style, the traffic category with color:

.. figure:: media/delay.png
   :align: center
   :width: 80%

.. In the case of the ``Default`` configuration, the MAC stores packets in a FIFO queue.
   Thus, higher-priority packets wait in line with the lower-priority packets, before getting sent eventually.

Estimating Delay for the Default Configuration
**********************************************

In the case of the ``Default`` configuration, the MAC stores both background and high-priority packets in the same FIFO queue. Thus, higher-priority packets wait in line with the lower-priority packets, before getting sent eventually.

Due to the limited queue length, the queue doesn't stay empty, so the delay for the frames of both traffic categories is about the same: it is rougly the transmission duration of a frame + queueing delay + interframe gap.
The transmission duration for a 1200B frame on 100Mbps Ethernet is about 0.1ms. On average, there are two frames in the queue so frames wait two frame transmission durations in the queue. The interframe gap for 100Mbps Ethernet is 0.96us, so we assume it negligable:

``delay ~= txDuration + 2 * txDuration + IFG = 3 * txDuration = 0.3ms``

Estimating Delay for the PriorityQueue Configuration
****************************************************

In the case of the ``PriorityQueue`` configuration, higher-priority frames wait in their own sub-queue in the PriorityQueue module in the MAC. If there are high priority frames present in the queue, the MAC will finish the ongoing low-priority transmission (if there is any) before beginning the transmission of the next high-priority frame. 
High-priority frames can be delayed, as the transmission of the current frame needs to finish before sending the high-priority frame.

Using a priority queue decreases the delay of the time sensitive frames and increases that of the background frames. 
Due to high background traffic, a frame is always present in the background queue. A time sensitive frame needs to wait until the background frame transmission finishes; on average, the remaining duration is half the transmission duration of a background frame:

``delay ~= txDuration + 0.5 * txDuration + IFG = 1.5 * txDuration = 0.15ms``

Estimating Delay for the Preemption Configuration
*************************************************

In the ``Preemption`` configuration, in addition to the higher-priority frames having their own queue, the MAC almost immediately stops transmitting the current low-priority frame, and sends the high-priority frame instead.

Thus using preemption is even more effective in reducing the delay of high priority frames. The delay is roughly the duration of an FCS + transmission duration + interframe gap. The duration of an FCS is about 1us, so we neglect it in the calculation:

``delay = txDuration + fcsDuration + IFG ~= txDuration = 0.1ms``

The calculated values above roughly match the results of the simulation.

Realistic Traffic
+++++++++++++++++

The mean end-to-end delay for the realistic traffic case is plotted on the following chart:

.. figure:: media/realisticdelay.png
   :align: center
   :width: 80%

The range indicated by the rectangle on the chart above is shown zoomed in on the chart below, so that its more visible:

.. figure:: media/realisticdelay_zoomed.png
   :align: center
   :width: 80%

The end-to-end delay for preemption is about the transmission duration of a time sensitive frame in case of both the realistic and the comparable length traffic. This is expected, because when preemption is used, the currently transmitting packet is interrupted as soon as a high priority frame becomes available, regardless of the low priority frame's length.

In case of the realistic traffic, the delay of the background frames is not affected by the delay reduction techniques, but the delay of the time sensitive frames are reduced significantly by them, because the traffic is different (originally, both the background and time sensitive packets, so they could be compared for better demonstration).