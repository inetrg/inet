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

Overview
~~~~~~~~

.. **V1** In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. When a high priority frame becomes available during the transmission of a low priority frame, the Ethernet MAC can interrupt the transmission of the low priority frame. After sending the high priority frame, the MAC continues the transmission of the low priority frame from where it left off, eventually sending the low priority frame in two (or more) fragments.

In time-sensitive networking applications, Ethernet preemption can significantly reduce latency. When a high priority frame becomes available during the transmission of a low priority frame, the Ethernet MAC can interrupt the transmission of the low priority frame, and start sending the high priority frame immediately. The MAC can continue transmission of the low priority frame from where it left off, eventually sending the low priority frame in two (or more) fragments. This technique guaranties a very low upper bound for latency for high priority frames, which is almost equal to the transmission delay.


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

Preemption is a feature of INET's composable Ethernet model. It uses packet streaming, so that packet transmission is represented as a stream which can be interrupted. Preemption requires the :ned:`LayeredEthernetInterface`, which contains a MAC and a PHY layer, displayed below:

.. figure:: media/LayeredEthernetInterface2.png
   :align: center

To enable preemption, the default submodules :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` need to be replaced with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`.

.. It requires the following modules in the composable Ethernet interface (:ned:`LayeredEthernetInterface`)

The :ned:`EthernetPreemptingMacLayer` contains two sub-MAC-layer submodules, a preemptable (:ned:`EthernetStreamingMacLayer`) and an express MAC layer (:ned:`EthernetFragmentingMacLayer`), each with its own queue for frames:

.. figure:: media/mac.png
   :align: center

:ned:`EthernetPreemptingMacLayer` uses intra-node packet streaming. Discrete packets come in the MAC module from the higher layers, but the sub-MAC-layers (express and preemptable) output packet streams. Packets are sent out as streams out of the MAC layer, through the PHY layer and through the Ethernet connection.

During preemption, packets initially stream from the preemptable sub-MAC-layer. The scheduler notifies the preempting server when a packet arrives at the express MAC. The server stops the preemptable stream, sends the express stream, and then it restarts the preemptable steam.

.. **V2** Here is an example for the preemption process:

   - packets initially stream from the preemptable sub-mac-layer
   - The scheduler notifies the preempting server when a packet arrives at the express mac
   - The server stops the preemptable stream, sends the express stream, and then it restarts the preemptable steam

Interframe gaps between the packets are inserted by the PHY layer. Note that only one frame can be fragmented at any given moment.

The :ned:`EthernetPreemptingPhyLayer` supports both packet streaming and fragmenting (sending packets in multiple fragments).

.. **TODO** supports streaming and fragmenting (which is tobb darabban kuld el csomagokat)

.. ------------------------------

.. **TODO** `not here?` Preemption requires the :ned:`EthernetPreemptingPhyLayer`. **TODO** its implemented in/needed for packet streaming

.. **TODO** we use LayeredEthernetInterface everywhere

.. Preemption requires packet streaming so that packet transmission is represented as a stream so that it can be interrupted.

Configuration
~~~~~~~~~~~~~

The simulation uses the following network:

.. figure:: media/network.png
   :align: center

It contains two :ned:`StandardHost`'s connected with 100Mbps Ethernet, and also a :ned:`PcapRecorder` to record PCAP traces.

.. ---------------------------

.. **V1** There are three configurations in omnetpp.ini, for the following three cases:

   - ``Default``: The baseline configuration; doesn't use any latency reduction techniques
   - ``PriorityQueue``: Uses priority queue in the Ethernet MAC for lower delay of high priority frames
   - ``Preemption``: Uses preemption for high priority frames for even lower delay than the priority queue case, because the ongoing transmission of low priority frames doens't need to finish before sending the high priority frame

   Primarily, we run these simulations with the same packet length for the low and high priority traffic, so that transmission delay is the same for the two. This way the end-to-end delay can be compared. Additionally, we demonstrate the three cases with more realistic traffic (longer low priority and shorter high priority frames); these simulations are the extension of the three mentioned above, and are defined in the configurations with the ``Realistic`` prefix.

.. ---------------------------

.. **V2** Primarily, we run simulations with the same packet length for the low and high priority traffic, so that transmission delay is the same for the two. This way the end-to-end delay can be compared. There are three configurations in omnetpp.ini, for the following three cases:

