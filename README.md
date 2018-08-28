# thunder-chat

`thunder-chat` is simple multi-threaded chatting server.



## freebsd + macos

### prerequisite

```
```

### compile

add `-D_FREEBSD` from Makefile



## centos

### prerequisite

```
sudo yum install -y gcc gcc-c++ curl-devel openssl-devel
sudo touch /usr/include/stropts.h
```

### compile

remove `-D_FREEBSD` from Makefile


## Test

