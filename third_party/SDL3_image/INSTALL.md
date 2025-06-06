
# Using this package

This package contains SDL3_image built for Visual Studio.

To use this package, edit your project properties:
- Add the include directory to "VC++ Directories" -> "Include Directories"
- Add the lib/_arch_ directory to "VC++ Directories" -> "Library Directories"
- Add SDL3_image.lib to Linker -> Input -> "Additional Dependencies"
- Copy lib/_arch_/SDL3_image.dll to your project directory.

You can include support for additional image formats by including the license and DLL files in the lib/_arch_/optional directory in your application. They will be automatically loaded by SDL_image as needed.

# Documentation

An API reference and additional documentation is available at:

https://wiki.libsdl.org/SDL3_image

# Discussions

## Discord

You can join the official Discord server at:

https://discord.com/invite/BwpFGBWsv8

## Forums/mailing lists

You can join SDL development discussions at:

https://discourse.libsdl.org/

Once you sign up, you can use the forum through the website or as a mailing list from your email client.

## Announcement list

You can sign up for the low traffic announcement list at:

https://www.libsdl.org/mailing-list.php

