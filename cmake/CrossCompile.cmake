SET(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR "armv7-a")

SET(CMAKE_SYSROOT "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi")
#SET(CMAKE_SYSROOT "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots")

SET(TARGETSYSROOT "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi")
SET(TOOLCHAIN_PATH "/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7ve -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7 -rdynamic")
SET(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7ve -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a7 -rdynamic")
SET(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++")

SET(CMAKER_AR "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-ar")
SET(CMAKER_LINKER "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-ld")
SET(CMAKER_NM "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-nm")
SET(CMAKER_OBJDUMP "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-objdump")
SET(CMAKER_RANLIB "${TOOLCHAIN_PATH}/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-ranlib")

#SET(OPENSSL_LIBRARIES ${TARGETSYSROOT}/usr/lib)
#SET(OPENSSL_INCLUDE_DIR ${TARGETSYSROOT}/usr/include/openssl)

#SET(CMAKE_FIND_ROOT_PATH 
#	${TOOLCHAIN_PATH}
#	${TOOLCHAIN_PATH}/usr/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/5.3.0
#	${TOOLCHAIN_PATH}/usr/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/5.3.0/plugin/include
#	${TARGETSYSROOT}/lib 
#	${TARGETSYSROOT}/usr/lib/arm-poky-linux-gnueabi/5.3.0
#	${TARGETSYSROOT}/usr/lib
#	${TARGETSYSROOT}/usr/include)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

