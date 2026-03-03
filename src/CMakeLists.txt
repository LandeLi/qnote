# QNote 源码构建配置

# 收集源文件
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
