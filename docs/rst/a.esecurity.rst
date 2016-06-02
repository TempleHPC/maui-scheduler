Appendix E: Security
####################

.. rubric:: E.1 Role Based Security Configuration
   :name: e.1-role-based-security-configuration

Maui provides access control mechanisms to limit how the scheduling
environment is managed. The primary means of accomplishing this is
through limiting the users and hosts which are trusted and have access
to privileged commands and data.

With regards to users, Maui breaks access into three distinct levels.

.. rubric:: E.1.1 Level 1 Maui Admin (Administrator Access)
   :name: e.1.1-level-1-maui-admin-administrator-access

A level 1 Maui Admin has global access to information and unlimited
control over scheduling operations. He is allowed to control scheduler
configuration, policies, jobs, reservations, and all scheduling
functions. He is also granted access to all available statistics and
state information. Level 1 admins are specified using the
`ADMIN1 <a.fparameters.html#admin1>`__ parameter.

.. rubric:: E.1.2 Level 2 Maui Admin (Operator Access)
   :name: e.1.2-level-2-maui-admin-operator-access

Level 2 Maui Admins are specified using the
`ADMIN2 <a.fparameters.html#admin2>`__ parameter. The users listed under
this parameter are allowed to change all job attributes and are granted
access to all informational Maui commands.\ ****

.. rubric:: E.1.3 Level 3 Maui Admin (Help Desk Access)
   :name: e.1.3-level-3-maui-admin-help-desk-access

**** Level 3 administrators users a specified via the
`ADMIN3 <a.fparameters.html#admin3>`__ parameter. They are allowed
access to all informational Maui commands. They cannot change scheduler
or job attributes.

.. rubric:: ` <>`__\ E.2 Interface Security
   :name: e.2-interface-security

As part of the U.S Department of Energy SSS Initiative, Maui interface
security is being enhanced to allow full encryption of data and GSI-like
security.

If these mechanisms are not enabled, Maui also provides a *shared secret
key* based security model. Under this model, each client request is
packaged with the client ID, a timestamp, and a encrypted key of the
entire request generated using a secret site selected key (checksum
seed). A default key is selected when the Maui **configure** script is
run and may be regenerated at any time by rerunning **configure** and
rebuilding Maui.

.. rubric:: E.2.1 Configuring Peer Specific Keys
   :name: e.2.1-configuring-peer-specific-keys

Peer-specific secret keys can be specified using the
`CLIENTCFG <a.fparameters.html#clientcfg>`__ parameter. This key
information must be kept secret and consequently can only be specified
in the ``maui-private.cfg`` file. With regards to security, there are
two key attributes which can be set. These attributes are listed in the
table below:

+--------------------------------------------------------------------------+
| **Attribute**                                                            |
| **Format**                                                               |
| **Description**                                                          |
| **Example**                                                              |
+--------------------------------------------------------------------------+
| **CSALGO**                                                               |
| one of **DES**, **HMAC**, or **MD5**.                                    |
| specifies the encryption algorithm to use when generating the message    |
| checksum.                                                                |
| ::                                                                       |
|                                                                          |
|     CLIENTCFG[AM:bank] CSALGO=HMAC                                       |
+--------------------------------------------------------------------------+
| **CSKEY**                                                                |
| STRING                                                                   |
| specifies the shared secret key to be used to generate the message       |
| checksum.                                                                |
| ::                                                                       |
|                                                                          |
|     CLIENTCFG[RM:clusterA] CSKEY=banana6                                 |
+--------------------------------------------------------------------------+

The **CLIENTCFG** parameter takes a string index indicating which peer
service will use the specified attributes. In most cases, this string is
simply the defined name of the peer service. However, for the special
cases of resource and allocation managers, the peer name should be
prepended with the prefix **RM:** or **AM:** respectively, as in
``CLIENTCFG[AM:bank]`` or ``CLIENTCFG[RM:devcluster]``.

.. rubric:: E.2.2 Interface Development Notes
   :name: e.2.2-interface-development-notes

Details about the checksum generation algorithm can be found in the
`Socket Protocol Description <wiki/socket.html>`__ document.
