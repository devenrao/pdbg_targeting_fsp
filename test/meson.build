gtest_dep = dependency('gtest', required: true)

test_sources = files(
  'test_target_service.C',  # add your test source files here
)

test_inc = include_directories(
  '..',
  '../targeting',
  '../targeting/common',
  '../targeting/adapters',
)

executable('targeting-tests',
  test_sources,
  include_directories: [test_inc, libfdt_inc],
  dependencies: [libfdt_dep, gtest_dep],
)

test('unit tests', 'targeting-tests')
