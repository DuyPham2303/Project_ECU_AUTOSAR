# app/CMakeLists.txt  —  BẢN TỐI GIẢN
cmake_minimum_required(VERSION 3.20)
project(ECU)

# ==== Paths ====
set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
# APP DIR
set(PLATFORM_COMMON_DIR "${ROOT_DIR}/platform/common")
set(PLATFORM_CSVLIB_DIR "${ROOT_DIR}/platform/csvlib")         # csvlib.h , csvlib.c
# bsw/services
set(OS_DIR            "${ROOT_DIR}/bsw/services/os")           # .../inc: Os.h
set(OS_ARCH_POSIX_DIR "${OS_DIR}/arch/posix")                  # Os_Posix.c
set(ECUM_DIR          "${ROOT_DIR}/bsw/services/ecum")         # .../inc: EcuM.h, EcuM.c
# bsw/communication
set(COM_DIR           "${ROOT_DIR}/bsw/communication/com")        
set(CANIF_DIR         "${ROOT_DIR}/bsw/communication/canif")
set(PDUR_DIR          "${ROOT_DIR}/bsw/communication/pdur")     
# bsw/ecua
set(ECUA_DIR          "${ROOT_DIR}/bsw/ecua/iohwab")     
# bsw/mcal
set(ADC_DIR          "${ROOT_DIR}/bsw/mcal/adc")    
set(CAN_DIR          "${ROOT_DIR}/bsw/mcal/can")    
set(DIO_DIR          "${ROOT_DIR}/bsw/mcal/dio")    
set(PWM_DIR          "${ROOT_DIR}/bsw/mcal/pwm")        
# RTE
set(RTE_SWC_IF_DIR       "${ROOT_DIR}/rte/swc_if")  
set(RTE_CORE_DIR         "${ROOT_DIR}/rte/core")
set(RTE_SRC_DIR         "${RTE_CORE_DIR}/src")
# swc
set(SWC_ACTUATOR_IF_DIR  "${ROOT_DIR}/Swc_ActuatorIf")
set(SWC_MOTOR_CTRL_DIR   "${ROOT_DIR}/Swc_MotorCtrl")
set(SWC_MOTOR_FB_ACQ_DIR "${ROOT_DIR}/Swc_MotorFbAcq")
set(SWC_VCU_CMD_IN_DIR   "${ROOT_DIR}/Swc_VcuCmdIn")

# ==== Kiểm tra file bắt buộc (giúp báo lỗi rõ ràng) ====
if(NOT EXISTS "${OS_ARCH_POSIX_DIR}/Os_Posix.c")
  message(FATAL_ERROR "Missing ${OS_ARCH_POSIX_DIR}/Os_Posix.c")
endif()
if(NOT EXISTS "${ECUM_DIR}/EcuM.c")
  message(FATAL_ERROR "Missing ${ECUM_DIR}/EcuM.c")
endif()
if(NOT EXISTS "${ECUM_DIR}/EcuM.h")
  message(FATAL_ERROR "Missing ${ECUM_DIR}/inc/EcuM.h")
endif()

# ==== C standard & warnings ====
set(CMAKE_BUILD_TYPE Debug)             # debug build
set(CMAKE_C_FLAGS_DEBUG "-g -O0")       # flag debug
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
  # Bỏ cảnh báo '/*' trong block comment nếu cần:
  # add_compile_options(-Wno-comment)
endif()

# ==== Include paths (TOÀN CỤC cho đơn giản) ====
include_directories(
  "${CMAKE_CURRENT_LIST_DIR}"       # app/
  "${PLATFORM_COMMON_DIR}"          # platform/common
  "${OS_DIR}/inc"                   # Os.h, Os_Types.h, Os_Cfg.h
  "${ECUM_DIR}"                     # EcuM.h
  "${PLATFORM_CSVLIB_DIR}"          # csvlib.h
  "${COM_DIR}"                      # Com.h
  "${CANIF_DIR}"                    # Canif.h
  "${PDUR_DIR}"                     # PduR.h
  "${ECUA_DIR}"                     # IoHwAb.h
  "${ADC_DIR}"                      # Adc.h    
  "${CAN_DIR}"                      # Can.h
  "$(DIO_DIR}"                      # Dio.h
  "${PWM_DIR}"                      # Pwm.h
  "${RTE_CORE_DIR}/inc"             # Rte.h , Rte_batch.h
  "${SWC_ACTUATOR_IF_DIR}"
  "${SWC_MOTOR_CTRL_DIR}"
  "${SWC_MOTOR_FB_ACQ_DIR}"
  "${SWC_VCU_CMD_IN_DIR}"
)       

# ==== Sources ====
set(APP_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/main.c"
  "${CMAKE_CURRENT_LIST_DIR}/hooks/Os_Hook.c"
  "${CMAKE_CURRENT_LIST_DIR}/tasks/InitTask.c"
  "${CMAKE_CURRENT_LIST_DIR}/tasks/Task_10ms.c"
  "${CMAKE_CURRENT_LIST_DIR}/tasks/Task_100ms.c"
  "${CMAKE_CURRENT_LIST_DIR}/tasks/Task_Com.c"
)

set(BSW_SOURCES
  "${OS_ARCH_POSIX_DIR}/Os_Posix.c"
  "${ECUM_DIR}/EcuM.c"
  "${COM_DIR}/Com.c"
  "${CANIF_DIR}/Canif.c"
  "${PDUR_DIR}/PduR.c"
  "${ECUA_DIR}/IoHwAb.c"
  "${ADC_DIR}/Adc.c"
  "${CAN_DIR}/Can.c"
  "${DIO_DIR}/Dio.c"
  "${PWM_DIR}/Pwm.c"
)

set(PLATFORM_SOURCES
    "${PLATFORM_CSVLIB_DIR}/csv_io.C"
)

set(RTE_SOURCES
   "${RTE_SRC_DIR}/rte.c"
   "${RTE_SRC_DIR}/Rte_batch.c"
)

set(SWC_SOURCES
    "${SWC_ACTUATOR_IF_DIR}/Swc_ActuatorIf.c"
    "${SWC_MOTOR_CTRL_DIR}/Swc_MotorCtrl.c"
    "${SWC_MOTOR_FB_ACQ_DIR}/Swc_MotorFbAcq.c"
    "${SWC_VCU_CMD_IN_DIR}/Swc_VcuCmdIn.c"
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

# (tuỳ chọn) define cho POSIX
target_compile_definitions(ecu_sim PRIVATE OS_POSIX=1)

# (tuỳ chọn) xuất binary ra bin/
# set_target_properties(ecu_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${ROOT_DIR}/bin")
