# ar9331-i2s-alsa
I2S alsa device drivers for the popular MIPS ar9331 SOC (Carambola2, Arduino Yun...)

#OpenWrt installation:
  * create directory <build_root>/package/kernel/ar9331-i2s-alsa/
  * copy driver content to <build_root>/package/kernel/ar9331-i2s-alsa/
  * make menuconfig -> enable i2s-alsa
  * make package/kernel/ar9331-i2s-alsa/compile V=s

It is implemented as a openwrt package and can be compiled with Linux Kernel Ver. 3.14.xx.

    insmod dev-audio.ko
    insmod ath79-i2s.ko
    insmod ath79-pcm-mbox.ko
    insmod wm8727.ko
    insmod ath-carambola2.ko
