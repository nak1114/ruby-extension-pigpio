root=File.expand_path('../../', __FILE__)
if ARGV[0]=="c" || ARGV[0]=="clean"
  system"docker container prune"
  system"docker image prune"
  system"docker image prune"
  exit 0
end
if ARGV[0]=="v" || ARGV[0]=="volume"
  Dir.chdir(root) do
    system %q(docker build -t pigpio -f ./bin/Dockerfile .)
    docker_run=%(docker run --rm -it -v "#{root}":/myapp -w /myapp #{ENV['DOCKER_OPT']} pigpio)
    system("#{docker_run} cp -pf /tmp/Gemfile.lock /myapp")
    system"#{docker_run} /bin/bash"
  end
  exit 0
end
Dir.chdir(root) do
  system %q(docker build -t pigpio -f ./bin/Dockerfile .)
  docker_run=%(docker run --rm -it -w /myapp #{ENV['DOCKER_OPT']} pigpio)
  system("#{docker_run} cp -pf /tmp/Gemfile.lock /myapp")
  system"#{docker_run} /bin/bash"
end
__END__