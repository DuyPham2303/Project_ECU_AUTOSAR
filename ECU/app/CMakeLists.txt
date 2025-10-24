# app/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(ECU C)

# ==== Paths ====
set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

# platform
set(PLATFORM_COMMON_DIR "${ROOT_DIR}/platform/common")
set(PLATFORM_CSVLIB_DIR "${ROOT_DIR}/platform/csvlib")

# bsw/services
set(OS_DIR            "${ROOT_DIR}/bsw/services/os")
set(OS_ARCH_POSIX_DIR "${OS_DIR}/arch/posix")
set(ECUM_DIR          "${ROOT_DIR}/bsw/services/ecum")

# bsw/communication
set(COM_DIR   "${ROOT_DIR}/bsw/communication/com")
set(CANIF_DIR "${ROOT_DIR}/bsw/communication/canif")
set(PDUR_DIR  "${ROOT_DIR}/bsw/communication/pdur")

# bsw/ecua
set(ECUA_DIR "${ROOT_DIR}/bsw/ecua/iohwab")

# bsw/mcal
set(ADC_DIR "${ROOT_DIR}/bsw/mcal/adc")
set(CAN_DIR "${ROOT_DIR}/bsw/mcal/can")
set(DIO_DIR "${ROOT_DIR}/bsw/mcal/dio")
set(PWM_DIR "${ROOT_DIR}/bsw/mcal/pwm")

# RTE
set(RTE_SWC_IF_DIR "${ROOT_DIR}/rte/swc_if")
set(RTE_CORE_DIR   "${ROOT_DIR}/rte/core")
set(RTE_SRC_DIR    "${RTE_CORE_DIR}/src")

# SWCs
set(SWC_ACTUATOR_IF_DIR  "${ROOT_DIR}/swc/Swc_ActuatorIf")
set(SWC_MOTOR_CTRL_DIR   "${ROOT_DIR}/swc/Swc_MotorCtrl")
set(SWC_MOTOR_FB_ACQ_DIR "${ROOT_DIR}/swc/Swc_MotorFbAcq")
set(SWC_VCU_CMD_IN_DIR   "${ROOT_DIR}/swc/Swc_VcuCmdIn")
set(SWC_ECU_STATE_DIR   "${ROOT_DIR}/swc/Swc_EcuState")

# ==== C standard & warnings ====
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==== Include paths ====
include_directories(
  "${CMAKE_CURRENT_LIST_DIR}"   # app/
  "${PLATFORM_COMMON_DIR}"
  "${PLATFORM_CSVLIB_DIR}"
  "${OS_DIR}/inc"
  "${ECUM_DIR}"
  "${COM_DIR}"
  "${CANIF_DIR}"
  "${PDUR_DIR}"
  "${ECUA_DIR}"
  "${ADC_DIR}"
  "${CAN_DIR}"
  "${DIO_DIR}"
  "${PWM_DIR}"
  "${RTE_CORE_DIR}/inc"
  "${RTE_SWC_IF_DIR}"
  "${SWC_ACTUATOR_IF_DIR}"
  "${SWC_MOTOR_CTRL_DIR}"
  "${SWC_MOTOR_FB_ACQ_DIR}"
  "${SWC_VCU_CMD_IN_DIR}"
  "${SWC_ECU_STATE_DIR}"
  
)

# ==== Sources (tự động quét file .c) ====
file(GLOB APP_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/hooks/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/tasks/*.c"
)

file(GLOB BSW_SOURCES
  "${OS_ARCH_POSIX_DIR}/*.c"
  "${ECUM_DIR}/*.c"
  "${COM_DIR}/*.c"
  "${CANIF_DIR}/*.c"
  "${PDUR_DIR}/*.c"
  "${ECUA_DIR}/*.c"
  "${ADC_DIR}/*.c"
  "${CAN_DIR}/*.c"
  "${DIO_DIR}/*.c"
  "${PWM_DIR}/*.c"
)

file(GLOB PLATFORM_SOURCES
  "${PLATFORM_CSVLIB_DIR}/*.c"
)

file(GLOB RTE_SOURCES
  "${RTE_SRC_DIR}/*.c"
)

file(GLOB SWC_SOURCES
  "${SWC_ACTUATOR_IF_DIR}/*.c"
  "${SWC_MOTOR_CTRL_DIR}/*.c"
  "${SWC_MOTOR_FB_ACQ_DIR}/*.c"
  "${SWC_VCU_CMD_IN_DIR}/*.c"
  "${SWC_ECU_STATE_DIR}/*.c"
)

# ==== Executable ====
add_executable(ecu_sim
  ${APP_SOURCES}
  ${BSW_SOURCES}
  ${PLATFORM_SOURCES}
  ${RTE_SOURCES}
  ${SWC_SOURCES}
)

# ==== POSIX/pthread ====
find_package(Threads REQUIRED)
target_link_libraries(ecu_sim PRIVATE Threads::Threads)

# ==== Compile definitions ====
target_compile_definitions(ecu_sim PRIVATE OS_POSIX=1)

