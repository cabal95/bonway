bonway
======

Configurable Bonjour gateway written in C. Designed to allow filtering
based on protocol and network interface.

Installation
============

Bonway should build fine on recent versions of Fedora and Ubuntu. Currently it
is known to build on Fedora 18 and Fedora 14.04. To build simply make sure
you have the build system installed (gcc, make, etc.) and then run *make*
to build. After it is built copy the **bonway** executable to the
**/usr/local/sbin** directory.

Currently there is only a boot-script for Ubuntu. Copy the
**contrib/ubuntu.init** script into **/etc/init/bonway.init** and then
run *update-rc.d bonway defaults* to activate the script. It should then
auto-start at the next boot.

Configuration
=============

Working example:

```
service {
	type = { "_airplay._tcp", "_raop._tcp" }
	server = { "eth2" }
	client = { "eth0", "eth1" }
}

service {
	type = { "_http._tcp" }
	server = { "eth0", "eth1", "eth2" }
	client = { "eth0", "eth1", "eth2" }
}
```

The *service* declaration defines an instance of a service (or multiple
related services) that you want to allow.

- type - This allows you to enter the service types that will be allowed
in this service declaration.
- server - This identifies which interface(s) host the service (for example,
the interfaces used to reach your Apple TV).
- client - Identifies the interface(s) which will be allowed to access your
Apple TV.

Remember, bonway is not a content filter, just a service filter. By *allowed*
I do not mean it will block the actual connection, just wether or not the
service is broadcasted.

In the above example we are allowing two types of bonjour services. The first
is Airplay, which uses both _airplay._tcp and _raop._tcp to advertise itself.
The Apple TVs that are accessible on *eth2* will have their services
broadcasted to *eth0* and *eth1*. If an Apple TV exists on *eth1* it will
**not** show up on either *eth0* or *eth2*.

The second example is for bonjour HTTP services, _http._tcp. Since all three
interfaces are listed in both the client and server specifications, any HTTP
bonjour service that is observed on any of the three interfaces will be
rebroadcasted on the other two (it will not re-broadcast on the interface it
came from).
