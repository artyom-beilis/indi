cmake_minimum_required(VERSION 3.13)
PROJECT(indilib C CXX)

option(ENABLE_CCD "Build libindi with CCD support" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
          FORCE)
endif(NOT CMAKE_BUILD_TYPE)



set(FLAGS "-fPIC -DINDI_AS_LIBRARY -Wall -Wextra -DHAVE_CLOCK_GETTIME -DHAVE_LIBNOVA")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined") 

find_path(NOVA_INC libnova/utility.h)
find_library(NOVA_LIB NAMES nova nova-0.16)

find_path(GSL_INC gsl/gsl_blas.h)
find_library(GSL_LIB NAMES gsl)

find_path(EV_INC ev.h)
find_library(EV_LIB ev)

include_directories(${NOVA_INC})
include_directories(${EV_INC})
include_directories(${GSL_INC})

set(CMAKE_INDI_VERSION_MAJOR 2)
set(CMAKE_INDI_VERSION_MINOR 1)
set(CMAKE_INDI_VERSION_RELEASE 0)


configure_file(../libs/indicore/indiapi.h.in indiapi.h @ONLY)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    .
    ../libs/sockets
    ../libs/indibase/timer/
    ../libs/dsp
    ../libs
    ../libs/indidevice/property/
    ../libs/indicore
    ../libs/indibase
    ../libs/indidevice
    ../libs/eventloop
    ../libs/indibase/thread/
    ../libs/indibase/stream/
    ../libs/indiclient/
    ../libs/indiabstractclient/ )

if(ENABLE_CCD)
    set(INDILIB_CCD_SRC 
     ../libs/indibase/stream/streammanager.cpp
     ../libs/indibase/stream/fpsmeter.cpp
     ../libs/indibase/stream/gammalut16.cpp
     ../libs/indibase/stream/recorder/recorderinterface.cpp
     ../libs/indibase/stream/recorder/recordermanager.cpp
     ../libs/indibase/stream/recorder/serrecorder.cpp
     ../libs/indibase/stream/encoder/encodermanager.cpp
     ../libs/indibase/stream/encoder/encoderinterface.cpp
     ../libs/indibase/stream/encoder/rawencoder.cpp
     ../libs/indibase/stream/encoder/mjpegencoder.cpp
     ../libs/indibase/indiccdchip.cpp
     ../libs/indibase/indiccd.cpp
     ../libs/indibase/thread/indisinglethreadpool.cpp
     ../libs/indibase/dsp/convolution.cpp ../libs/indibase/dsp/dspinterface.cpp ../libs/indibase/dsp/manager.cpp ../libs/indibase/dsp/transforms.cpp
     ../libs/indibase/indisensorinterface.cpp
     ../libs/indibase/stream/jpegutils.c
     ../libs/indibase/stream/ccvt_c2.c
     ../libs/indibase/stream/ccvt_misc.c
     ../libs/fpack/fpack.c ../libs/fpack/fpackutil.c
     ../libs/dsp/align.c ../libs/dsp/buffer.c ../libs/dsp/convert.c ../libs/dsp/convolution.c
     ../libs/dsp/feature.c ../libs/dsp/fft.c ../libs/dsp/file.c ../libs/dsp/filters.c ../libs/dsp/fits.c ../libs/dsp/signals.c ../libs/dsp/stats.c ../libs/dsp/stream.c
     )
    set(LIBDEPS_CCD cfitsio jpeg fftw3)
else()
    set(INDILIB_CCD_SRC )
    set(LIBDEPS_CCD)
endif()

