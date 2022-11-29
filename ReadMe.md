# Twin Stick Shooter - Replication

The goal of this project was to take the **Twin Stick Shooter** template on **Unreal Engine** and make it work in a *Network*.


## Objectives
___


## Project Overview
___

After launching the application, you will fall on a simple menu. This is where you will be able to **create your network session** or **join an existing one**. Simply set your *player settings* (pseudo, color) and click on a button to start

<p align="center"><img src="Annexe/ReadMe/MainMenu.png"  width="450"> </p>

When clicking on the **Host** button, a network session is created and you fall on a lobby room, waiting for you to launch the game. All the connected players will appear here.\
You can start the game session at any time by clicking the "*StartServer*" button or delete it by clicking the "*LeaveLobby*" button.

<p align="center"><img src="Annexe/ReadMe/LobbyRoom.png"  width="450"> </p>

When clicking on the **Join** button, you will have access to all the available sessions on the server. You can select any session and join it to play with other players.

> **Note:** If the session is already started (not on the lobby room), joining will automatically make you spawn in the game map.

<p align="center"><img src="Annexe/ReadMe/LobbyList.png"  width="450"> </p>

## Sources
___

Unreal Engine Doc : [Actors Replication](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/Actors/)