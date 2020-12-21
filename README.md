# mled
## Music Lights with WS281x LED strips

# Lights follows the beats. 
Currently tracks only lower frequency bands.
    TODO:
        Tracks beats on dominant frequency band.

## INSTALL

## Requires
# rpi_ws281x (https://github.com/jgarff/rpi_ws281x)

# FFTW3
    sudo apt install libfftw3-dev

# PulseAudio and Bluetooth

    sudo apt install pulseaudio pulseaudio-module-bluetooth pavucontrol bluez-firmware libpulse-dev

## Build and install with CMake:

   # Install CMake

   # Configure

      mkdir build
      cd build
      cmake ..
      
   # Build

      cmake --build .


I had problems with pulseaudio quality (popping/cracking sounds) when playing audio, fixed by changing the following in the file

/etc/pulse/daemon.conf

resample-method = ffmpeg

flat-volumes = no

