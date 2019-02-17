require "bundler/setup"
require "pigpio"

RSpec.configure do |config|
  # Enable flags like --only-failures and --next-failure
  config.example_status_persistence_file_path = ".rspec_status"

  # Disable RSpec exposing methods globally on `Module` and `main`
  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end

  config.filter_run :focus => true
  config.run_all_when_everything_filtered = true
end

SeqFile=File.expand_path('../ext/values.txt', __FILE__)
ArgsFile=File.expand_path('../ext/args.txt', __FILE__)
def write_seq(str)
  File.write(SeqFile,str.chomp)
end
def read_args()
  ret=nil
  if File.exist? ArgsFile
    ret=File.read(ArgsFile)
    FileUtils.rm(ArgsFile)
  end
  ret
end
