# thunder-chat

`thunder-chat` is light-weight and very fest and simple multi-threaded chatting server.
It is working on ios and centos.
thunder-chat is using kqueue as io-multiplexing on IOS and Freebsd and using epoll on CentOS.

Thunder-chat uses epoll edge-trigger or edge-trigger + epoll oneshot in Centos.


## system setting
```
sysctl -p
setenforce 0
ulimit -n 102400
ulimit -c 10000 # core

sysctl -w net.core.somaxconn=65535


/etc/security/limits.d/custom.conf
root soft nofile 1000000
root hard nofile 1000000
* soft nofile 1000000
* hard nofile 1000000


/etc/sysctl.conf
fs.file-max = 1000000
fs.nr_open = 1000000
net.ipv4.netfilter.ip_conntrack_max = 1048576
net.nf_conntrack_max = 1048576
```


## prerequisite (centos)

```
sudo yum install cmake -y
sudo yum install -y gcc gcc-c++ curl-devel openssl-devel
sudo touch /usr/include/stropts.h
```


### compile

```
mkdir build
cd build
cmake ..
make
mv ChatServer ../
```

