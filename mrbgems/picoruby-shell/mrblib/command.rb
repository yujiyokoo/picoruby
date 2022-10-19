class Shell
  class Command
    def initialize
    end

    attr_accessor :feed

    BUILTIN = %w(
      irb
      ruby
      alias
      cd
      echo
      export
      exec
      exit
      fc
      help
      history
      kill
      pwd
      type
      unset
    )

    def command_type(name)
      if BUILTIN.include?(name)
        :builtin
      elsif find_executable(name)
        :executable
      else
        nil
      end
    end

    def find_executable(name)
      false
    end

    def exec(params)
      args = params[1, params.length - 1]
      case params[0]
      when "irb"
        _irb(*args)
      when "echo"
        _echo(*args)
      when "type"
        _type(*args)
      when "vim"
        Vim.new(args[0]).start
      else
        print "#{params[0]}: command not found"
      end
      print @feed
    end

    #
    # Builtin commands
    #

    def _irb(*params)
      Shell.new.start(:irb)
    end

    def _type(*params)
      params.each_with_index do |name, index|
        print @feed if 0 < index
        if BUILTIN.include?(name)
          print "#{name} is a shell builtin"
        elsif path = find_executable(name)
          print "#{name} is #{path}"
        else
          print "type: #{name}: not found"
        end
      end
    end

    def _echo(*params)
      params.each_with_index do |param, index|
        print " " if 0 < index
        print param
      end
    end

  end
end
