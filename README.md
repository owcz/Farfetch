![header](https://u.teknik.io/hh5Tl.png)

<div align=center>
	<b>Farfetch</b> is a free, open source, easily configurable system fetcher, far faster than it's competitors.
</div>

# Installation

To compile the binary, use `make`, then, use `make install`, this command will move the main settings.ini default file into ~/.config/farfetch/settings.ini (it won't overwrite it) and all the ascii files inside ~/.config/farfetch/ascii/* , it will also make a ~/bin folder if it didn't exist and add `export PATH=$PATH:$HOME/bin` into your .bashrc.

```bash
make
make install
```

Then, if you want another ascii rather than the skeleton example, edit ~/.config/farfetch/settings.ini and change the section art (or make a new one), inside [custom] with the name of the ascii file, the script will search this ascii file in the same directory as the settings.ini or, if specified, in the full path of the file.

*Refer to section **Customization** if you want to learn more about the settings and ascii configurations.*

# Usage

The usage is simple, it just needs one argument, the config file absolute or relative path, examples:

```bash
ff .config/farfetch/settings.ini
ff /home/$USER/.config/farfetch/settings.ini
```

You can alias this command inside your .bashrc if you want, or put it at the end so it shows up every time you open a terminal:

```bash
alias ff='ff ~/.config/farfetch/settings.ini'
ff
```

# Customization

![config](https://u.teknik.io/q4Sg9.png)

[*For more detailed information, click this link to check the wiki*](https://github.com/Capuno/Farfetch/wiki)

In the example file settings.ini you have every available config with it's description commented, for the [custom] > h and for the ascii file, you can use custom colors, specified in `colors.h`, for example:

```
{RED}This is red text. {BOLD}{YELLOW} This is yellow bold text.{RESET} Back to normal.
```

<hr>

Current list of working modules:

```
Kernel,Host,CPU,Packages,Uptime,GPU,Processes,User,Distro
```
*GPU is experimental, if it's not working in your system, please open an issue with your `locate pci.ids` output*

<hr>

Current list of working bars:

```
disk,ram,palette
```

## TODO

* Check if sys.ini variables are enabled in the config before initialising them.
* Improve **Customization** section.
* Add ascii wiki page.
* ~~make install.~~
* **WIP** ~~Make more system info modules.~~
* Improve the cache file to hold different things.
* ~~Make the bar label inside the actual bar, that would look cool.~~
* **WIP** ~~Move rplc and other functions to another header.~~

<div align="center">
	<i>Pokémon, Pokémon character names, Nintendo 3DS, Nintendo DS, Wii, Wii U, and Wiiware are trademarks of Nintendo.</i>
</div>