Primarily, we run simulations with the same packet length for the low and high priority traffic, so that we can compare the end-to-end delay in the following three configurations in omnetpp.ini:

- ``Default``: The baseline configuration; doesn't use any latency reduction techniques
- ``PriorityQueue``: Uses priority queue in the Ethernet MAC for lower delay of high priority frames
- ``Preemption``: Uses preemption for high priority frames for a very low delay with a guarantied upper bound

.. - ``V1 Preemption``: Uses preemption for high priority frames for even lower delay than the priority queue case, because the ongoing transmission of low priority frames doens't need to finish before sending the high priority frame. This technique guaranties a minimum delay.
   - ``V2 Preemption``: Uses preemption for high priority frames for even lower delay than the priority queue case. This technique guaranties a minimum delay.
   - ``V3 Preemption``: Uses preemption for high priority frames for a guarantied, even lower delay than the priority queue case, because the ongoing transmission of low priority frames doens't need to finish before sending the high priority frame.

.. **TODO**

.. **V1** Additionally, we demonstrate these three delay reduction techniques with more realistic traffic (longer and more frequent low priority and shorter, less frequent high priority frames); these simulations are the extension of the three mentioned above, and are defined in ini file as the configurations with the ``Realistic`` prefix.

Additionally, we demonstrate the above delay reduction techniques with more realistic traffic (longer and more frequent low priority and shorter, less frequent high priority frames); these simulations are the extension of the three configurations mentioned above, and are defined in ini file as the configurations with the ``Realistic`` prefix.

.. ---------------------------

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

.. ------------------------------

.. **TODO** not so detailed

We set up high background traffic (96 Mbps) and lower time-sensitive traffic (9.6 Mbps); both send 1200B packets:

.. literalinclude:: ../omnetpp.ini
   :start-at: app[0].source.packetLength
   :end-at: app[1].source.productionInterval
   :language: ini

.. .. note:: Both UDP apps send packets with the same length so that transmission time for both priorities are the same; this way the delay for the different configurations can be compared.

.. **TODO** seems redundant -> it is. make sure its contained above.

In the ``Default`` configuration, no preemption or priority queue is used; the configuration just limits the :ned:`EthernetMac`'s queue length to 4. 

In all three cases, the queues need to be short to decrease the queueing time's effect on the measured delay. However, if they are too short, they might be empty too often, which renders the priority queue useless (it cannot prioritize if it contains just one packet, for example). The queue length of 4 is an arbitrary choice. The queue type is set to :ned:`DropTailQueue` so that it can drop packets if the queue is full:

.. not include queueing time in the measured end-to-end delay **TODO** false:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-at: typename
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Default
   :end-before: Config
   :language: ini

In the ``PriorityQueue`` configuration, we change the queue type in the Mac layer from the default :ned:`PacketQueue` to :ned:`PriorityQueue`:

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-at: classifierClass
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config PriorityQueue
   :end-before: Config
   :language: ini

The priority queue needs two internal queues, for the two traffic categories; to limit the queueing time's effect on the measured end-to-end delay, we also limit the length of internal queues to 4. We also disable the shared buffer, and set the queue type to :ned:`DropTailQueue`. We configure the priority queue's classifier to classify packets based on the VLAN ID request, which indicates the traffic category.

.. **TODO** too much we configure

.. **TODO** we'll use VLAN tags to indicate the traffic categories. The UDP apps put VLAN tags to the packets they create, 

