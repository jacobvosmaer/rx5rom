# rx5rom

Utilities for working with Yamaha RX5 wave ROM dumps.

- `rx5-ls`: list contents of ROM
- `rx5-split`: extract ROM wave data as separate WAV files
- `rx5-program`: upload 1 or more ROM data files to RX5USB hardware (using [hidapi](https://github.com/libusb/hidapi))

## Programming example

rx5-program takes two arguments: the first slot to write to (0-3) and
the number of banks to write (1-4). It reads the bank contents from
standard input so you can use `cat` to write multiple banks at once.

For example, to program the 4 banks that make up cartridges WRC01 and WRC02:

```
cat WRC01A.rx5bank WRC01B.rx5bank WRC02A.rx5bank WRC02A.rx5bank | rx5-program 0 4
```

## Credits

Parsing of ROM data is based on in [gligli/rx5usb](https://github.com/gligli/rx5usb).
