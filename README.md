# Simple-DNS-Sniffer

Simple DNS sniffer can capture standart DNS request/response.
Also you can change response packets with editing code (see example in main.cpp)

# Building

### For Windows

Open file with empty console project in Visual Studio and click Build

OR

execute in Developer command prompt

```
cl main.cpp
```

### For linux

```
g++ main.cpp
```

# Usage:

1. Run program on first (host) machine (for example this machine has address 192.168.1.2)
2. Connect to network second (target) machine and select for DNS address host address (for example this machine has address 192.168.1.3 and DNS address 192.168.1.2)
