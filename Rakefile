
require "rake/extensiontask"
Rake::ExtensionTask.new "score_index" do |ext|
  ext.lib_dir = "lib/score_index"
end

require 'rake/testtask'
desc 'Run all unit tests'
Rake::TestTask.new(:test) do |test|
  test.pattern = 'test/test_*.rb'
  test.verbose = false
  test.warning = false
end

task default: %i[compile test]
