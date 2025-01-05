# avalon_game
A game server that allows multiple user connect to it and play Avalon board game

## usage on Raspberry pi
In the working directory, run makefile command ```make```

This will generate a ```program```. By running ```./program <port>```, it will start listening on <port>

On client laptops which is connected to the same network as raspberry pi, simply run ```nc rpi.local <port>``` and start the game.
