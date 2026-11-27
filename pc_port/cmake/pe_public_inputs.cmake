# Optional-input helpers: one CMakeLists.txt for the development tree and for the
# published source tree.
#
# The published tree cannot carry some inputs (retail-derived test vectors,
# dev-only tests and headers).  Instead of two diverging CMakeLists.txt files,
# every target that needs such an input asks this file whether the input is
# present.  A complete (development) tree answers "yes" for everything, so its
# targets, tests and build are unchanged.
#
#   pe_test_inputs_present(<out-var> <target-name> <source>...)
#       Sets <out-var> to TRUE when every quoted #include reachable from the
#       given test sources (transitively, through the same directories the
#       targets compile with) exists.  Otherwise sets it to FALSE and records
#       the target and the missing files for pe_report_skipped_tests().
#       Sources are relative to pc_port/.  A target whose answer is FALSE is
#       not created; nothing compiles a reduced binary under the same name.
#
#   pe_test_data_present(<out-var> <target-name> <file>...)
#       Same answer for plain data files a test reads (not #include closures).
#
#   pe_note_skipped_tests(<target>...)
#       Records targets that are skipped for another reason (e.g. registered
#       together with a target that could not be built).
#
#   pe_report_skipped_tests()
#       One STATUS message naming what is missing and which targets were
#       skipped, plus ${CMAKE_BINARY_DIR}/pe-tests-skipped.txt (read by
#       scripts/build_pc_port.sh).  Silent when nothing was skipped.
#
# Include once, before the first test target.

set(PE_INPUT_SEARCH_DIRS
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/platform"
    "${CMAKE_SOURCE_DIR}/bootstrap"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/tests"
    "${CMAKE_SOURCE_DIR}/tests/winmm_fixture")
set_property(GLOBAL PROPERTY PE_SKIPPED_TEST_TARGETS "")
set_property(GLOBAL PROPERTY PE_MISSING_TEST_INPUTS "")

function(pe_test_inputs_present out target)
    set(queue "")
    foreach(src IN LISTS ARGN)
        list(APPEND queue "${CMAKE_SOURCE_DIR}/${src}")
    endforeach()
    set(seen "")
    set(missing "")
    while(queue)
        list(POP_FRONT queue file)
        if(file IN_LIST seen)
            continue()
        endif()
        list(APPEND seen "${file}")
        if(NOT EXISTS "${file}")
            file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}" "${file}")
            list(APPEND missing "${rel}")
            continue()
        endif()
        get_filename_component(file_dir "${file}" DIRECTORY)
        file(STRINGS "${file}" include_lines REGEX "^[ \t]*#[ \t]*include[ \t]+\"[^\"]+\"")
        foreach(line IN LISTS include_lines)
            string(REGEX REPLACE "^[ \t]*#[ \t]*include[ \t]+\"([^\"]+)\".*$" "\\1" name "${line}")
            set(found "")
            foreach(dir IN ITEMS "${file_dir}" ${PE_INPUT_SEARCH_DIRS})
                if(EXISTS "${dir}/${name}")
                    set(found "${dir}/${name}")
                    break()
                endif()
            endforeach()
            if(found)
                list(APPEND queue "${found}")
            else()
                string(REGEX REPLACE "^(\\.\\./)+" "" name "${name}")
                list(APPEND missing "${name}")
            endif()
        endforeach()
    endwhile()
    if(missing)
        list(REMOVE_DUPLICATES missing)
        set(${out} FALSE PARENT_SCOPE)
        set_property(GLOBAL APPEND PROPERTY PE_SKIPPED_TEST_TARGETS "${target}")
        set_property(GLOBAL APPEND PROPERTY PE_MISSING_TEST_INPUTS ${missing})
    else()
        set(${out} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(pe_test_data_present out target)
    set(missing "")
    foreach(file IN LISTS ARGN)
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${file}")
            list(APPEND missing "${file}")
        endif()
    endforeach()
    if(missing)
        set(${out} FALSE PARENT_SCOPE)
        set_property(GLOBAL APPEND PROPERTY PE_SKIPPED_TEST_TARGETS "${target}")
        set_property(GLOBAL APPEND PROPERTY PE_MISSING_TEST_INPUTS ${missing})
    else()
        set(${out} TRUE PARENT_SCOPE)
    endif()
endfunction()

function(pe_note_skipped_tests)
    set_property(GLOBAL APPEND PROPERTY PE_SKIPPED_TEST_TARGETS ${ARGN})
endfunction()

function(pe_report_skipped_tests)
    get_property(skipped GLOBAL PROPERTY PE_SKIPPED_TEST_TARGETS)
    get_property(missing GLOBAL PROPERTY PE_MISSING_TEST_INPUTS)
    set(report "${CMAKE_BINARY_DIR}/pe-tests-skipped.txt")
    if(NOT skipped)
        file(REMOVE "${report}")
        return()
    endif()
    list(REMOVE_DUPLICATES missing)
    set(retail "")
    set(other "")
    foreach(name IN LISTS missing)
        if(name MATCHES "(^|/)retail_")
            list(APPEND retail "${name}")
        else()
            list(APPEND other "${name}")
        endif()
    endforeach()
    set(why "")
    if(retail)
        list(LENGTH retail n)
        string(APPEND why "retail-derived test vectors absent (pc_port/tests/retail_*_cases.h, ${n} files missing); ")
    endif()
    if(other)
        list(JOIN other ", " other_text)
        string(APPEND why "dev-only test inputs absent (${other_text}); ")
    endif()
    list(JOIN skipped " " skipped_text)
    set(msg "${why}these test targets are not built: ${skipped_text}")
    message(STATUS "pe: ${msg}")
    file(WRITE "${report}" "${msg}\n")
endfunction()
