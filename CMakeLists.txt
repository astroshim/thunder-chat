project("Thunder-Chat")

if(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
endif()

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()


if(LINUX)
  message(STATUS ">>> Linux")
                    
  add_definitions(-D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_ONESHOT -D_DEBUG)

  include_directories(/usr/local/opt/openssl/include)

  add_executable(ChatServer 
        MainProcess.cpp 		
		./src/CircularBuff.cpp      
		./src/Client.cpp            
		./src/ChatUser.cpp       
		./src/ClientChatServer.cpp
		./src/ClientServer.cpp      
		./src/ClientSocket.cpp      
		./src/IOMP_Select.cpp       
		./src/IOMP_EPoll.cpp       
		./src/Mutex.cpp             
		./src/NPDebug.cpp           
		./src/NPLog.cpp             
		./src/NPUtil.cpp            
		./src/Process.cpp           
		./src/Properties.cpp        
		./src/ServerInfoDN.cpp    
		./src/ServerInfoDNMgr.cpp  
		./src/ServerSocket.cpp      
		./src/SharedMemory.cpp      
		./src/Socket.cpp            
		./src/TcpSocket.cpp         
		./src/Thread.cpp            
		./src/ThreadManager.cpp     
		./src/ThreadAcceptor.cpp    
		./src/ThreadWorker.cpp     
		./src/ThreadReceiver.cpp    
		./src/BroadcastMessage.cpp 
		./src/ThreadBroadcaster.cpp 
		./src/ThreadTic.cpp     
		./src/CircularQueue.cpp 
		./src/ChatManager.cpp  
		./src/ChatServer.cpp		
  )
  
  TARGET_LINK_LIBRARIES( ChatServer
    pthread
    curl
  )
else()
  message(STATUS ">>> MacOS")

  add_definitions(-D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_ONESHOT -D_FREEBSD -D_DEBUG)

  include_directories(/usr/local/opt/openssl/include)

  add_executable(ChatServer 
        MainProcess.cpp 		
		./src/CircularBuff.cpp      
		./src/Client.cpp            
		./src/ChatUser.cpp       
		./src/ClientChatServer.cpp
		./src/ClientServer.cpp      
		./src/ClientSocket.cpp      
		./src/IOMP_Select.cpp       
		./src/IOMP_KQUEUE.cpp       
		./src/Mutex.cpp             
		./src/NPDebug.cpp           
		./src/NPLog.cpp             
		./src/NPUtil.cpp            
		./src/Process.cpp           
		./src/Properties.cpp        
		./src/ServerInfoDN.cpp    
		./src/ServerInfoDNMgr.cpp  
		./src/ServerSocket.cpp      
		./src/SharedMemory.cpp      
		./src/Socket.cpp            
		./src/TcpSocket.cpp         
		./src/Thread.cpp            
		./src/ThreadManager.cpp     
		./src/ThreadAcceptor.cpp    
		./src/ThreadWorker.cpp     
		./src/ThreadReceiver.cpp    
		./src/BroadcastMessage.cpp 
		./src/ThreadBroadcaster.cpp 
		./src/ThreadTic.cpp     
		./src/CircularQueue.cpp 
		./src/ChatManager.cpp  
		./src/ChatServer.cpp		
  )
  
  TARGET_LINK_LIBRARIES( ChatServer
    pthread
    curl
  )

endif()




