PROJECT ( GameBenchmark )

SET (
	SHARED_SOURCES
	src/game/GameObject.cpp
	src/game/Player.cpp
	src/game/Quest.cpp
	src/game/Region.cpp
	src/utils/Configurator.cpp
	src/utils/IPUtils.cpp
	src/utils/RateMonitor.cpp
	src/utils/RecycleBin.cpp
	src/utils/SDL_barrier.cpp
	src/utils/Serializator.cpp
	src/comm/Message.cpp
	src/comm/MessageModuleIN.cpp
	src/comm/MessageModuleOUT.cpp
	src/comm/MessageOkJoin.cpp
	src/comm/MessageQueue.cpp
	src/comm/MessageRequestClientUpdate.cpp
	src/comm/MessageWithIP.cpp
	src/comm/MessageWithSerializator.cpp
	src/comm/MessageXY.cpp
)

SET (
	CLIENT_SOURCES
	src/client/Client.cpp
	src/client/ClientActionModule.cpp
	src/client/ClientRenderModule.cpp
	src/client/PlayerAI.cpp
	src/client/AStar/AStar.cpp
	src/client/AStar/MinHeap.cpp
	src/client/AStar/NodeSet.cpp
	src/client/AStar/PointStack.cpp
	src/graphics/OglWorld.cpp
	src/graphics/3ds/GLTexture.cpp
	src/graphics/3ds/Model_3DS.cpp
	src/graphics/texture/texture.cpp
	src/graphics/font/BitmapFont.cpp
	${SHARED_SOURCES}
)

SET (
	SERVER_SOURCES
	src/server/CustomPlayerList.cpp
	src/server/MapManagModule.cpp
	src/server/PeriodicEventsModule.cpp
	src/server/PlayerBucket.cpp
	src/server/RegularUpdateModule.cpp
	src/server/Server.cpp
	src/server/ServerData.cpp
	src/server/StatisticsModule.cpp
	src/server/WorldUpdateModule.cpp
	${SHARED_SOURCES}
)

SET (
	MASTER_SOURCES
	src/master/AlgLightest.cpp
	src/master/AlgLocalityAware.cpp
	src/master/AlgSpread.cpp
	src/master/BasicLoadBalance.cpp
	src/master/Master.cpp
	src/master/MasterModule.cpp
	src/master/SenderModule.cpp
	${SHARED_SOURCES}
)

# REQUIRED does not work in CMake <=2.4.6 for SDL
Find_Package ( SDL REQUIRED )
Find_Package ( SDL_net REQUIRED ) # if using SDL_image

# Workaround for the non-working REQUIRED flag
if ( NOT SDL_FOUND )
	message ( FATAL_ERROR "SDL not found!" )
endif ( NOT SDL_FOUND )

LINK_LIBRARIES (
	${SDL_LIBRARY}
	${SDLNET_LIBRARY} # if using SDL_image, obviously
	SDLmain # Sadly not included in SDL_LIBRARY variable
	GL GLU z
)

INCLUDE_DIRECTORIES(src ${SDL_INCLUDE_DIR})

ADD_EXECUTABLE (
   client
   WIN32 # Only if you don't want the DOS prompt to appear in the background in Windows
   MACOSX_BUNDLE
   ${CLIENT_SOURCES}
)

ADD_EXECUTABLE (
   server
   WIN32 # Only if you don't want the DOS prompt to appear in the background in Windows
   MACOSX_BUNDLE
   ${SERVER_SOURCES}
)

ADD_EXECUTABLE (
   master
   WIN32 # Only if you don't want the DOS prompt to appear in the background in Windows
   MACOSX_BUNDLE
   ${MASTER_SOURCES}
)
