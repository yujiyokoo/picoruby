class FAT

  AM_RDO = 0b000001 # Read only
  AM_HID = 0b000010 # Hidden
  AM_SYS = 0b000100 # System
  #AM_?? = 0b001000 # ???????
  AM_DIR = 0b010000 # Directory
  AM_ARC = 0b100000 # Archive

  class Stat
    def initialize(stat_hash)
      @stat_hash = stat_hash
    end

    def mode
      @stat_hash[:mode]
    end

    def mode_str
      (0 < mode & AM_ARC ? "A" : "-") <<
      (0 < mode & AM_DIR ? "D" : "-") <<
      (0 < mode & AM_SYS ? "S" : "-") <<
      (0 < mode & AM_HID ? "H" : "-") <<
      (0 < mode & AM_RDO ? "R" : "-")
    end

    def datetime
      @stat_hash[:datetime]
    end

    def size
      @stat_hash[:size]
    end

    def directory?
      0 < mode & AM_DIR
    end
  end

  class Dir
    def stat_label
      {
        attributes: "ADSHR",
        datetime: "datetime",
        size: "size"
      }
    end
  end

  class File
  end

  # drive can be "0".."9", :ram, :flash, etc
  # The name is case-insensitive
  def initialize(drive = "0", label = nil)
    @device = drive
    @prefix = "#{drive}:"
    @label = label || "PicoRuby"
  end

  attr_reader :device, :mountpoint

  def mkfs
    self._mkfs(@prefix)
    self.setlabel
    self
  end

  def setlabel
    return 0 unless @label
    self._setlabel("#{@prefix}#{@label}")
  end

  def getlabel
    self._getlabel(@prefix)
  end

  def sector_count
    res = self._getfree(@prefix)
    {total: (res >> 16), free: (res & 0b1111111111111111) }
  end

  def mount(mountpoint)
    @mountpoint = mountpoint
    @fatfs = self._mount("#{@prefix}#{mountpoint}")
  end

  def unmount
    self._unmount(@prefix)
    @fatfs = nil
  end

  def open_dir(path)
    FAT::Dir.new("#{@prefix}#{path}")
  end

  def open_file(path, mode)
    FAT::File.new("#{@prefix}#{path}", mode)
  end

  def chdir(path)
    # FatFs where FF_STR_VOLUME_ID == 2 configured
    # calls f_chdrive internally in f_chdir.
    # This is the reason of passing also @prefix
    path = "" if path == "/"
    if path == "" || FAT._exist?("#{@prefix}#{path}")
      FAT._chdir("#{@prefix}#{path}")
    else
      0
    end
  end

  def erase
    FAT._erase(@prefix)
  end

  def mkdir(path, mode)
    FAT._mkdir("#{@prefix}#{path}", mode)
  end

  def chmod(mode, path)
    FAT._chmod(mode, "#{@prefix}#{path}")
  end

  def stat(path)
    return Stat.new({mode: AM_DIR}) if path == "/"
    Stat.new FAT._stat("#{@prefix}#{path}")
  end

  def exist?(path)
    FAT._exist?("#{@prefix}#{path}")
  end

  def unlink(path)
    FAT._unlink("#{@prefix}#{path}")
  end

  def directory?(path)
    return true if path == "/"
    FAT._directory?("#{@prefix}#{path}")
  end
end
