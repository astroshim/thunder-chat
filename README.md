# thunder-chat

`thunder-chat` is light-weight and very fest and simple multi-threaded chatting server.
It is working on ios and centos.
thunder-chat is using kqueue as io-multiplexing on IOS and Freebsd and using epoll on CentOS.


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

