Case Study: Data Staging
########################

**Overview**

NCSA has an existing cluster where a significant portion of submitted
jobs require input data to be staged from an hierarchical storage
manager (HSM).  Currently, each user must stage the input data for each
job manually before submitting the job.  Also, each job must be
intelligent enough to determine if its input data is present before
beginning execution.  Not only is this inconvenient for users, it can
lead to scheduling inefficiencies as "running" jobs sit idle waiting for
input data to arrive. 

NCSA's cluster consists of 100 single processor nodes connected by a
switched gigabit network.  The storage manager is a 200 terabyte tape
storage system with 4 terabyte disk cache. 

**Goals**

The goals of allowing Moab to manage the data staging requirements of a
cluster include:

*Efficiency*
The scheduler must not start a job until its input data has been staged
to the appropriate location.
*Adaptation*
The scheduler must make intelligent decisions based on the estimated
completion time of data staging operations.
*Space Allocation*
The scheduler must maintain enough free disk space to accommodate the
stage-out data of currently running jobs.
**Analysis**

Moab supports a `STAGEIN <../13.3rmextensions.html#STAGEIN>`__ resource
manager extension that allows the user to specify stage-in requirements
for a job.  If this extension is present, Moab will not attempt to start
the job until the specified file is present.  Additionally, if a size is
given, Moab will wait until the file exists and is at least as big as
the specified size. 

Moab will estimate the time required for a staging operation to complete
if the `BANDWIDTH <../13.2rmconfiguration.html#bandwidth>`__ parameter
in ``moab.cfg`` is set.  This allows Moab to make properly timed,
pre-execution data stage requests from the storage manager. 

The `TARGETUSAGE <../13.2rmconfiguration.html#targetusage>`__ parameter
can be used to set the desired storage manager disk cache utilization
level.  It should be set below 100% to allow for unexpected external
usage and job stage-out files with sizes that are not known in advance. 

**Configuration**

A resource manager named ``hsm`` will be created to monitor and manage
the storage manager load and resources. 

*FIX: Where does transfer rate info go???*


::

    # configure the storage manager
    RMCFG[hsm] TYPE=NATIVE
    RMCFG[hsm] RESOURCETYPE=STORAGE TARGETUSAGE=80%

    # gives Moab information about the storage manager
    RMCFG[hsm] CLUSTERQUERYURL=exec:///$TOOLSDIR/dstage.clusterquery.pl

    # allow Moab to check for the existence of a file and its size
    RMCFG[hsm] SYSTEMQUERYURL=exec:///$TOOLSDIR/dstage.systemquery.pl

    # allow Moab to request a staging operation
    RMCFG[hsm] SYSTEMMODIFYURL=exec:///$TOOLSDIR/dstage.systemmodify.pl


The following script files are provided with Moab.  They can be
customized to work with any type of data staging solution.  In addition,
there are many scripts provided with Moab that work with 3rd party
storage solutions without the need for any customization. 

-  `dstage.clusterquery.pl <../dstage.clusterquery.pl>`__
-  `dstage.systemquery.pl <../dstage.systemquery.pl>`__
-  `dstage.systemmodify.pl <../dstage.systemmodify.pl>`__

More information about these resource manager interfaces can be found in
the `data staging <../18.1datastaging.html>`__ documentation. 

**Monitoring**

The `checkjob <../commands/checkjob.html>`__ command will print the data
staging requirements of a job, if any, and the status of the data
staging operation. 
**Conclusions**

Moab will make the best use of computing resources when it understands
the data staging needs of its jobs and the speed at which staging
requests can be met by the storage manager. 
