# Psy-Q exclusion guard (owner directive 2026-09-28; docs/ai_context/PSYQ_PORT_REPLACEMENT.md).
#
# include(cmake/psyq_guard.cmake) AFTER the parasite-eve-port and
# pe-native-tests targets exist.  Registers the ctest `psyq-port-guard`:
#   * default: relative to pc_port/psyq_guard_allowlist.txt -- fails on any NEW
#     Psy-Q function/table/include and on any STALE allowlist entry (the list
#     must shrink as replacements land);
#   * -DPE_PSYQ_GUARD_STRICT=ON: ignores the allowlist (goal state: no Psy-Q
#     code, no Psy-Q tables, no PsyCross/Psy-Q/host/backend headers in game TUs).
# The table check reads the libgte table bytes from the user's own retail EXE
# (build/extracted/disc1/SLUS_006.62) and is skipped with a warning without it.
option(PE_PSYQ_GUARD_STRICT "psyq-port-guard ignores the allowlist" OFF)
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND AND TARGET parasite-eve-port)
    set(_pe_guard_root "${CMAKE_CURRENT_SOURCE_DIR}/..")
    set(_pe_guard_args
        "${_pe_guard_root}/tools/analysis/psyq_port_guard.py"
        --tree "${CMAKE_CURRENT_SOURCE_DIR}"
        --binary "$<TARGET_FILE:parasite-eve-port>")
    if(TARGET pe-native-tests)
        list(APPEND _pe_guard_args --binary "$<TARGET_FILE:pe-native-tests>")
    endif()
    if(PE_PSYQ_GUARD_STRICT)
        list(APPEND _pe_guard_args --strict)
    endif()
    add_test(NAME psyq-port-guard COMMAND ${Python3_EXECUTABLE} ${_pe_guard_args})
    set_tests_properties(psyq-port-guard PROPERTIES TIMEOUT 600 LABELS "policy")
    add_test(NAME psyq-port-guard-selftest
             COMMAND ${Python3_EXECUTABLE} -m unittest -q test_psyq_port_guard
             WORKING_DIRECTORY "${_pe_guard_root}/tools/analysis")
    set_tests_properties(psyq-port-guard-selftest PROPERTIES TIMEOUT 120 LABELS "policy")
endif()
