# stealthpng

## About

Stealthpng is a steganography program made by Jack Cogdill written in C. It uses [libpng](http://libpng.org/pub/png/libpng.html) and [openssl](https://www.openssl.org/) for encryption.

## License and credits

Code taken from [here](http://zarb.org/~gc/html/libpng.html) ([plaintext](http://zarb.org/~gc/resource/libpng-short-example.c)) to serve as a basis for libpng functions, and code also used from [here](http://saju.net.in/blog/?p=36) ([plaintext](http://saju.net.in/code/misc/openssl_aes.c.txt)) to help with the AES encryption and decryption with openssl.  
Otherwise completely written from scratch by Jack Cogdill and licensed with [GPLv3](https://www.gnu.org/licenses/gpl.html).

## Features

* Hides plaintext data or files in the least significant bits of pixels in png images
* Uses 256 bit AES for encryption
 * To protect against brute forcing, the password is hashed 65536 times with SHA512 for the key (still, the longer the password, the more secure it is)
 * For security, there is no way of knowing if the decryption was successful. If you input an incorrect password, the resulting data will just be a bunch of garbage

## Compiling
> _Note: Must have [libpng](http://libpng.org/pub/png/libpng.html) and [openssl](https://www.openssl.org/) libraries installed to compile correctly, as well as [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/)_

Simply run `./configure && make`

## Example

Download this image and run `$ ./stealthpng -d <image>`. Decode with the password `onegai`.

![Image](http://i.imgur.com/SFUSLoz.png)

## Help
> _Note: when encoding, by default the program outputs to a new png file called "new-image.png".

Run `./stealthpng -h` for more options.
