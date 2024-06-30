# Unreal Engine 5 Server-Side Rewind

A demo of lag compensation using server-side rewind in Unreal Engine 5 C++.

![Main](https://github.com/marcohenning/ue5-server-side-rewind/assets/91918460/b257f6b9-f162-4a3f-953e-32dfd5f78fc8)

## About

Lag compensation is an essential part of every fast-paced multiplayer game, especially shooter games. The specific lag compensation concept this project is focussing on is called server-side rewind and is widely used in the shooter game industry. This project is a small demo of how server-side rewind can be implemented in Unreal Engine 5. It consists of a simple kill functionality triggered by pressing the left mouse button and aiming the crosshair at another player. Killed players will collapse and fall to the ground. Every time you hit a player on your local machine, a red box is drawn at the location of the hit to easily identify if it's correctly registered by the server. Server-side rewind can be enabled and disabled in the game mode blueprint. Ping is simulated by specifying the desired amount of packet lag (`PktLag`) in `DefaultEngine.ini`.

## How does it work?

The problem server-side rewind solves is simple. When one player shoots another player, a request to check for a hit is sent to the server. The amount of time it takes the request to reach the server is determined by the player's ping. If the ping is high and the request takes, say, 500ms to reach the server, then the server is checking for a hit on a version of the game that's actually 500ms ahead of the moment the player pulled the trigger. If their opponent was moving, it is very likely that they have already moved far enough to not be in the trajectory of the bullet anymore. This way the server will detect no hit and the high-ping player will not get rewarded for what looked like a hit on their machine. Server-side rewind generally works by storing positional data of all players and, instead of checking for a hit on the current version of the game, rewinding time back to when the trigger was actually pulled and then checking for a hit on that version of the game. By using this technique, high-ping players can get rewarded for their hits regardless of their ping, which massively improves their gameplay experience.

## Low Ping without Server-Side Rewind

Without lag everything works as intended.

https://github.com/marcohenning/ue5-server-side-rewind/assets/91918460/528a2208-1335-46ee-a1bb-7fd87c5627c5

## High Ping without Server-Side Rewind

With heavy lag, however, most hits aren't registered.

https://github.com/marcohenning/ue5-server-side-rewind/assets/91918460/834b689e-dc35-44eb-bfab-502279ca2cb6

## High Ping with Server-Side Rewind

By using server-side rewind, even with heavy lag, the hit is registered correctly. Due to the time the packets require to travel to the server and back to the client under high ping conditions, the kill is delayed, as it is only shown once the client receives confirmation from the server. This is why every large shooter game using server-side rewind will impose ping limits, above which server-side rewind is disabled.

https://github.com/marcohenning/ue5-server-side-rewind/assets/91918460/6a2404da-01ef-49a5-a0b5-1d1d2f0b577b

## Implementation

Every frame the character's hitbox positions are saved in a struct called `FServerSideRewindSnapshot` and stored in a list named `ServerSideRewindSnapshotHistory`. Snapshots older than the maximum rewind time are removed from the list.

https://github.com/marcohenning/ue5-server-side-rewind/assets/91918460/d2be0d8a-51d5-4fbe-8581-203494f9c825

When a potential kill needs to be checked using server-side rewind, the method `CheckForKill()` is called. It first finds the closest available snapshot to the client's time of request using the `FindSnapshotToCheck()` method, then rewinds the hitbox positions to where they were at the time of the snapshot by using the method `MoveHitBoxesToSnapshot()` and finally performs a line trace against the custom trace channel of the hitboxes. Once this is done, the original hitbox positions are restored and a bool containing the result is returned.

The main server-side rewind functionality is implemented in the following classes:

```cpp
class SERVERSIDEREWIND_API UServerSideRewindComponent : public UActorComponent
```

* Subclass of `UActorComponent`, which is the base class for components defining reusable behavior that can be added to different types of Actors (i.e. Characters)
* Handles everything related to server-side rewind
* Defines methods such as `TakeServerSideRewindSnapshot()`, `SaveServerSideRewindSnapshot()`, `ShowServerSideRewindSnapshot()`, `FindSnapshotToCheck()`, `MoveHitBoxesToSnapshot()` and `CheckForKill()`

```cpp
class SERVERSIDEREWIND_API AFirstPersonCharacter : public ACharacter
```

* The character controlled by the players
* Has a `UServerSideRewindComponent`, which is responsible for handling everything related to server-side rewind
* Has a TMap `HitBoxes` containing all of the character's hitboxes used for server-side rewind
* Each hitbox is a `UBoxComponent` attached to its respective bone on the character model in the constructor

## Version

This project was made using Unreal Engine 5.3.2.

## License

This software is licensed under the [MIT license](LICENSE).
