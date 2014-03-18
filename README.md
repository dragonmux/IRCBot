# IRCBot

This project was created when I had need of doing more with an IRC bot than what Eggdrop allows. It's primary goal was to create a responsive, multi-threaded bot that could easily be integrated with database technologies and even contact other services such as websites to provide information on request or availability to a channel.

## Building the bot

rSON is required, but otherwise there are no further dependancies.
A way to automate satisfying that dependancy is comming soon.

A working build can be acomplished by simply typing "make" at the command prompt.
This has only been tested on Linux but theortically works the same on Windows and Mac too. The project uses standard GNU Make techniques.

## Configuration

The bot relies on a JSON configuration file named "server.json" to contain basic information such as what server on what port to connect to and what channel(s) to join under what alias.

An example configuration file is as follows

```JSON
{
	"server": "chat.freenode.net",
	"port": 6667,
	"channel": "#help",
	"nick": "test"
}
```

## The License

As stated in the code, I have licensed the library using GPL v3+.
Please report bugs to dx-mon@users.sourceforge.net

## Known Bugs

Known bugs and issues are documented in the open issues associated with this repository on GitHub