In the ``Preemption`` configuration, we replace the :ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer` modules default in :ned:`LayeredEthernetInterface` with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, which support preemption.

.. TODO there is no priority queue; there are queues in the two submacs; lehet a merged macben is shared queue; 

.. **V2** In the **Preemption** configuration, we replace the default MAC and PHY layer submodules of :ned:`LayeredEthernetInterface` (:ned:`EthernetMacLayer` and :ned:`EthernetPhyLayer`) with :ned:`EthernetPreemptingMacLayer` and :ned:`EthernetPreemptingPhyLayer`, which support preemption.

.. .. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :end-at: dropperClass
   :language: ini

.. literalinclude:: ../omnetpp.ini
   :start-at: Config Preemption
   :end-at: DropTailQueue
   :language: ini

.. note:: There is no priority queue in this configuration, but the sub-mac-layer modules each have their own queue. It is also possible to disable these internal queues and have just one shared queue in the EthernetPreemptableMac module.

We also limit the queue length to 4, and configure the queue type to be :ned:`DropTailQueue`.

We use the following traffic for the ``RealisticDefault``, ``RealisticPriorityQueue`` and ``RealisticPreemption`` configurations:

.. literalinclude:: ../omnetpp.ini
   :start-after: Config RealisticBase
   :end-before: Config RealisticDefault
   :language: ini

.. **TODO**

   so

   - the same 1200B packets is not very realistic in TSN networks
   - more like sporadic short frames for high priority
   - and more frequent longer frames for low priority

In this traffic configuration, high priority packets are a 100 times less frequent, and have 1/10th of the size of low priority packets.

Results
-------

In the case of the ``Default`` configuration, the MAC stores packets in a FIFO queue.
Thus higher priority packets wait in line with the lower priority packets, before getting sent eventually.

In the case of the ``PriorityQueue`` configuration, higher priority frames wait in their own sub-queue in the PriorityQueue module in the MAC. If there are high priority frames present in the queue, the MAC will send them first, or after finishing the transmission of the current low priority frame. High priority frames can be delayed, as the transmission of the current frame needs to finish before sending the high priority frame.

In the ``Preemption`` configuration, in addition to the higher priority frames having their own queue, the MAC almost immediately stops transmitting the current low priority frame, and sends the high priority frame instead.


.. After the high priority frame transmission is complete, it sends the remaining fragment of the low priority frame.

.. **TODO** for even better delay

.. **TODO** remove duplicates

.. In some inspectors, frames are displayed as they happen in the simulation. These inspectors include the animation and the packet log in Qtenv, and the event log file/sequence chart tool. In these the background3 frame is notified/logged/displayed/recorded three times:

.. In the animation and the packet log in Qtenv, and the event log file/sequence chart tool, frames are displayed or recorded as soon as they happen in the simulation. In these tools, the ``background-3`` frame is recorded three times:

In the animation, the packet log, and the event log file/sequence chart tool, frames are displayed or recorded as soon as they happen in the simulation. In these features, the ``background-3`` frame is recorded three times:

.. **TODO** too complex; 2 parts?

- At the start of transmission, when the MAC still intended to send the frame unfragmented (``background-3``)
- When the time sensitive frame arrives at the Ethernet MAC; ``background-3`` becomes ``background-3-frag0:progess`` (the MAC will actually stops transmitting the frame after sending an FCS, but this is still in the future in this moment)
- The end of ``background-3-frag0`` (``background-3-frag0:end``)

.. note:: So from the packet log and the sequence chart it might seem that a 1200B packet was sent (as it was logged that way). This can be confusing, but it's actually the proper operation of the packet log and the sequence chart tool.

In contrast, the frames are recorded in the PCAP file at the end of the transmission of each frame or fragment, so that the 1200B frame is not present there, only the two fragments.

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

The Ethernet MAC in ``host1`` starts transmitting ``background-3``. During the transmission, a time-sensitive frame (``ts-1``) arrives at the MAC. The MAC interrupts the transmission of  ``background-3``; in the animation, ``background-3`` is first displayed as a whole frame, and changes to ``background-3-frag0:progress`` when the high priority frame is available. After transmitting the high priority frame, the remaining fragment of ``background-3-frag1`` is transmitted.

.. so

   - the background-3 frame is updated three times
   - first it starts sending as a complete unfragmented frame
   - when the time sensitive frame arrives at the Ethernet MAC, background-3 becomes background-3 frag-0:progess
   - actually, the Ethernet MAC immediatelly stops transmitting the frame; background-3 frag-0 is very short, it is just an FCS at the frame's end. The end of fragment 0 is background3-frag0:end
   - these updates are present on all displays which displays the frame on the fly, i.e. the packet log, the animation, and the sequence chart. These contain the whole unfragmented frame, and the two fragments as well. The frame is written to the PCAP file after the transmission, so it contains background3 as it was sent, not how the MAC thought it would send it at the time

.. **V2** In the animation, background 3 is displayed as a whole frame (as opposed to a fragment) at first, because the MAC plans to transmit it in one go. Then the ts-0 frame becomes available, and the transmission is interrupted; the frame in the animation changes to background3 frag0. After an interframe gap period, the ts-0 frame is sent. Then the remaining fragment.of background 3.

.. so

   - in general when packet streaming is used, the start and end of frames are recorded
   - in the case of background3, the transmission is about halfway done when ts-0 arrives at the Ethernet MAC
   - the frame changes to background3-frag0:progess

The frame sequence is displayed in the Qtenv packet log:

.. figure:: media/packetlog5.png
   :align: center
   :width: 100%

.. **V1** In the packet log, background-3 is present as a whole frame (1254B), as the MAC was about to send it uninterrupted. When ts-0 becomes available, background-3 frag0 (x B) is logged.

Lines are added to the packet log as they happen in the simulation. At first, ``background-3`` is logged as an uninterrupted frame. When the high priority frame becomes available, the frame name changes to ``background-3-frag0``, and it's logged separately. Actually, only  one frame named ``background-3-frag0`` was sent before ``ts-1``, but with three separate packet updates.

.. a logbol ugy tunik hogy egy 1200Bos csomag atment de nem mert rajott h nem lesz
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

Just as in the packet log, the sequence chart contains the whole uninterrupted ``background-3`` frame, as it's logged when its transmission is started. 

.. This can be confusing, but it's actually the proper operation of the sequence chart tool. 

.. note:: You can think of it as there are actually two time dimensions present on the sequence chart: the events and messages as they happen at the moment, and what the modules "think" about the future, i.e. how long will a transmission take. In reality, the transmission might be interrupted and so both the original (``background-3``) and the "updated" (``background-3-frag0``) is present on the chart.

.. **TODO** background3! consistently

.. ------------------------------

Here is the frame sequence on a sequence chart on a linear timeline, with the ``background-3-frag0`` frame highlighted:

.. figure:: media/linear.png
   :align: center
   :width: 100%

.. **V2!** Here is the frame sequence on a sequence chart on a linear timeline:

   .. figure:: media/linear2.png
      :align: center
      :width: 100%

   ------------------------------

   **TODO** the background3 frag0 frame is just a minimum ethernet frame long -> its just an FCS

Note that ``background-3-frag0:progess`` is very short (it basically contains just an updated packet with an FCS, as a remaining data part of the first fragment). Transmission of ``ts-1`` starts after a short interframe gap.

Here is the same frame sequence displayed in Wireshark:

.. figure:: media/wireshark.png
   :align: center
   :width: 100%

.. In the Wireshark log, ``frame 5`` is the first fragment of ``background-3``. Note that FPP refers to `Frame Preemption Protocol`. 

In the Wireshark log, ``frame 5`` and ``frame 7`` are the two fragments of ``background-3``. Note that FPP refers to `Frame Preemption Protocol`; ``frame 6`` is ``ts-1``, sent between the two fragments.

.. - wireshark displays fragmented ethernet frames weirdly
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

Here is ``background-3-frag1`` displayed in Qtenv's packet inspector:

.. figure:: media/packetinspector5.png
   :align: center
   :width: 100%

.. **TODO** -> you can see the phy header details -> elso header-t is!
ezert nincs benne byteocuntchunk mert nincs benne ethernet mac header 

This fragment doesn't contain a MAC header, because it's the second part of hte original Ethernet frame.

.. **TODO** without highlight

.. At the start of the simulation/at simulation time 0, both the high and the low priority app sends an UDP packet to lower layers. The Ethernet MAC starts transmitting the lower priority frame **TODO** why

The paths that the high and low priority (express and preemptable) packets take in the :ned:`EthernetPreemptingMayLayer` is illustrated below by the red lines:

.. figure:: media/preemptible2.png
   :align: center

.. figure:: media/express2.png
   :align: center

We plot the mean end-to-end delay of the UDP packets for the three cases on the following chart; note that the configuration is indicated with line style, the traffic category with color:

.. figure:: media/delay.png
   :align: center
   :width: 100%

.. **V1** As expected, in the default case, the delay for the two traffic categories are the same. The priority queue decreases the delay of the high priority frames and increases that of the low priority frames. Using preemption is even more effective in reducing the delay of high priority frames.

   In case of the default configuration, there is only one queue. the queue is not empty, so the delay for the background and time sensitive frames is about the same, due to the limited queue length.

   In the priority queue configuration, there is always a frame in the background queue. The time sensitive frame needs to wait until the background frame transmission finishes; on average, the remaining duration is half the transmission duration of a background frame.

   In case of preemption, the delay is roughly the duration of an FCS + transmission duration + interframe gap.

.. As expected, in the default case, the delay for the two traffic categories are the same. 

In case of the default configuration, there is only one queue. The queue is not empty, so the delay for the background and time sensitive frames is about the same, due to the limited queue length.
The delay for both traffic categories is rougly the transmission duration of a frame + queueing delay + interframe gap.
The transmission duration for a 1200B frame on 100Mbps Ethernet is about 0.1ms. On average, there are two frames in the queue so frames wait two frame transmission durations in the queue. The interframe gap for 100Mbps Ethernet is 0.96us, so we assume it negligable:

``delay ~= txDuration + 2 * txDuration + IFG = 3 * txDuration = 0.3ms``

Using a priority queue decreases the delay of the time sensitive frames and increases that of the background frames. 
A frame is always present in the background queue. The time sensitive frame needs to wait until the background frame transmission finishes; on average, the remaining duration is half the transmission duration of a background frame:

``delay ~= txDuration + 0.5 * txDuration + IFG = 1.5 * txDuration = 0.15ms``

Using preemption is even more effective in reducing the delay of high priority frames. The delay is roughly the duration of an FCS + transmission duration + interframe gap. The duration of an FCS is about 1us, so we neglect it.

``delay = txDuration + fcsDuration + IFG ~= txDuration = 0.1ms``

The calculated values above roughly match the results of the simulation.

.. **TODO** szamolas hogy jon ki roughly

.. Using preemption has a similar effect but its more effective/Using preemption produces a similar effect but more effective/

.. .. figure:: media/preemptive.png
   :align: center
   :width: 90%

.. .. figure:: media/express.png
   :align: center
   :width: 90%

.. **TODO** realistic

The mean end-to-end delay for the realistic traffic case is plotted on the following charts (the range indicated by the rectangle in the first chart is shown zoomed in on the second, so that its more visible):

.. figure:: media/realisticdelay.png
   :align: center
   :width: 100%

.. figure:: media/realisticdelay_zoomed.png
   :align: center
   :width: 100%

.. The delay for preemption is the same as in the comparable length packet case. This is expected, because when preemption is used, the currently transmitting packet is interrupted as soon as a high priority frame becomes available, regardless of the low priority frame's length. **TODO**

The end-to-end delay for preemption is about the transmission duration of a time sensitive frame in case of both the realistic and the comparable length traffic. This is expected, because when preemption is used, the currently transmitting packet is interrupted as soon as a high priority frame becomes available, regardless of the low priority frame's length.

.. In these charts, the delay of the time sensitive frames in all cases is better than in the comparable packet length case, and that of the background frames is worse. But its because the queues are emptier. **TODO** ?

   - realistic: background traffic szamaro completely irrelevant of what kind of reduction techniques is used. because the traffic is different
   - nagyobb a hatasa a delay reduction methodnak
   (azert volt originally ugyanakkora hogy comparable legyen hogy meg lehessen erteni)

   The delay of the background traffic is not affected by any of the delay reduction techniques;  for the time sensitive traffic, the delay reduction techniques have a greater affect compared to the original, same packet length configurations.

   The delay of the background frames is not affected by any of the delay reduction techniques; the delay reduction techniques have a greater effect on the time sensitive frames.

In case of the realistic traffic, the delay of the background frames is not affected by the delay reduction techniques, but the delay of the time sensitive frames are reduced significantly by them, because the traffic is different (originally, both the background and time sensitive packets, so they could be compared for better demonstration).

.. **TODO**

.. Here is part of the above chart zoomed in (the displayed range is indicated by the rectangle in the above chart), so that its more visible:

.. **TODO**

   - in case of the default, if the queues are not empty, the delay should be about the same
   - in case of the priority queue, the queues are not empty, so there is always a frame in the background queue;
     the incoming time sensitive frame needs to wait -> about half a frame on average
   - in case of the preemption, the delay is the transmission duration + interframe gap

.. ?he priority queue, there is always a frame in the background queue, so there is always a transmission. The time sensitive frame needs to wait until the background frame transmission finishes; on average, the remaining duration is half the transmission durat