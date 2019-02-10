require "bundler/gem_tasks"
require "rspec/core/rake_task"
require "rake/extensiontask"
require "rdoc/task"

Rake::ExtensionTask.new "pigpio" do |ext|
  ext.lib_dir = "lib/pigpio"
end

RSpec::Core::RakeTask.new(:spec)

task :default => :spec


RDOC_FILES = FileList["ext/pigpio/pigpio.c","lib/**/*.rb"]

Rake::RDocTask.new do |rd|
  rd.main = "pigpio.c"
  rd.rdoc_dir = "doc"
  rd.rdoc_files.include(RDOC_FILES)
end
