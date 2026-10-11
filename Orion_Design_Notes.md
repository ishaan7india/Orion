# Orion keyboard — draft design package

Source basis:
- User-uploaded Schematik procurement BOM PDF exported 2026-10-11 17:39.
- Connection tables pasted by the user in this conversation.
- Live Schematik project URL could not be fetched by the browsing tool during this turn.

Files:
- Orion_BOM.csv: BOM transcribed from the uploaded PDF. Quantities and source descriptions are preserved.
- Orion_Connections.csv: connection summary based on the wiring tables in chat.
- Orion_Architecture.svg: visual block-level architecture (not an electrical schematic).
- Orion_KiCad_Starter.kicad_pro: minimal KiCad project settings starter.
- Orion_Design_Notes.md: assumptions and blockers.

IMPORTANT:
This package does NOT contain a verified electrical schematic or routed PCB. A BOM and connection table are insufficient to safely infer all symbol pins, footprints, exact switch coordinates, matrix nets, connector pinouts, USB power isolation, and PCB routing. No ERC/DRC or firmware build has been run here.

Before producing a manufacturing-ready KiCad .kicad_sch/.kicad_pcb or EasyEDA project, confirm:
1. Exact Pico variant/footprint and whether the design uses a socket/module or bare-board header.
2. Full matrix switch list, physical key coordinates, diode orientation and all row/column net assignments.
3. Exact symbol/footprint/pinout for joystick, encoder, OLED and power connectors.
4. Exact 74AHCT125 package pin mapping and tie-offs for unused channels.
5. 5 V supply input, fuse rating/derating, USB back-feed protection, power connector rating, copper width and LED power injection.
6. Whether each of 128 LEDs really gets a 100 nF capacitor on this PCB; placement and routing may be impractical.
7. Auxiliary connector's 24 pin assignment (8 rows + 16 columns).
8. Firmware core and HID library versions, then compile and test.

The SVG is only a high-level architecture picture. Do not send it to a PCB manufacturer.
