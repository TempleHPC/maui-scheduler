Appendix J: Maui Differences Guide
##################################

.. rubric:: Maui 3.2.6 patch 10
   :name: maui-3.2.6-patch-10

For full list of changes, see **CHANGELOG** file included with
distribution

-  **Scalability**

   -  Added client data compression

-  **Inter-operability**

   -  Added support for SSS 3.0.3 job structure
   -  Added support for SSS suspend-resume features
   -  Support PBS 'state-unknown' mapping
   -  Improved TORQUE 'status' attribute auto-detection

-  **Security**

   -  Added client data encryption
   -  Added client bounds checking to prevent buffer overflow
   -  Enhanced group ID mapping support

-  **Features**

   -  Added scheduler-level config based node feature specification
   -  Enabled Dynamic Hostlist Modification
   -  Enabled AM Job Failure Action Support
   -  Added support for class/queue level feature requirement defaults
   -  Added support for dedicated resource specification w/in standing
      reservations

-  **Fault Tolerance**

   -  Fixed TORQUE server data auto-detect
   -  Fixed data corruption server crash
   -  Improved logic for stale job handling with failing resource
      managers

-  **Useability**

   -  Improved Node State Diagnostics

-  **Accounting**
