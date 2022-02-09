module MRuby
  class Build
    def disable_libmruby_core
      @enable_libmruby_core = false
    end
  end
end

MRuby::Build.new do |conf|
  conf.toolchain

  disable_presym
  disable_libmruby_core
  disable_libmruby
  conf.mrbcfile = "#{conf.build_dir}/bin/picorbc"

  conf.gem core: 'mruby-mrubyc'
  conf.gem core: 'mruby-pico-compiler'
  conf.gem core: 'mruby-bin-picorbc'
  conf.gem core: 'mruby-bin-picoruby'
  conf.gem core: 'mruby-bin-picoirb'

  conf.cc.defines << "DISABLE_MRUBY"
  conf.cc.defines << "NDEBUG" unless ENV["PICORUBY_DEBUG"]
  conf.cc.defines << "MRBC_ALLOC_LIBC"
  conf.cc.defines << "REGEX_USE_ALLOC_LIBC"
  conf.cc.defines << "MRBC_USE_HAL_POSIX"
end
