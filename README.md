# rx5rom

Utilities for working with Yamaha RX5 wave ROM dumps.

- `rx5-ls`: list contents of ROM
- `rx5-split`: extract ROM wave data as separate WAV files
- `rx5-program`: upload 1 or more ROM data files to RX5USB hardware (using [hidapi](https://github.com/libusb/hidapi))
- `rx5-build`: assemble WAV files into a ROM image

If you are using Windows you may want to use the [original RX5USB software](https://github.com/gligli/rx5usb) instead.

Also see:

- [YouTube demo](https://www.youtube.com/watch?v=c1J6XAVyTX0)
- [Blog post](https://blog.jacobvosmaer.nl/0064-rx5usb-software/)

## Programming example

rx5-program takes two arguments: the first slot to write to (0-3) and
the number of banks to write (1-4). It reads the bank contents from
standard input so you can use `cat` to write multiple banks at once.

For example, to program the 4 banks that make up cartridges WRC01 and WRC02:

```
cat WRC01A.rx5bank WRC01B.rx5bank WRC02A.rx5bank WRC02A.rx5bank | rx5-program 0 4
```

## Building ROM images

The ROM builder `rx5-build` reads a ROM description from standard input and writes the resulting 128KB ROM image to stdout. A description consists of an optional `romid` followed by one or more file sections, which start with `file12 /path/to/some-file.wav` (or `file8` if you want to store the audio as 8-bit PCM). The path to the wav file may not contain a newline character. Optionally, you can set one of the following parameters on the current file:

- octave
- note
- loop
- loopstart
- loopend
- attackrate
- decay1rate
- decay1level
- decay2rate
- releaserate
- gatetime
- bendrate
- bendrange
- reverseattackrate
- level
- channel

All parameters use the native numerical format of the ROM dumps. For example, `channel` starts at 0. Channels automatically increment from file to file.

By default, the voice name is the first 6 characters of the filename. You can override this by adding a `name` statement.

The `copy` keyword will copy the last entered voice. This allows you to have multiple voices that play back the same PCM audio data.

The native sample rate is 25kHz. If you use WAV files with a different sample rate they will play back too fast or too slow on the RX5. Stereo WAV files are automatically summed to mono by rx5-build.

Syntax example:

```
# Add kick.wav with custom envelope settings and name
file12 kick.wav
name Kick!!
decay1rate 50
decay1level 0
# Add rim.wav with default settings
file12 rim.wav
# Add snare.wave with custom level
file12 snare.wav
level 30
```

### ROM building example

Assemble all wav files in the current directory into a ROM image:

```
ls *.wav | sed 's/^/file12 /' | rx5-build > rom.bin
```

### Bugs

The loopstart and loopend fields don't work correctly in rx5-build. You can however loop the entire sample by writing `loop 1`.

## Credits

Parsing of ROM data is based on in [gligli/rx5usb](https://github.com/gligli/rx5usb).
