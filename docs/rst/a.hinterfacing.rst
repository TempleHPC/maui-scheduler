Appendix H: Interfacing to Maui
###############################

Maui interfaces to systems providing various services and using various
protocols. This appendix is designed to assist users who wish to enable
Maui in new environments using one of the existing interfaces. It does
not cover the steps required to create a new interface.

.. rubric:: H.1 Utilizing Maui Client Services
   :name: h.1-utilizing-maui-client-services

The standard Maui distribution provides a scheduler server (maui) and a
number of user clients (showq, showres, etc). By default, these clients
communicate with the scheduler using an internal single use, 'byte count
+ secret key' based TCP connection protocol called simply the
'*SingleUseTCP*' protocol. This protocol is documented in the Wiki
'`Socket Interface <wiki/socket.html>`__ ' section with an overview and
sample code describing how to generate the byte count, timestamp,
encrypted checksum, etc. This protocol is functional but is not a
commonly used standard outside of the Maui project. A further issue with
creating client interfaces is that even though the socket interface is
well defined, the data flowing through this interface to support client
requests is not standardized. As of Maui 3.2.5, some clients receive raw
binary data, others raw text, and still others formatted text ready for
display. This has resulted from the evolutionary nature of the client
interface which has not received a much needed design 'refresh'. The
good news is that this refresh is now under way.

As part of the Department of Energy's 'Scalable Systems Software'
initiative, there have been significant enhancements to the
scheduler/client protocol. Maui now supports multiple socket level
protocol standards in communicating with its clients and peer services.
These including 'SingleUseTCP', 'SSS-HALF', and 'HTTP'. The client
socket protocol can be specified by setting the **MCSOCKETPROTOCOL**
parameter to **SUTCP** , **SSS-HALF**, or **HTTP**. Further protocols
are being defined and standardized over time and backwards compatibility
will be maintained. Documentation on the SSS-HALF implementation can be
found within the DOE's `SSS Project
Notebooks <http://www.scidac.org/ScalableSystems>`__.

+----------+--------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | HTTP support is currently (Oct 24, 2002) in active development and is not expected to be in production use until Maui 3.2.6.   |
+----------+--------------------------------------------------------------------------------------------------------------------------------+

In addition to the socket protocol advances, there has also been work in
the area of standardizing the format in which the client data is
actually transmitted. The SSS project has selected XML as the means to
frame all inter-client data. To date, a number of Maui clients have been
ported over to enable optional use of XML based data framing. These
clients include **mshow** (showq), **showstate**, **checknode** ,
**mjobctl** (runjob, setjobhold, setspri, canceljob), and **mresctl**
(setres, releaseres).

+----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| |Note|   | The XML used in these clients is still evolving. It is expected to be finalized for the clients listed by mid December 2002. If there is interest in working with these protocols or defining specifications, please `contact us <mailto:help@supercluster.org>`__ so we can coordinate any changes and take your needs into account when prioritizing future development.   |
+----------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. rubric:: H.2 Resource Management Interfaces
   :name: h.2-resource-management-interfaces

`See Moab Workload Manager </resources/docs/mwm/a.hinterfacing.html>`__

.. rubric:: H.3 Allocation Management Interfaces
   :name: h.3-allocation-management-interfaces

`See Moab Workload Manager </resources/docs/mwm/a.hinterfacing.html>`__

.. rubric:: H.4 Grid Scheduling System Interfaces
   :name: h.4-grid-scheduling-system-interfaces

`See Moab Workload Manager </resources/docs/mwm/a.hinterfacing.html>`__

.. rubric:: H.5 Information Service Interfaces
   :name: h.5-information-service-interfaces

`See Moab Workload Manager </resources/docs/mwm/a.hinterfacing.html>`__

.. rubric:: H.6 Event Management Services
   :name: h.6-event-management-services

| `See Moab Workload
  Manager </resources/docs/mwm/a.hinterfacing.html>`__
