# QNote 源码构建配置

# 自动收集源文件
set(CORE_SOURCES
    core/Logger.cpp
    core/Settings.cpp
    core/Utils.cpp
)

set(CORE_HEADERS
    core/Logger.h
    core/Settings.h
    core/Utils.h
)

set(APP_SOURCES
    app/Application.cpp
    main.cpp
)

set(APP_HEADERS
    app/Application.h
)

# 数据层 (M01) - 可选
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/models)
    file(GLOB_RECURSE MODEL_SOURCES models/*.cpp)
    file(GLOB_RECURSE MODEL_HEADERS models/*.h)
    list(APPEND CORE_SOURCES ${MODEL_SOURCES})
    list(APPEND CORE_HEADERS ${MODEL_HEADERS})
    add_definitions(-DENABLE_DATA_LAYER)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/storage)
    file(GLOB_RECURSE STORAGE_SOURCES storage/*.cpp)
    file(GLOB_RECURSE STORAGE_HEADERS storage/*.h)
    list(APPEND CORE_SOURCES ${STORAGE_SOURCES})
    list(APPEND CORE_HEADERS ${STORAGE_HEADERS})
endif()

# Markdown服务 (M02) - 可选
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/services/MarkdownService.cpp)
    file(GLOB_RECURSE SERVICE_SOURCES services/*.cpp)
    file(GLOB_RECURSE SERVICE_HEADERS services/*.h)
    list(APPEND CORE_SOURCES ${SERVICE_SOURCES})
    list(APPEND CORE_HEADERS ${SERVICE_HEADERS})
    add_definitions(-DENABLE_MARKDOWN_SERVICE)
endif()

# UI模块 (M04, M05) - 可选
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
    file(GLOB_RECURSE UI_SOURCES ui/*.cpp)
    file(GLOB_RECURSE UI_HEADERS ui/*.h)
    list(APPEND CORE_SOURCES ${UI_SOURCES})
    list(APPEND CORE_HEADERS ${UI_HEADERS})
    add_definitions(-DENABLE_UI)
endif()

# 平台模块 (M06) - 可选
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/platform)
    file(GLOB_RECURSE PLATFORM_SOURCES platform/*.cpp)
    file(GLOB_RECURSE PLATFORM_HEADERS platform/*.h)
    list(APPEND CORE_SOURCES ${PLATFORM_SOURCES})
    list(APPEND CORE_HEADERS ${PLATFORM_HEADERS})
    add_definitions(-DENABLE_PLATFORM)
endif()

# 业务层 (M03) - 可选
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/managers)
    file(GLOB_RECURSE MANAGER_SOURCES managers/*.cpp)
    file(GLOB_RECURSE MANAGER_HEADERS managers/*.h)
    list(APPEND CORE_SOURCES ${MANAGER_SOURCES})
    list(APPEND CORE_HEADERS ${MANAGER_HEADERS})
    add_definitions(-DENABLE_MANAGERS)
endif()

# 资源文件
set(RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/qnote.qrc
)

# 可执行文件
add_executable(qnote
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${RESOURCES}
)

# 链接 Qt 库
target_link_libraries(qnote PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# 包含目录
target_include_directories(qnote PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/app
)

# 第三方库
if(EXISTS ${CMAKE_SOURCE_DIR}/third-party)
    target_include_directories(qnote PRIVATE
        ${CMAKE_SOURCE_DIR}/third-party
    )
endif()

# 打印模块状态
message(STATUS "=== QNote Modules ===")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/storage)
    message(STATUS "M01 Data Layer: ENABLED")
else()
    message(STATUS "M01 Data Layer: disabled")
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/services/MarkdownService.cpp)
    message(STATUS "M02 Markdown Service: ENABLED")
else()
    message(STATUS "M02 Markdown Service: disabled")
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ui)
    message(STATUS "UI Modules: ENABLED")
else()
    message(STATUS "UI Modules: disabled")
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/platform)
    message(STATUS "M06 Platform: ENABLED")
else()
    message(STATUS "M06 Platform: disabled")
endif()
message(STATUS "=====================")