set(INDILIB_SRC 
 ../libs/indibase/indirotatorinterface.cpp
 ../libs/indibase/defaultdevice.cpp
 ../libs/indibase/fitskeyword.cpp
 ../libs/indibase/indicontroller.cpp
 ../libs/indibase/indidome.cpp
 ../libs/indibase/indidustcapinterface.cpp
 ../libs/indibase/indifilterinterface.cpp
 ../libs/indibase/indifilterwheel.cpp
 ../libs/indibase/indifocuser.cpp
 ../libs/indibase/indifocuserinterface.cpp
 ../libs/indibase/indigps.cpp
 ../libs/indibase/indigpsinterface.cpp
 ../libs/indibase/indiguiderinterface.cpp
 ../libs/indibase/indiinputinterface.cpp
 ../libs/indibase/indilightboxinterface.cpp
 ../libs/indibase/indilogger.cpp
 ../libs/indibase/indioutputinterface.cpp
 ../libs/indibase/indirotator.cpp
 ../libs/indibase/inditelescope.cpp
 ../libs/indidevice/basedevice.cpp
 ../libs/indidevice/indibase.cpp
 ../libs/indidevice/indistandardproperty.cpp
 ../libs/indidevice/parentdevice.cpp
 ../libs/indidevice/watchdeviceproperty.cpp
 ../libs/indicore/indiutility.cpp
 ../libs/indicore/libastro.cpp
 ../libs/indicore/lilxml.cpp
 ../libs/indicore/sharedblob_parse.cpp
 ../libs/indidevice/property/indiproperties.cpp
 ../libs/indidevice/property/indipropertybasic.cpp
 ../libs/indidevice/property/indipropertyblob.cpp
 ../libs/indidevice/property/indiproperty.cpp
 ../libs/indidevice/property/indipropertylight.cpp
 ../libs/indidevice/property/indipropertynumber.cpp
 ../libs/indidevice/property/indipropertyswitch.cpp
 ../libs/indidevice/property/indipropertytext.cpp
 ../libs/indidevice/property/indipropertyview.cpp
 ../libs/indibase/timer/indielapsedtimer.cpp
 ../libs/indibase/timer/inditimer.cpp
 ../libs/indibase/connectionplugins/connectioninterface.cpp
 ../libs/indibase/connectionplugins/connectionserial.cpp
 ../libs/indibase/connectionplugins/connectiontcp.cpp
 ../libs/indibase/connectionplugins/ttybase.cpp
 ../libs/indiabstractclient/abstractbaseclient.cpp
 ../libs/indibase/pid/pid.cpp
 ../libs/indiclient/baseclient.cpp
 ../libs/sockets/tcpsocket_unix.cpp
 ../libs/sockets/tcpsocket.cpp
 ../libs/alignment/AlignmentSubsystemForDrivers.cpp
 ../libs/alignment/BasicMathPlugin.cpp
 ../libs/alignment/BuiltInMathPlugin.cpp
 ../libs/alignment/ConvexHull.cpp
 ../libs/alignment/DriverCommon.cpp
 ../libs/alignment/InMemoryDatabase.cpp
 ../libs/alignment/MapPropertiesToInMemoryDatabase.cpp
 ../libs/alignment/MathPlugin.cpp
 ../libs/alignment/MathPluginManagement.cpp
 ../libs/alignment/TelescopeDirectionVectorSupportFunctions.cpp
 ../libs/alignment/Common.cpp
 ../libs/indicore/base64.c ../libs/indicore/indicom.c ../libs/indicore/indidevapi.c ../libs/indicore/indiuserio.c ../libs/indicore/sharedblob.c ../libs/indicore/shm_open_anon.c ../libs/indicore/userio.c
 ../libs/eventloop/eventloop.c
 ../libs/indicore/base64.c ../libs/indicore/indicom.c ../libs/indicore/indidevapi.c ../libs/indicore/indiuserio.c ../libs/indicore/sharedblob.c ../libs/indicore/shm_open_anon.c ../libs/indicore/userio.c
 ../libs/indibase/indidriver.c ../libs/indibase/indidriverio.c ../libs/indibase/indidrivermain.c 
)

#set(LIBDEPS nova z gsl gslcblas)
set(LIBDEPS ${NOVA_LIB} z ${GSL_LIB})

set(USE_HIDDEN FALSE)


if(USE_HIDDEN) 
    set(INDI_DRV indi_driver)
    add_library(indi_driver STATIC ${INDILIB_SRC} ${INDILIB_CCD_SRC})
    target_link_libraries(indi_driver ${LIBDEPS} ${LIBDEPS_CCD})

    set_target_properties(indi_driver PROPERTIES 
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden)
    add_definitions(-DUSE_HIDDEN)
else()    
    set(INDI_DRV indi_unified)
endif()

add_library(indi_unified SHARED ${INDILIB_SRC} ${INDILIB_CCD_SRC})
target_link_libraries(indi_unified ${LIBDEPS} ${LIBDEPS_CCD})

add_library(indi_unified_client_server SHARED ../libs/indiclient/baseclient.cpp ../indiserver/indiserver.cpp)
target_link_libraries(indi_unified_client_server indi_unified ${EV_LIB})

if(ENABLE_CCD)
    add_library(indidriver_ccd_simulator SHARED ../drivers/ccd/ccd_simulator.cpp)
    if(USE_HIDDEN) 
        set_target_properties(indidriver_ccd_simulator PROPERTIES CXX_VISIBILITY_PRESET hidden)
    endif()    
    target_link_libraries(indidriver_ccd_simulator ${INDI_DRV})
endif()    

add_library(indidriver_telescope_skywatcherAltAz SHARED ../drivers/telescope/skywatcherAPIMount.cpp ../drivers/telescope/skywatcherAPI.cpp)
if(USE_HIDDEN) 
    set_target_properties(indidriver_telescope_skywatcherAltAz PROPERTIES CXX_VISIBILITY_PRESET hidden)
endif()
target_link_libraries(indidriver_telescope_skywatcherAltAz ${INDI_DRV})

add_library(indidriver_telescope_simulator SHARED ../drivers/telescope/telescope_simulator.cpp ../drivers/telescope/scopesim_helper.cpp)
target_link_libraries(indidriver_telescope_simulator ${INDI_DRV})


add_library(indi_SVD_MathPlugin SHARED ../libs/alignment/SVDMathPlugin.cpp)
target_link_libraries(indi_SVD_MathPlugin indi_unified)

add_library(indi_Nearest_MathPlugin SHARED ../libs/alignment/NearestMathPlugin.cpp)
target_link_libraries(indi_Nearest_MathPlugin indi_unified)

add_executable(test_mount test_mount.cpp)
target_link_libraries(test_mount indi_unified_client_server indi_unified)

file(GLOB INST_HDR 
    ../libs/indiclient/*.h
    ../libs/indiabstractclient/*.h
    ../libs/indidevice/*.h
    ../libs/indicore/*.h
    ../libs/*.h
    ../libs/indidevice/property/*.h)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indiapi.h
    ${INST_HDR}
    DESTINATION
    include/libindi)

install(TARGETS 
    indi_unified
    indi_unified_client_server
    indidriver_telescope_simulator
    indidriver_telescope_skywatcherAltAz
    indi_SVD_MathPlugin indi_Nearest_MathPlugin
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/libindi)

if(ENABLE_CCD) 
    install(TARGETS 
        indidriver_ccd_simulator
        LIBRARY DESTINATION lib)
endif()

