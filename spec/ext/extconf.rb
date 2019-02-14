require 'mkmf'
$defs << "-DVFILENAME=\\\"#{File.expand_path('../values.txt', __FILE__)}\\\""
$defs << "-DAFILENAME=\\\"#{File.expand_path('../args.txt', __FILE__)}\\\""
create_makefile('libpigpiod_if2')
