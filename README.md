# stegano

## About

Stegano is a steganography program made by Yentup written in C. It uses `libpng` and `openssl` for encryption.

## Features

* Hides plaintext data or files in the least significant bits of png images
* Uses 256 bit AES for encryption
 * To protect against brute forcing, the password is hashed 65536 times with SHA512 for the key (still, the longer the password, the more secure it is)
 * For security, there is no way of knowing if the decryption was successful. If you input an incorrect password, the resulting data will just be a bunch of garbage

## Compiling
> _Note: Must have [libpng](http://libpng.org/pub/png/libpng.html) and [openssl](https://www.openssl.org/) libraries installed to compile correctly, as well as [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/)_

Simply run `./configure` and then `make` 

## Todo

* Add option to save encoded images to specific file names (-o)

## Example

Download this image and run `$ ./stegano -d <image>`. Decode with the password `onegai`.

![Image](http://i.imgur.com/SFUSLoz.png)
