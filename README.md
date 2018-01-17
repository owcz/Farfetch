![header](https://u.teknik.io/hh5Tl.png)

<div align=center>
	<b>Farfetch</b> is a free, open source, easily configurable system fetcher, far faster than it's competitors.
</div>

# Installation

To compile the binary, use `make`, after that, the binary (located in build/ff) will need a config file and an ascii file, inside the build folder there are already example files for that, if you wish, you can move this files inside ~/.config/farfetch/ and then alias a command to run the binary with the config location.

```bash
make
mkdir -p ~/.config/farfetch/ ~/bin
cp build/settings.ini build/skeleton.ascii ~/.config/farfetch/
cp build/ff ~/bin/
echo 'alias ff="~/bin/ff /home/$USER/.config/farfetch/settings.ini"' >> ~/.bashrc
```

Then, if you want another ascii rather than the skeleton example, edit ~/.config/farfetch/settings.ini and change the section art, inside [custom] with the name of the ascii file, the script will search this ascii file in the same directory as the settings.ini or, if specified, in the full path of the file.

Refer to section **Customization** if you want to learn more about the settings and ascii configurations.

# Usage

The usage is simple, it just needs one argument, the config file absolute or relative path, examples:

```bash
ff .config/farfetch/settings.ini
ff /home/$USER/.config/farfetch/settings.ini
```

# Customization

![config](https://u.teknik.io/q4Sg9.png)

In the example file settings.ini you have every available config with it's description commented, for the [custom] > h and for the ascii file, you can use custom colors, specified in `colors.h`, for example:

```
{RED}This is red text. {BOLD}{YELLOW} This is yellow bold text.{RESET} Back to normal.
```

Current list of working modules:

```
Kernel,Host,CPU,Packages,Uptime
```

Current list of working bars:

```
disk,ram,palette
```

## TODO

* Improve **Customization** section.
* make install.
* Make more system info modules.
* Improve the cache file to hold different things.
* Make the bar label inside the actual bar, that would look cool.
* Text and bar colors inside settings file.
* Move rplc and other functions to another header.

<div align="center">
	<i>Pokémon, Pokémon character names, Nintendo 3DS, Nintendo DS, Wii, Wii U, and Wiiware are trademarks of Nintendo.</i>
</div>