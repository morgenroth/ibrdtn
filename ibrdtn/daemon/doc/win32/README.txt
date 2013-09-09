To install the daemon as Windows service, run this command from
a previledged console:

C:\> sc create "DtnStack" binPath= C:\Path\To\dtnserv.exe

The basic service configuration (configuration file, log file, etc.)
are set via Windows Registry. The file config.reg is an example of
such a configuration. For the detailed configuration we use the same
configuration file as on all other platforms.

The interfaces of the daemon must be explicit configured in the
configuration file. The identifier for the interfaces are the unique ID
of them. Not the friendly name or the description!

To list all available interfaces start the dtnd.exe with the
option --interfaces.

