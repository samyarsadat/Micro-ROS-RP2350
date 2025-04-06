<h1 align="center">Micro-ROS RP2350 - RP2040 Backport</h1>

<p align="center">
	<br>
	<a href="https://www.ros.org"><img src="https://github.com/samyarsadat/ROS-Robot/raw/stage-1/assets/logos/ROS_logo.svg"></a><br>
	<br>
	<br>
	<a href="LICENSE"><img src="https://img.shields.io/github/license/samyarsadat/Micro-ROS-RP2350?color=blue"></a>
	|
	<a href="../../issues"><img src="https://img.shields.io/github/issues/samyarsadat/Micro-ROS-RP2350"></a>
	<br><br>
</p>

----
This branch contains the exact same example and workflow as the `main` branch, just backported to the RP2040 (Pico 1). I did this because there were several general workflow improvements made during the RP2350 transition (as well as a new version of the Pico SDK), and I wanted to have those improvements available for the RP2040 as well.

Additionally, proper multi-core publishing and subscribing now also works on the RP2040, unlike before. I presume the new `2.x` Pico SDK might have something to do with it.

<br>

## Contact
You can contact me via e-mail.<br>
E-mail: samyarsadat@gigawhat.net<br>
<br>
If you think that you have found a bug or issue please report it <a href="../../issues">here</a>.

<br>


## Credits
| Role           | Name                                                             |
| -------------- | ---------------------------------------------------------------- |
| Lead Developer | <a href="https://github.com/samyarsadat">Samyar Sadat Akhavi</a> |

<br>

Copyright © 2025 Samyar Sadat Akhavi.