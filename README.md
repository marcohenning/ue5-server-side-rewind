# Unreal Engine 5 Server Side Rewind

A demo of lag compensation using server-side rewind in Unreal Engine 5 C++.

## About

Lag compensation is an essential part of every fast-paced multiplayer game, especially shooter games. The specific lag compensation concept this project is focussing on is called server-side rewind and is widely used in the shooter game industry. This project is a small demo of how server-side rewind can be implemented in Unreal Engine 5. It consists of a simple kill functionality triggered by pressing the left mouse button and aiming the crosshair at another player. Killed players will collapse and fall to the ground. Server-side rewind can be enabled and disabled in the game mode blueprint. Ping is simulated by specifying the desired amount of packet lag (`PktLag`) in `DefaultEngine.ini`.

## How does it work?

The problem server-side rewind solves is simple. When one player shoots another player, a request to check for a hit is sent to the server. The amount of time it takes the request to reach the server is determined by the player's ping. If the ping is high and the request takes, say, 500ms to reach the server, then the server is checking for a hit on a version of the game that's actually 500ms ahead of the moment the player pulled the trigger. If their opponent was moving, it is very likely that they have already moved far enough to not be in the trajectory of the bullet anymore. This way the server will detect no hit and the high-ping player will not get rewarded for what looked like a hit on their machine. Server-side rewind generally works by storing positional data of all players and, instead of checking for a hit on the current version of the game, rewinding time back to when the trigger was actually pulled and then checking for a hit on that version of the game. By using this technique, high-ping players can get rewarded for their hits regardless of their ping, which massively improves their gameplay experience.

## Low Ping without Server-Side Rewind

Without lag everything works as intended.

## High Ping without Server-Side Rewind

With heavy lag, however, most hits aren't registered.

## High Ping with Server-Side Rewind

By using server-side rewind, even with heavy lag, the hit is registered correctly. Due to the time the packets require to travel to the server and back to the client under high ping conditions, the kill is delayed, as it is only shown once the client receives confirmation from the server. This is why every large shooter game using server-side rewind will impose ping limits, above which server-side rewind is disabled.

## License

This software is licensed under the [MIT license](LICENSE).