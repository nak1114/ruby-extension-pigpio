require "bundler/gem_tasks"
require "rspec/core/rake_task"
require "rake/extensiontask"
require "rdoc/task"

Rake::ExtensionTask.new "pigpio" do |ext|
  ext.lib_dir = "lib/pigpio"
end

RSpec::Core::RakeTask.new(:spec)

task default: :spec

RDOC_FILES = FileList["ext/pigpio/pigpio.c", "lib/**/*.rb"]

Rake::RDocTask.new do |rd|
  rd.main = "pigpio.c"
  rd.rdoc_dir = "docs"
  rd.rdoc_files.include(RDOC_FILES)
end

task :dummy do
  cd("spec/ext") do
    sh("ruby extconf.rb && make && \\cp -f libpigpiod_if2.so /usr/lib")
  end
end

task :dockerv do
  sh("ruby ./bin/docker.rb v")
end

task :reset do
  sh "git fetch origin"
  sh "git reset --hard origin/master"
end
