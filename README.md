# thunder-chat

`thunder-chat` is simple multi-threaded chatting server.



## prerequisite (centos)

```
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

