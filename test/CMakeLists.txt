function(add_libc8_test name)
    set(test_src "${CMAKE_CURRENT_SOURCE_DIR}/test_${name}.c")
    set(runner_src "${CMAKE_CURRENT_BINARY_DIR}/test_${name}_runner.c")

    add_custom_command(
        OUTPUT ${runner_src}
        COMMAND ruby ${CMAKE_SOURCE_DIR}/external/Unity/auto/generate_test_runner.rb ${test_src} ${runner_src}
        DEPENDS ${test_src} ${CMAKE_SOURCE_DIR}/external/Unity/auto/generate_test_runner.rb
        COMMENT "Generating ${runner_src}"
    )

    # Define the test executable
    add_executable(${name}_tests ${test_src} ${runner_src})
    target_link_libraries(${name}_tests c8 Unity gcov)

    # Register as a CTest test
    add_test(NAME ${name} COMMAND ${name}_tests)
endfunction()

add_libc8_test(debug)
add_libc8_test(decode)
add_libc8_test(encode)
add_libc8_test(instruction)
add_libc8_test(symbol)
add_libc8_test(util)
