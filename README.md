# WiFi Cat Feeder v2.0.0

An ESP8266-based automated cat feeder with WiFi configuration, web interface, and Alexa integration.

## New Features in v2.0.0

- **WiFi Manager**: No more hardcoded WiFi credentials
- **Easy Setup Process**: Configure WiFi through a web interface
- **Hardware Reset**: Added button to reset WiFi settings
- **Improved Reliability**: Enhanced connection handling and state management

## Hardware Requirements

- ESP8266 microcontroller
- Servo motor or DC motor for dispensing
- Limit switch for rotation detection
- Push button for WiFi reset (connected to D2)
- Power supply
- Food container and dispensing mechanism

## Pin Configuration

- `D1`: Motor control
- `D2`: WiFi reset button
- `D3`: Limit switch

## Initial Setup

1. Power on the device
2. The device will create a WiFi access point named `CatFeeder-[ChipID]`
3. Connect to this access point using your phone or computer
4. Open a web browser and navigate to `192.168.4.1`
5. Enter your WiFi network credentials
6. The device will save the credentials and restart
7. The cat feeder will now connect to your WiFi network automatically

## Changing WiFi Network

If you need to connect to a different WiFi network:

1. Press and hold the reset button (connected to D2)
2. The device will clear stored credentials and restart
3. Follow the initial setup process again

## Features

- Web interface for manual feeding
- Scheduled feeding with timer
- Feed count tracking
- Alexa integration for voice control
- WiFi configuration portal
- Status monitoring and debugging output

## License

This project is licensed under the GNU General Public License v2.0 - see the LICENSE file for details.

## Troubleshooting

### Common Issues

1. **Device not creating access point**
   - Check power supply
   - Try cycling power

2. **Cannot connect to WiFi**
   - Verify credentials are correct
   - Ensure network is within range
   - Check if network is 2.4GHz (5GHz not supported)

3. **Web interface not responding**
   - Verify device IP address
   - Check network connection
   - Try refreshing browser

### LED Status Indicators

The built-in LED will indicate different states:
- Rapid blinking: Setting up access point
- Slow blinking: Attempting to connect
- Solid: Connected to WiFi
- Off: Connection failed or device in error state

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Support and Updates

For bug reports and feature requests, please use the GitHub issues page.
