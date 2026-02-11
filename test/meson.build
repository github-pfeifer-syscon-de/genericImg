
public_headers = include_directories('../include')

unic_test = executable('unic_test'
    , 'unic_test.cpp'
    , dependencies: [glibmm2_deps, gtkmm3_deps, thread_deps, jsonglib1_deps, fontconfig_deps]
    , include_directories : public_headers
    , link_with : project_target)
test('unic_test', unic_test)

log_test = executable('log_test'
    , 'log_test.cpp'
    , dependencies: [glibmm2_deps, sysdlog_deps]
    , include_directories : public_headers
    , link_with : project_target)
test('log_test', log_test)

worker_test = executable('worker_test'
    , 'WorkerTest.cpp'
    , dependencies: [glibmm2_deps, gtkmm3_deps ]
    , include_directories : public_headers
    , link_with : project_target)
test('worker_test', worker_test)

json_test = executable('json_test'
    , 'JsonTest.cpp'
    , dependencies: [glibmm2_deps, jsonglib1_deps ]
    , include_directories : public_headers
    , link_with : project_target)
test('json_test', json_test)

util_test = executable('util_test'
    , 'util_test.cpp'
    , dependencies: [glibmm2_deps, gtkmm3_deps ]
    , include_directories : public_headers
    , link_with : project_target)
test('util_test', util_test)

pdf_test = executable('pdf_test'
    , 'pdf_test.cpp'
    , dependencies: [glibmm2_deps, gtkmm3_deps, fontconfig_deps ]
    , include_directories : public_headers
    , link_with : project_target)
test('pdf_test', pdf_test)

