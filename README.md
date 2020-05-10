Velo Blockchain Agent
=====================

The Velo Blockchain Agent is a service that provides the ability for authorized
users to query the Velo Blockchain or submit transactions to this blockchain.

Building
--------

Building this project requires that the Velo Toolchain has been successfully
installed, and that this either exists in the default installation location, or
that `TOOLCHAIN_DIR` is specified, pointing to this location.

To build, run the following command:

    make

By default, this runs a single-threaded build.  The jobs parameter can be set to
the number of cores on the build machine to parallelize the build.  For
instance, to build using 8 cores:

    make -j8

Testing
-------

To run the unit testing suite, run the following command:

    make test

To run the model checking suite, run the following command:

    make model-check

Installation
------------

To install `agentd`, the command `make install` should be run as root.  By
default, `agentd` will be installed to `/opt/velo-blockchain`.  This can be
overridden using the `PREFIX` environment variable.

    make install

By default, an example root blockchain and simple secrets will be used.  These
are _not secure_, but can be used to demonstrate that the blockchain agent is
behaving correctly.  `blocktool`, which is a separate Velo Blockchain utility,
can be used to generate a root block and secrets for an agent installation.
Additionally, `agentd` is set up with a default configuration file that points
to secrets and a data store created in the install prefix.  The `agentd` service
boots as root and chroots child processes under this prefix by default.  All of
this can be controlled via `agentd.config`, which is installed by default in
`/opt/velo-blockchain/etc`.

Configuration
-------------

The `agentd.config` file uses a simple language for describing configuration.
Paths are relative to the installation prefix, or if a custom configuration is
loaded via the `-c configFile` command-line option, the path is relative to the
location of that config file.

To customize logging, the `logdir` and `loglevel` attributes are specified.
The `logdir` takes a relative or absolute path to an existing directory used for
writing log files.  The `loglevel` attribute represents a number between `0` and
`9` representing the verbosity level for the log file.  A `0` `loglevel`
indicates that logging is disabled.  A `9` `loglevel` is the highest level of
verbosity available for the service.

    logdir log
    loglevel 3

The `canonization` section is used by the canonization service (when configured)
to manage how often the process queue is scanned for attested transactions and
how many transactions can be put in a new block.  The following example scans
the process queue every 5 seconds and creates blocks with a maximum of 1000
transactions per block.

    canonization {
        max milliseconds 5000
        max transactions 1000
    }

The `secret` attribute specifies the local path to a private key certificate for
the agent.  This should be readable only by root, and should never be included
in a container.  In the future, support for secrets wiring through a one-time
service call will be added.

    secret secret/agent-46801f7a-8991-40d5-9ada-87c6b1ab352d.cert

The `rootblock` attribute specifies the local path to the root block
certificate, which is used to bootstrap a given blockchain.

    rootblock rootblock/blockchain-620bdf07-20d8-4769-a94a-be4a1bfe5fc4.cert

The `datastore` attribute specifies the location for the agent's datastore.
This must be an existing directory.

    datastore data

The `listen` attribute specifies a domain name / IP address and port to which
the agent listens for connections from peers.

    listen 0.0.0.0:941

The `chroot` attribute specifies the location for the new root where child
processes for the agent service will live.  The parent agent service acts as a
reincarnation service for the agent child services; each child service runs in a
reduced privilege mode and is rooted in the following location.  Note that this
workdir should be owned by root, and should only be readable and executable by
root.  All other permissions should be stripped.

    chroot workdir

The `usergroup` attribute specifies the user and group that should be used for
the child agents, separated by a colon.  This should be a `nologin` account with
reduced capabilities in the system and a ulimit that makes sense for the
blockchain service.

    usergroup veloblock:veloblock
