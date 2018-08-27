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
yum install gcc
yum install gcc-c++
yum install curl-devel
yum install openssl-devel
touch /usr/include/stropts.h
```

### compile

remove `-D_FREEBSD` from Makefile


## Test

