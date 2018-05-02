* Dependencies

- C++ 11
- Boost 

* Build

```
mkdir build
cd build
cmake ..
make -j 4
```

* Running

** Server

```
server/server
```

** Cliente

Put a text on database:
```
client/client put 1 Makefile
```

Get concatenated text:
```
client/client get
```


