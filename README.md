# Multicast Xfer [![Actions Status](https://github.com/Fullaxx/mcast_xfer/workflows/CI/badge.svg)](https://github.com/Fullaxx/mcast_xfer/actions)
A Multicast File Transfer Utility

## Requirements for building
First we need to make sure we have all the appropriate libraries. \
Please consult this chart for help with installing the required packages. \
If your OS is not listed, please help us fill out the table, or submit a request via github.

| OS     | Commands (as root)                                      |
| ------ | ------------------------------------------------------- |
| CentOS | `yum install -y gcc openssl-devel`                      |
| Debian | `apt update; apt install -y build-essential libssl-dev` |
| Fedora | `yum install -y gcc openssl-devel`                      |
| Ubuntu | `apt update; apt install -y build-essential libssl-dev` |

## Compile the code
Install the openssl development package and compile the code
```
cd src
./compile.sh
```

## Prepare your route
In order to use the multicast protocol, you need to have a proper route set. \
Use the ip or route utilities to set a multicast route on a specific network interface.
```
route add -net 224.0.0.0 netmask 240.0.0.0 eth0
```
or
```
ip route add 224.0.0.0/4 dev eth0
```

## Sending and Receiving
Start the receiver with default options
```
./mcastrecv.exe
```

Start the sender with default options
```
./mcastsend.exe <FILE>
```

Start the receiver with options \
Set BAILONMISS to abort current operation on any missed chunks.
```
export BAILONMISS=1
./mcastrecv.exe
```

Start the sender with options \
Set the MTU to send packets no larger than this value. \
Set the SLEEPDELAY to wait this many microseconds before sending another chunk.
```
export MTU=3000
export SLEEPDELAY=250
./mcastsend.exe <FILE>
```
