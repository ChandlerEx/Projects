## Electronics & Maker Projects

Welcome! This is a collection of my hands-on electronics and product design builds — each project blends embedded programming, hardware integration, and 3D-printed enclosures.
They’re documented here both as a personal portfolio and as a resource for anyone interested in building similar devices.

## Projects
| Project | One-Sheet PDF | Description | Preview |
|---------|-----------------------|-------------|---------|
| [**Electrum Coin**](./Electrum) | * | The Electrum coin was a 3D printing exercise to familiarize myself with the capabilities of the Stratasys J850 Polyjet 3D printing system. | <img src="./Electrum/visual-media/ElectrumPic1.jpg" alt="Electrum Coin Result" width="150"/> |
| [**AI-CAD Ring Box**](./Cat_Speedometer) | * | This design proved the idea of using Gemini to quickly create useful CAD files to spec by implementing a new interface and rubric for it based on Stanford research. | <img src="./AI-CAD Ring Box/visual-media/RingBox2.jpg" alt="Finished Ring Box" width="150"/> |
| [**Cat Speedometer**](./Cat_Speedometer) | * | The Cat Wheel Speedometer is a computer-vision-based telemetry system that measures the running speed of a cat exercise wheel in real time. | <img src="./Cat_Speedometer/visual-media/Speedo.jpg" alt="Cat Speedometer Finished Build" width="150"/> |
| [**GPS Clock**](./GPSClock) | * | The GPSClock is a low-power, GPS-disciplined wall clock built around an ATmega328P microcontroller. It combines a DS3231 real-time clock with periodic synchronization from a GT-U7 GPS module to maintain accurate time even during long periods without satellite visibility. | <img src="./GPSClock/visual-media/GPSClock.jpg" alt="Cat Speedometer Finished Build" width="150"/> |
| [**The Orb**](./TheOrb) | [Quick Details and Build Reference](https://raw.githubusercontent.com/ChandlerEx/Projects/main/TheOrb/docs/Orb_One_Sheet.pdf) | Ambient LED orb that visualizes live stock performance through color and pulsing light. Powered by an ESP8266 D1 Mini with custom PCB, in a 3D-printed enclosure topped with a frosted glass globe. | <img src="./TheOrb/visual-media/OrbThumb.jpg" alt="The Orb Finished Build" width="150"/> |
| [**Soil Moisture Monitor**](./SoilMonitor) | [Quick Details and Build Reference](https://raw.githubusercontent.com/ChandlerEx/Projects/main/SoilMonitor/docs/SoilMon_One_Sheet.pdf) | Battery-powered OLED moisture monitor that tells plant lovers when to water. Built with a XIAO nRF52840 and capacitive sensor, in a precision-fit 3D-printed enclosure with calibration-tuned firmware for succulents. | <img src="./SoilMonitor/visual-media/SoilMonInUse.jpg" alt="Soil Moisture Monitor in Use" width="150"/> |
| [**Scoreboard**](./Scoreboard) | [Quick Details and Build Reference](https://raw.githubusercontent.com/ChandlerEx/Projects/main/Scoreboard/docs/Scoreboard_One_Sheet.pdf) | Desk-friendly digital clock with a hidden touchscreen that displays live MLB game data on demand. Uses an ESP32 CYD with custom 3D-printed enclosure and captive portal Wi-Fi setup. | <img src="./Scoreboard/visual-media/ScoreboardScore.jpg" alt="Scoreboard Showing Live MLB Score" width="150"/> |

## Technologies & Skills Featured
- Built new interface for Google Gemini so it could quickly generate solid, accurate parts
- Worked with GrabCAD software to slice and print PolyJet parts for Stratasys J850, J750, and J55 printers
- 3D modeling in Fusion 360; basic use of Rhino (threaded, snap-fit, and friction-fit joins)
- 3D printing in PLA, PETG, and PolyJet (including translucent parts)
- PCB design and fabrication
- Hardware integration (microcontrollers, sensors, and displays)
- Microcontroller programming (ESP32, ESP8266 D1 Mini, and nRF52840)
- Arduino-style C++ development for embedded systems (with AI assistance)
- Battery management and low-power firmware design
- I²C device integration (OLED displays, sensors)

## Repository Structure
Most projects follows the same organized folder structure:<br><br>
ProjectName/cad/&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;# Fusion 360 source files (.f3d) and STL exports  
ProjectName/docs/&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;# One-sheet PDF, dimension drawings, reference documents  
ProjectName/firmware/&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;# Arduino source code  
ProjectName/media/&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;# Photos, renders, and videos (static assets for README)  
ProjectName/README.md&nbsp;&nbsp;# Project-specific documentation  

## About Me
I’m Chandler, a prototype technician and maker who builds electro-mechanical devices from the ground up. My work blends embedded electronics, soldering and wiring, 3D printing, and Fusion 360 design to create hardware that’s reliable, reproducible, and cleanly executed. I like turning rough ideas into well-built systems: custom jigs, tight enclosures, clear documentation, and hardware that survives real use. I focus on practical engineering, thoughtful UX, and build workflows that scale.
