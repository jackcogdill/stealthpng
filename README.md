# stegano

## About

Stegano is a steganography program made by Yentup written in C. It uses `libpng` and `openssl` for encryption. It is currently still in progress.

## Features

* Hides plaintext data or files in png images (in progress)
* Uses 256 bit AES for encryption
* Hashes the password 65536 times with SHA512 for the key (the longer the key, the more secure it is)

## Todo

* Fully test encryption
* Implement main algorithm for least "two significant bit" steganography
