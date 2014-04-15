# stegano

## About

Stegano is a steganography program made by Yentup written in C. It uses `libpng` and `openssl` for encryption. It is currently still in progress.

## Features

* Hides plaintext data or files in the least significant bits of png images (still being developed)
* Uses 256 bit AES for encryption
  * Hashes the password 65536 times with SHA512 for the key (the longer the password, the more secure it is)

## Compiling
> _Note: Must have [libpng](http://libpng.org/pub/png/libpng.html) and [openssl](https://www.openssl.org/) libraries installed to compile correctly_

Simply run `./configure` and then `make` 

## Todo

* Fully test encryption
* Implement main algorithm for "least two significant bits" steganography
* Add functionality to display how much space an image can hold
